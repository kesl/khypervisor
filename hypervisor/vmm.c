/* Virtual Machine Memory Management Module */

#include <k-hypervisor-config.h>
#include <arch_types.h>
#include "vmm.h"
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include <gic_regs.h>

#include <config/cfg_platform.h>
#include <log/print.h>

/* Stage 2 Level 1 */
#define VMM_L1_PTE_NUM          4
#define VMM_L1_PADDING_PTE_NUM   (512 - VMM_L1_PTE_NUM)
/* Stage 2 Level 2 */
#define VMM_L2_PTE_NUM          512
#define VMM_L3_PTE_NUM          512
/**
 * @brief Totla Number of Level 2, Level 3 Page Table Entry
 *
 * VMM_L2_PTE_NUM * VMM_L3_PTE_NUM = /<br>
 * Total Number Of Level 3 Page Table Entry<br>
 * + VMM_L2_PTE_NUM = Total Number Of Level 2 Page Table Entry
 */
#define VMM_L2L3_PTE_NUM_TOTAL  (VMM_L2_PTE_NUM \
        * VMM_L3_PTE_NUM + VMM_L2_PTE_NUM)
/**
 * @brief Total Number of All Page Table Entry
 */
#define VMM_PTE_NUM_TOTAL  (VMM_L1_PTE_NUM                  \
        + VMM_L1_PADDING_PTE_NUM + VMM_L2L3_PTE_NUM_TOTAL   \
        * VMM_L1_PTE_NUM)
/* \defgroup VTTBR
 * @{
 */
#define VTTBR_INITVAL                                   0x0000000000000000ULL
#define VTTBR_VMID_MASK                                 0x00FF000000000000ULL
#define VTTBR_VMID_SHIFT                                48
#define VTTBR_BADDR_MASK                                0x000000FFFFFFF000ULL
#define VTTBR_BADDR_SHIFT                               12
/** @} */
/** \defgroup VTCR
 * @{
 */
#define VTCR_INITVAL                                    0x80000000
#define VTCR_SH0_MASK                                   0x00003000
#define VTCR_SH0_SHIFT                                  12
#define VTCR_ORGN0_MASK                                 0x00000C00
#define VTCR_ORGN0_SHIFT                                10
#define VTCR_IRGN0_MASK                                 0x00000300
#define VTCR_IRGN0_SHIFT                                8
#define VTCR_SL0_MASK                                   0x000000C0
#define VTCR_SL0_SHIFT                                  6
#define VTCR_S_MASK                                     0x00000010
#define VTCR_S_SHIFT                                    4
#define VTCR_T0SZ_MASK                                  0x00000003
#define VTCR_T0SZ_SHIFT                                 0
/** @} */
/*
 * Stage 2 Translation Table, look up begins at second level
 * VTTBR.BADDR[31:x]: x=14, VTCR.T0SZ = 0, 2^32 input address range,
 * VTCR.SL0 = 0(2nd), 16KB aligned base address
 * Statically allocated for now
 */

static union lpaed *_vmid_ttbl[NUM_GUESTS_STATIC];

static union lpaed
_ttbl_guest0[VMM_PTE_NUM_TOTAL] __attribute((__aligned__(4096)));
static union lpaed
_ttbl_guest1[VMM_PTE_NUM_TOTAL] __attribute((__aligned__(4096)));

/**
 * @brief Memory Mapping Descriptor
 *
 * Memory Mapping information Descriptor
 */
struct memmap_desc {
    char *label;  /**< string, memory region name*/
    uint64_t va;  /**< Virtual Address */
    uint64_t pa;  /**< Physical Address */
    uint32_t size;/**< Size of memory region */
    enum lpaed_stage2_memattr attr;/**< Memory Attribute Information */
};

static struct memmap_desc guest_md_empty[] = {
    {       0, 0, 0, 0,  0},
};

static struct memmap_desc guest_device_md0[] = {
    /*  label, ipa, pa, size, attr */
    CFG_GUEST0_DEVICE_MEMORY,
    { 0, 0, 0, 0,  0},
};

static struct memmap_desc guest_device_md1[] = {
    /*  label, ipa, pa, size, attr */
    CFG_GUEST1_DEVICE_MEMORY,
    { 0, 0, 0, 0,  0},
};

static struct memmap_desc guest_memory_md0[] = {
    /* 756MB */
    {"start", 0x00000000, 0, 0x30000000,
     LPAED_STAGE2_MEMATTR_NORMAL_OWB | LPAED_STAGE2_MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

static struct memmap_desc guest_memory_md1[] = {
    /* 256MB */
    {"start", 0x00000000,          0, 0x10000000,
     LPAED_STAGE2_MEMATTR_NORMAL_OWB | LPAED_STAGE2_MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

/* Memory Map for Guest 0 */
static struct memmap_desc *guest_mdlist0[] = {
    &guest_device_md0[0],   /* 0x0000_0000 */
    &guest_md_empty[0],     /* 0x4000_0000 */
    &guest_memory_md0[0],
    &guest_md_empty[0],     /* 0xC000_0000 PA:0x40000000*/
    0
};

/* Memory Map for Guest 0 */
static struct memmap_desc *guest_mdlist1[] = {
    &guest_device_md1[0],
    &guest_md_empty[0],
    &guest_memory_md1[0],
    &guest_md_empty[0],
    0
};

/**
 * @brief Get TTBL_L3 Entry
 * Returns address of L3 TTBL at 'l2index' entry of L2
 * union lpaed *TTBL_L3(union lpaed *ttbl_l2, uint32_t index_l2);
 */
#define TTBL_L3(ttbl_l2, index_l2) \
    (&ttbl_l2[VMM_L2_PTE_NUM + (VMM_L3_PTE_NUM * (index_l2))])
/**
 * @brief Get TTBL_L2 Entry
 * Returns address of L2 TTBL at 'l1index' entry of L1
 * union lpaed *TTBL_L2(union lpaed *ttbl_l1, uint32_t index_l1);
 */
#define TTBL_L2(ttbl_l1, index_l1) \
    (&ttbl_l1[(VMM_L1_PTE_NUM + VMM_L1_PADDING_PTE_NUM) \
              + (VMM_L2L3_PTE_NUM_TOTAL * (index_l1))])

/**
 * @brief Virtual Machine's Memory mapping to ttbl3 Entry
 *
 * Mapping  the Information about physical address and<br>
 * Virtual Machine's Virtual address to ttbl3 Entry
 * @param lpaed_t *ttbl3 target Level 3 Translation Table Pointer
 * @param uint64_t offset target's location
 * @param uint32_t pages number of pages
 * @param uint64_t pa Physical address
 * @param lpaed_stage2_memattr_t mattr Memory Attribute
 * @return void
 */
static void vmm_ttbl3_map(union lpaed *ttbl3, uint64_t offset, uint32_t pages,
                uint64_t pa, enum lpaed_stage2_memattr mattr)
{
    int index_l3 = 0;
    int index_l3_last = 0;
    printh("%s[%d]: ttbl3:%x offset:%x pte:%x pages:%d, pa:%x\n",
            __func__, __LINE__, (uint32_t) ttbl3, (uint32_t) offset,
            &ttbl3[offset], pages, (uint32_t) pa);
    /* Initialize the address spaces with 'invalid' state */
    index_l3 = offset;
    index_l3_last = index_l3 + pages;
    for (; index_l3 < index_l3_last; index_l3++) {
        lpaed_stage2_map_page(&ttbl3[index_l3], pa, mattr);
        pa += LPAE_PAGE_SIZE;
    }
}

/**
 * @brief Virtual machine's Memory unmapping in ttbl3 Entry
 *
 * Unmapping Virtual Address in ttbl3 Entry<br>
 * And Invalidate that Entry
 * @param lpaed_t *ttbl3 Target Level 3 Translation Table Pointer
 * @param uint64_t offset target's location
 * @param uint32_t pages number of pages
 * @return void
 */
static void vmm_ttbl3_unmap(union lpaed *ttbl3, uint64_t offset,
                uint32_t pages)
{
    int index_l3 = 0;
    int index_l3_last = 0;
    /* Initialize the address spaces with 'invalid' state */
    index_l3 = offset >> LPAE_PAGE_SHIFT;
    index_l3_last = index_l3 + pages;
    for (; index_l3 < index_l3_last; index_l3++)
        ttbl3[index_l3].pt.valid = 0;
}

/**
 * @brief Virtual machine's Memory Address unmapping in ttbl2 & ttbl3 Entry
 *
 * Unmapping Virtual Address in ttbl2 & ttbl3 Entry<br>
 * And Invalidate that Entry<br>
 * <br>
 * - va_offset: 0 ~ (1GB - size), start contiguous<br>
 *   virtual address within level 1 block (1GB), <br>
 *   L2 lock size(2MB) aligned<br>
 * - size: <= 1GB<br>
 *   page size aligned<br>
 * @param lpaed_t *ttbl2 Target Level 2 Translation Table Pointer
 * @param uint64_t va_offset Virtual Address Offset
 * @param uint32_t size size of target Memory
 * @return void
 */
static void vmm_ttbl2_unmap(union lpaed *ttbl2, uint64_t va_offset,
                uint32_t size)
{
    int index_l2 = 0;
    int index_l2_last = 0;
    int num_blocks = 0;
    /* Initialize the address spaces with 'invalid' state */
    num_blocks = size >> LPAE_BLOCK_L2_SHIFT;
    index_l2 = va_offset >> LPAE_BLOCK_L2_SHIFT;
    index_l2_last = num_blocks;

    for (; index_l2 < index_l2_last; index_l2++)
        ttbl2[index_l2].pt.valid = 0;

    size &= LPAE_BLOCK_L2_MASK;
    if (size) {
        /* last partial block */
        union lpaed *ttbl3 = TTBL_L3(ttbl2, index_l2);
        vmm_ttbl3_unmap(ttbl3, 0x00000000, size >> LPAE_PAGE_SHIFT);
    }
}

/**
 * @brief Virtual machine's Memory mapping to ttbl2 Entry
 *
 * Mapping Virtual Machine's Virtual Address to TTBL2 Entry & TTBL3 Entry<br>
 * - First, check the TTBL2's Index & block_ offset<br>
 * - Second, if block's offset is not zero<br>
 *    (==location is not fit in Level2 TTB Entry)<br>
 *   mapping it first & mapping other to n Blocks<br>
 * - Third, if tail's size is remained, mapping it
 * @param lpaed_t *ttbl2 target Level2 Translation Table Pointer
 * @param uint64_t va_offset Virtual Address's Offset
 * @param uint64_t pa Physical Address
 * @param uint32_t size size of target Memory
 * @param lpaed_stage2_memattr_t Memory Attribute
 * @return void
 */
static void vmm_ttbl2_map(union lpaed *ttbl2, uint64_t va_offset, uint64_t pa,
                uint32_t size, enum lpaed_stage2_memattr mattr)
{
    uint64_t block_offset;
    uint32_t index_l2;
    uint32_t index_l2_last;
    uint32_t num_blocks;
    uint32_t pages;
    union lpaed *ttbl3;
    int i;
    HVMM_TRACE_ENTER();

    printh("ttbl2:%x va_offset:%x pa:%x size:%d\n",
            (uint32_t) ttbl2, (uint32_t) va_offset, (uint32_t) pa, size);
    index_l2 = va_offset >> LPAE_BLOCK_L2_SHIFT;
    block_offset = va_offset & LPAE_BLOCK_L2_MASK;
    printh("- index_l2:%d block_offset:%x\n",
            index_l2, (uint32_t) block_offset);
    /* head < BLOCK */
    if (block_offset) {
        uint64_t offset;
        offset = block_offset >> LPAE_PAGE_SHIFT;
        pages = size >> LPAE_PAGE_SHIFT;
        if (pages > VMM_L3_PTE_NUM)
            pages = VMM_L3_PTE_NUM;

        ttbl3 = TTBL_L3(ttbl2, index_l2);
        vmm_ttbl3_map(ttbl3, offset, pages, pa, mattr);
        lpaed_stage2_enable_l2_table(&ttbl2[index_l2]);
        va_offset |= ~LPAE_BLOCK_L2_MASK;
        size -= pages * LPAE_PAGE_SIZE;
        pa += pages * LPAE_PAGE_SIZE;
        index_l2++;
    }
    /* body : n BLOCKS */
    if (size > 0) {
        num_blocks = size >> LPAE_BLOCK_L2_SHIFT;
        index_l2_last = index_l2 + num_blocks;
        printh("- index_l2_last:%d num_blocks:%d size:%d\n"
                index_l2_last, (uint32_t) num_blocks, size);
        for (i = index_l2; i < index_l2_last; i++) {
            lpaed_stage2_enable_l2_table(&ttbl2[i]);
            vmm_ttbl3_map(TTBL_L3(ttbl2, i), 0, VMM_L3_PTE_NUM, pa, mattr);
            pa += LPAE_BLOCK_L2_SIZE;
            size -= LPAE_BLOCK_L2_SIZE;
        }
    }
    /* tail < BLOCK */
    if (size > 0) {
        pages = size >> LPAE_PAGE_SHIFT;
        printh("- pages:%d size:%d\n", pages, size);
        if (pages) {
            ttbl3 = TTBL_L3(ttbl2, index_l2_last);
            vmm_ttbl3_map(ttbl3, 0, pages, pa, mattr);
            lpaed_stage2_enable_l2_table(&ttbl2[index_l2_last]);
        }
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Initialize TTBL2 Entry and TTBL3 Entry
 *
 * Initilize Level2 Translation Table Entry<br>
 * and Level3 Translation Table Entries<br>
 * And Set Invalidate
 * @param lpaed_t *ttbl2 Target Level 2 Translation Table Entry Point
 * @return void
 */
static void vmm_ttbl2_init_entries(union lpaed *ttbl2)
{
    int i, j;
    HVMM_TRACE_ENTER();
    union lpaed *ttbl3;
    for (i = 0; i < VMM_L2_PTE_NUM; i++) {
        ttbl3 = TTBL_L3(ttbl2, i);
        printh("ttbl2[%d]:%x ttbl3[]:%x\n", i, &ttbl2[i], ttbl3);
        lpaed_stage2_conf_l2_table(&ttbl2[i], (uint64_t)((uint32_t) ttbl3), 0);
        for (j = 0; j < VMM_L2_PTE_NUM; j++)
            ttbl3[j].pt.valid = 0;
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Initialize TTBL2
 *
 * Initilize Level 2 Translation Table and<br>
 * Mapping Device Memory Mapping Descriptor's Information
 * @param lpaed_t *ttbl2 Target Level 2 Translation Table Pointer
 * @param struct memmap_desc *md Device Memroy Mapping Information Descriptor
 * @return void
 */
static void vmm_init_ttbl2(union lpaed *ttbl2, struct memmap_desc *md)
{
    int i = 0;
    HVMM_TRACE_ENTER();
    printh(" - ttbl2:%x\n", (uint32_t) ttbl2);
    if (((uint64_t)((uint32_t) ttbl2)) & 0x0FFFULL)
        printh(" - error: invalid ttbl2 address alignment\n");

    /* construct l2-l3 table hirerachy with invalid pages */
    vmm_ttbl2_init_entries(ttbl2);
    vmm_ttbl2_unmap(ttbl2, 0x00000000, 0x40000000);
    while (md[i].label != 0) {
        vmm_ttbl2_map(ttbl2, md[i].va, md[i].pa, md[i].size, md[i].attr);
        i++;
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Initialize All Translation Table
 *
 * Mapping Virtual Machine's Virtual Address<br>
 * to All Translation Talbe Entry<br>
 * @param lpaed_t *ttbl Target Translation Table Pointer
 * @param struct memmap_desc *mdlist[] memory mapping Descriptor List
 * @return void
 */
static void vmm_init_ttbl(union lpaed *ttbl, struct memmap_desc *mdlist[])
{
    int i = 0;
    HVMM_TRACE_ENTER();
    while (mdlist[i]) {
        struct memmap_desc *md = mdlist[i];
        if (md[0].label == 0)
            lpaed_stage2_conf_l1_table(&ttbl[i], 0, 0);
        else {
            lpaed_stage2_conf_l1_table(&ttbl[i],
                    (uint64_t)((uint32_t) TTBL_L2(ttbl, i)), 1);
            vmm_init_ttbl2(TTBL_L2(ttbl, i), md);
        }
        i++;
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Initialize MMU
 *
 * Configure VTCR(Virtualization Translation Control Register)<br>
 *
 * <b>Configuration</b><br>
 * - SL0(Starting Level)<br>
 *  - 00 : Start at Second Level<br>
 *  - 01 : Start at First Level    V<br>
 *  - 10, 11 : Reserved, UNPREDICTABLE<br>
 * - ORGN0(Outer Cacheability Attribute)<br>
 *  - 00 : Normal Memory, Outer Non-Cacheable<br>
 *  - 01 : Normal Memory, Outer Write-Back Write Allocate Cacheable<br>
 *  - 10 : Normal Memory, Outer Write-Through Cacheable<br>
 *  - 11 : Normal Memory, Outer Write-Back no Write Allocate Cacheable<br>
 * - IRGN0(Inner Cacheability Attribute)<br>
 *  - 00 : Normal Memory, Inner Non-Cacheable<br>
 *  - 01 : Normal Memory, Inner Write-Back Write Allocate Cacheable<br>
 *  - 10 : Normal Memory, Inner Write-Through Cacheable<br>
 *  - 11 : Normal Memory, Inner Write-Back no Write Allocate Cacheable<br>
 * - T0SZ(The Size Offset of the Memory Region Address by VTTBR)<br>
 *  - Base Address = (SL0 == 0 ? 14 - T0SZ : 5 - T0SZ)<br>
 * .
 * In Here, configure<br>
 *  - SL0 - Start at First Level<br>
 *  - ORGN0 - Normal Memory, Outer Write-Back no Write Allocate Cacheable<br>
 *  - IRGN0 - Normal Memory, Inner Write-Back no Write Allocate Cacheable
 *
 * @param void
 * @return void
 */
static void vmm_init_mmu(void)
{
    uint32_t vtcr, vttbr;
    HVMM_TRACE_ENTER();
    vtcr = read_vtcr();
    uart_print("vtcr:");
    uart_print_hex32(vtcr);
    uart_print("\n\r");
    /* start lookup at level 1 table */
    vtcr &= ~VTCR_SL0_MASK;
    vtcr |= (0x01 << VTCR_SL0_SHIFT) & VTCR_SL0_MASK;
    vtcr &= ~VTCR_ORGN0_MASK;
    vtcr |= (0x3 << VTCR_ORGN0_SHIFT) & VTCR_ORGN0_MASK;
    vtcr &= ~VTCR_IRGN0_MASK;
    vtcr |= (0x3 << VTCR_IRGN0_SHIFT) & VTCR_IRGN0_MASK;
    write_vtcr(vtcr);
    vtcr = read_vtcr();
    uart_print("vtcr:");
    uart_print_hex32(vtcr);
    uart_print("\n\r");
    {
        uint32_t sl0 = (vtcr & VTCR_SL0_MASK) >> VTCR_SL0_SHIFT;
        uint32_t t0sz = vtcr & 0xF;
        uint32_t baddr_x = (sl0 == 0 ? 14 - t0sz : 5 - t0sz);
        uart_print("vttbr.baddr.x:");
        uart_print_hex32(baddr_x);
        uart_print("\n\r");
    }
    /* VTTBR */
    vttbr = read_vttbr();
    uart_print("vttbr:");
    uart_print_hex64(vttbr);
    uart_print("\n\r");
    HVMM_TRACE_EXIT();
}

/*
 * Initialization of Virtual Machine Memory Management
 * Stage 2 Translation
 */
void vmm_init(void)
{
    /*
     * Initializes Translation Table for Stage2 Translation (IPA -> PA)
     */
    int i;
    HVMM_TRACE_ENTER();
    for (i = 0; i < NUM_GUESTS_STATIC; i++)
        _vmid_ttbl[i] = 0;

    _vmid_ttbl[0] = &_ttbl_guest0[0];
    _vmid_ttbl[1] = &_ttbl_guest1[0];
    /*
     * VA: 0x00000000 ~ 0x3FFFFFFF,   1GB
     * PA: 0xA0000000 ~ 0xDFFFFFFF    guest_bin_start
     * PA: 0xB0000000 ~ 0xEFFFFFFF    guest2_bin_start
     */
    guest_memory_md0[0].pa = (uint64_t)((uint32_t) &guest_bin_start);
    guest_memory_md1[0].pa = (uint64_t)((uint32_t) &guest2_bin_start);
    vmm_init_ttbl(&_ttbl_guest0[0], &guest_mdlist0[0]);
    vmm_init_ttbl(&_ttbl_guest1[0], &guest_mdlist1[0]);
    vmm_init_mmu();
    HVMM_TRACE_EXIT();
}

/* Translation Table for the specified vmid */
union lpaed *vmm_vmid_ttbl(vmid_t vmid)
{
    union lpaed *ttbl = 0;
    if (vmid < NUM_GUESTS_STATIC)
        ttbl = _vmid_ttbl[vmid];

    return ttbl;
}

/* Enable/Disable Stage2 Translation */
void vmm_stage2_enable(int enable)
{
    uint32_t hcr;
    /* HCR.VM[0] = enable */
    /* uart_print( "hcr:"); uart_print_hex32(hcr); uart_print("\n\r"); */
    hcr = read_hcr();
    if (enable)
        hcr |= (0x1);
    else
        hcr &= ~(0x1);

    write_hcr(hcr);
}

hvmm_status_t vmm_set_vmid_ttbl(vmid_t vmid, union lpaed *ttbl)
{
    uint64_t vttbr;
    /*
     * VTTBR.VMID = vmid
     * VTTBR.BADDR = ttbl
     */
    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    uart_print("current vttbr:");
    uart_print_hex64(vttbr);
    uart_print("\n\r");
#endif
    vttbr &= ~(VTTBR_VMID_MASK);
    vttbr |= ((uint64_t)vmid << VTTBR_VMID_SHIFT) & VTTBR_VMID_MASK;
    vttbr &= ~(VTTBR_BADDR_MASK);
    vttbr |= (uint32_t) ttbl & VTTBR_BADDR_MASK;
    write_vttbr(vttbr);
    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    uart_print("changed vttbr:");
    uart_print_hex64(vttbr);
    uart_print("\n\r");
#endif
    return HVMM_STATUS_SUCCESS;
}
