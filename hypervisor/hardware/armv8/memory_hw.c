#include <k-hypervisor-config.h>
#include <armv8_processor.h>
#include <arch_types.h>
#include <hvmm_trace.h>
#include <lpae.h>
#include <memory.h>
#include <log/print.h>
#include <log/uart_print.h>
#include <guest.h>
#include <smp.h>
/**
 * \defgroup Memory_Attribute_Indirection_Register (MAIR)
 *
 * It provide the memory attribute encodings corresponding to the possible
 * AttrIndx values in a Long descriptor format translation table entry for
 * stage-1 translations.
 *
 * - Only accessible from EL1 or higher.
 * AttrIndx[2] selects the appropriate MAIR
 * - AttrIndx[2] to 0 selects MAIR0
 *               to 1 selects MAIR1
 * - AttrIndx[1:0] gives the value of m in Attrm
 * - The MAIR bit assignments.
 * <pre>
 * |    |31       24|23       16|15        8|7         0|
 * |----|-----------|-----------|-----------|-----------|
 * |MAIR|   Attr3   |   Attr2   |   Attr1   |   Attr0   |
 * |----|-----------|-----------|-----------|-----------|
 * |    |63       56|55       48|47       40|39       32|
 * |----|-----------|-----------|-----------|-----------|
 * |    |   Attr7   |   Attr6   |   Attr5   |   Attr4   |
 * </pre>
 * - MAIR Attrm[7:4] encoding
 * <pre>
 * |Attrm[7:4]     |Meaning                                                    |
 * |---------------|-----------------------------------------------------------|
 * |0000           |Strongly-ordered or Device memory, see encoding Attrm[3:0] |
 * |---------------|-----------------------------------------------------------|
 * |00RW, RW not 00| - Normal memory, Outer Write-Through transient.           |
 * |---------------|-----------------------------------------------------------|
 * |0100           |Normal memory, Outer non-cacheable.                        |
 * |---------------|-----------------------------------------------------------|
 * |01RW, RW not 00| - Normal memory, Outer Write-back transient.              |
 * |---------------|-----------------------------------------------------------|
 * |10RW           |Normal memory, Outer Write-Through Cacheable, Non-transient|
 * |---------------|-----------------------------------------------------------|
 * |11RW           | Normal memory, Ouet Write-Back Cacheable, Non-transient.  |
 * </pre>
 *
 * - MAIR Attrm[3:0] encoding
 * <pre>
 * |Attrm[3:0]|When Attrm[7:4] is 0b000|When Attrm[7:4] is not 0b0000
 * |----------|------------------------|--------------------------------------|
 * |0000      |Strongly-ordered memory | UNPREDICTABLE                        |
 * |----------|------------------------|--------------------------------------|
 * |00RW,     | UNPREDICTABLE          | - Normal memory, Inner Write-Through |
 * |RW not 00 |                        |   transient.                         |
 * |----------|------------------------|--------------------------------------|
 * |0100      | Device memory          | Normal memory, Inner Non-cacheable.  |
 * |----------|------------------------|--------------------------------------|
 * |01RW,     | UNPREDICTABLE          | - Normal memory, Inner Write-Back    |
 * |RW not 00 |                        |   Transient.                         |
 * |----------|------------------------|--------------------------------------|
 * |10RW      | UNPREDICTABLE          | Normal memory, Inner Write-Through   |
 * |          |                        | Cacheable, Non-transient.            |
 * |----------|------------------------|--------------------------------------|
 * |11RW      | UNPREDICTABLE          | Normal memory, Inner Write-back      |
 * |          |                        | Cacheable, Non-transient.            |
 * </pre>
 *
 * -R, W bit mean read-allocate and wrtie-allocate.
 *
 * - Used initial value of MAIR.
 * <pre>
 * |    |31       24|23       16|15        8|7         0|
 * |----|-----------|-----------|-----------|-----------|
 * |MAIR| 1110 1110 | 1010 1010 | 0100 0100 | 0000 0000 |
 * |----|-----------|-----------|-----------|-----------|
 * |    |63       56|55       48|47       40|39       32|
 * |----|-----------|-----------|-----------|-----------|
 * |    | 1111 1111 | 0000 0000 | 0000 0000 | 0000 0100 |
 * </pre>
 * @{
 */
#define INITIAL_MAIR0VAL 0xeeaa4400
#define INITIAL_MAIR1VAL 0xff000004ULL
#define INITIAL_MAIRVAL (INITIAL_MAIR0VAL|(INITIAL_MAIR1VAL<<32))
/** @}*/

/**
 * \defgroup SCTLR (EL2, EL3)
 *
 * System Control Register.
 * The SCTLR provides the top level control of the system, including its memory
 * system.
 *
 * @{
 */
#define SCTLR_EE        (1<<25) /**< Exception Endianness. */
#define SCTLR_WXN       (1<<19) /**< Write permission emplies XN. */
#define SCTLR_I         (1<<12) /**< Instruction cache enable.  */
#define SCTLR_SA        (1<<3)  /**< Stack Alignment Check enable. */
#define SCTLR_C         (1<<2)  /**< Cache enable. */
#define SCTLR_A         (1<<1)  /**< Alignment check enable. */
#define SCTLR_M         (1<<0)  /**< MMU enable. */
#define SCTLR_BASE        0x00c50078  /**< STCLR Base address */
//#define SCTLR_EL2_BASE       0x30c51838  /**< STCLR_EL2 Base Settings */
#define SCTLR_EL2_BASE       0x30c51830  /**< STCLR_EL2 Base Settings */
/** @}*/

/**
 * \defgroup TTBR0_EL2 (HTTBR)
 * @brief Hyp Translation Table Base Register
 *
 * The HTTBR holds the base address of the translation table for the stasge-1
 * translation of memory accesses from Hyp mode.
 * @{
 */
#define TTBR0_EL2_INITVAL               0x0000000000000000ULL
#define TTBR0_EL2_BADDR_MASK            0x0000FFFFFFFFF000ULL
#define TTBR0_EL2_BADDR_SHIFT           12
/** @}*/

/**
 * \defgroup TCR_EL2 (HTCR)
 * @brief Hyp Translation Control Register
 *
 * The TCR_EL2 controls the translation table walks required for the stage-1
 * translation of memory accesses from Hyp mode, and holds cacheability and
 * shareability information for the accesses.
 * @{
 */
#define TCR_EL2_INITVAL                 0x80800000
#define TCR_EL2_TBI_MASK                0x00100000
#define TCR_EL2_TBI_SHIFT               20
#define TCR_EL2_PS_MASK                 0x00070000
#define TCR_EL2_PS_SHIFT                16
#define TCR_EL2_TG0_MASK                0x0000C000
#define TCR_EL2_TG0_SHIFT               14
#define TCR_EL2_SH0_MASK                0x00003000 /**< \ref Shareability */
#define TCR_EL2_SH0_SHIFT               12
#define TCR_EL2_ORGN0_MASK              0x00000C00 /**< \ref Outer_cacheability */
#define TCR_EL2_ORGN0_SHIFT             10
#define TCR_EL2_IRGN0_MASK              0x00000300 /**< \ref Inner_cacheability */
#define TCR_EL2_IRGN0_SHIFT             8
#define TCR_EL2_T0SZ_MASK               0x0000003F
#define TCR_EL2_T0SZ_SHIFT              0
/** @} */

/* EL2 Stage 1 Level 0 */
#define HMM_L0_PTE_NUM  512
/* EL2 Stage 1 Level 1 */
#define HMM_L1_PTE_NUM  512

/* EL2 Stage 1 Level 2 */
#define HMM_L2_PTE_NUM  512

/* EL2 Stage 1 Level 3 */
#define HMM_L3_PTE_NUM  512

#define HEAP_ADDR (CFG_MEMMAP_MON_OFFSET + 0x02000000)
#define HEAP_SIZE 0x0D000000

#define ENTRY_MASK 0x1FF
#define L1_ENTRY_MASK (ENTRY_MASK << L1_SHIFT)
#define L1_SHIFT 30

#define L1_REMAIN_MASK  0x3FFFF000

#define L2_ENTRY_MASK (ENTRY_MASK << L2_SHIFT) 
#define L2_SHIFT 21

#define L2_REMAIN_MASK  0x1FF000

#define L3_ENTRY_MASK (ENTRY_MASK << L3_SHIFT)
#define L3_SHIFT 12

#define HEAP_END_ADDR (HEAP_ADDR + HEAP_SIZE)
#define NALLOC 1024

/* Stage 2 Level 0 */
#define VMM_L0_PTE_NUM          512 
/* Stage 2 Level 1 */
#define VMM_L1_PTE_NUM          512
/* Stage 2 Level 2 */
#define VMM_L2_PTE_NUM          512
#define VMM_L3_PTE_NUM          512

/**
 * \defgroup VTTBR
 *
 * Virtualization Translation Table Base Register(VTTBR) is almost the same as
 * \ref TTBRx "64bit TTBRx". It is just different from ASID to VMID.
 *
 * - VMID[55:48]
 *   - The VMID for the translation table.
 * @{
 */
#define VTTBR_INITVAL                                   0x0000000000000000ULL
#define VTTBR_VMID_MASK                                 0x00FF000000000000ULL
#define VTTBR_VMID_SHIFT                                48
#define VTTBR_BADDR_MASK                                0x0000FFFFFFFFF000ULL
#define VTTBR_BADDR_SHIFT                               12
/** @} */

/**
 * \defgroup VTCR
 *
 * Virtualization Translation Control Register(VTCR) controls the translation
 * table walks required for stage-2 translation of memory accesses from
 * non-secure modes other than hyp mode, and holds cacheability and
 * shareability information for the acceeses.
 *
 * - bit[31]
 *   - UNK/SBOP.
 * - PS[18:16]
 *   - Physical Address Size
 * - TG0[15:14]
 *   - Granule size
 * - SH0[13:12]
 *   - Shareability attribute.
 *   - \ref Shareability
 * - ORGN0[11:10]
 *   - Outer cacheability attribute for memory.
 *   - \ref Outer_cacheability
 * - IRGN0[9:8]
 *   - Inner cacheability attribute for memory.
 *   - \ref Inner_cacheability
 * - SL0[7:6]
 *   - Starting level for translation table walks using VTTBR.
 * <pre>
 * SL0 Meaning
 * - 00 : Start at second level
 * - 01 : Start at first level
 * - 20 : Start at zero level
 * </pre>
 * - S[4]
 *   - Sign extension bit. this bit must be programmed to the value of the
 *     T0SZ[3].
 * - T0SZ[3:0]
 *   - The size offset of the memory region addressed by VTTBR.
 * @{
 */
#define VTCR_INITVAL                                    0x80000000
#define VTCR_PS_MASK                                    0x00070000
#define VTCR_PS_SHIFT                                   16
#define VTCR_TG0_MASK                                   0x0000C000
#define VTCR_TG0_SHIFT                                  14
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
#define VTCR_T0SZ_MASK                                  0x0000003f
#define VTCR_T0SZ_SHIFT                                 0
/** @} */

#define VTCR_TG0_4K     0x0
#define VTCR_TG0_64k    0x1
#define VTCR_TG0_16K    0x2
/*
 * Stage 2 Translation Table, look up begins at second level
 * VTTBR.BADDR[47:x]: x=14, VTCR.T0SZ = 0, 2^32 input address range,
 * VTCR.SL0 = 0(2nd), 16KB aligned base address
 * Statically allocated for now
 */

static union lpaed *_vmid_ttbl[NUM_GUESTS_STATIC];
/*
 * TODO: if you change the static variable, you will meet the system fault.
 * We don't konw about this issue, so we will checking this later time.
 */
union lpaed
_ttbl0_guest[NUM_GUESTS_STATIC][VMM_L0_PTE_NUM]\
                __attribute((__aligned__(4096)));
union lpaed // 2 ~ 255 is empty space, 0-1, 256-263
_ttbl1_guest[NUM_GUESTS_STATIC][VMM_L1_PTE_NUM]\
                __attribute((__aligned__(4096)));
/* ttbl2 */
union lpaed
_ttbl2_guest_dev[NUM_GUESTS_STATIC][VMM_L2_PTE_NUM* \
        (CFG_MEMMAP_DEV_AREA/SZ_2G)]\
        __attribute((__aligned__(4096)));
union lpaed
_ttbl2_guest_mem[NUM_GUESTS_STATIC][VMM_L2_PTE_NUM* \
        (CFG_MEMMAP_GUEST_SIZE/SZ_1G)]\
        __attribute((__aligned__(4096)));
/* ttbl3 */
union lpaed
_ttbl3_guest_dev[NUM_GUESTS_STATIC][VMM_L3_PTE_NUM*VMM_L2_PTE_NUM* \
        (CFG_MEMMAP_DEV_AREA/SZ_2G)]\
        __attribute((__aligned__(4096)));
union lpaed
_ttbl3_guest_mem[NUM_GUESTS_STATIC][VMM_L3_PTE_NUM*VMM_L2_PTE_NUM* \
        (CFG_MEMMAP_GUEST_SIZE/SZ_1G)]\
        __attribute((__aligned__(4096)));

static union lpaed _hmm_pgtable/*[HMM_L0_PTE_NUM]*/ \
                __attribute((__aligned__(4096)));
static union lpaed _hmm_pgtable_l1[HMM_L1_PTE_NUM] \
                __attribute((__aligned__(4096)));
static union lpaed _hmm_pgtable_l2[CFG_MEMMAP_PHYS_SIZE / SZ_1G] \
                 [HMM_L2_PTE_NUM] \
                __attribute((__aligned__(4096)));
static union lpaed _hmm_pgtable_l3[CFG_MEMMAP_PHYS_SIZE / SZ_1G] \
                 [HMM_L2_PTE_NUM][HMM_L3_PTE_NUM] \
                __attribute((__aligned__(4096)));


/* used malloc, free, sbrk */
union header {
    struct {
        union header *ptr; /* next block if on free list */
        unsigned int size; /* size of this block */
    } s;
    /* force align of blocks */
    long x;
};

/* AArch64 Memory Management Feature Register0 */
static uint64_t id_aa64mmfr0_el1;

#define ID_AA64MMFR0_PARange    (0xf << 0)
#define ID_AA64MMFR0_ASIDBits   (0xf << 4)
#define ID_AA64MMFR0_BigEnd     (0xf << 8)
/* free list block header */

static uint64_t mm_break; /* break point for sbrk()  */
static uint64_t mm_prev_break; /* old break point for sbrk() */
static uint64_t last_valid_address; /* last mapping address */
static union header freep_base; /* empty list to get started */
static union header *freep; /* start of free list */

/**
 * @brief Initilization of heap memory region.
 *
 * Initializes the configuration variable of heap region for malloc operation.
 * - mm_break = mm_prev_break = last_valid_address = HEAD_ADDR
 *
 * @return void
 */
static void host_memory_heap_init(void)
{
    mm_break = HEAP_ADDR;
    mm_prev_break = HEAP_ADDR;
    last_valid_address = HEAP_ADDR;
    freep = 0;
}

/**
 * @brief Flush the TLB
 *
 * Invalidate entire unified TLB.
 *
 * @return void
 */
static void host_memory_flushTLB(void)
{
    /* Invalidate all TLB, EL2 */
    invalidate_tlb(alle2);

    asm volatile("dsb sy");
    asm volatile("isb");
}

/**
 * @brief Returns the level 3 table entry.
 *
 * Returns the level 3 translation table descriptor by matching the virtual
 * address and number of pages. If delivered virtual address is over the max
 * memory size, returns zero value.
 *
 * @param virt Virtual address.
 * @param npages Number of pages.
 * @return The level 3 table entry.
 */
static union lpaed *host_memory_get_l3_table_entry(unsigned long virt,
                unsigned long npages)
{
    int l1_index = (virt >> L1_SHIFT) & L1_ENTRY_MASK;
    int l2_index = (virt >> L2_SHIFT) & L2_ENTRY_MASK;
    int l3_index = (virt >> L3_SHIFT) & L3_ENTRY_MASK;
    int maxsize = ((HMM_L2_PTE_NUM * HMM_L3_PTE_NUM) - \
            ((l2_index + 1) * (l3_index + 1)) + 1);
    if (maxsize < npages) {
        printh("%s[%d] : Map size \"pages\" is exceeded memory size\n",
                __func__, __LINE__);
        if (maxsize > 0)
            printh("%s[%d] : Available pages are %d\n", maxsize);
        else
            printh("%s[%d] : Do not have available pages for map\n");
        return 0;
    }
    return &_hmm_pgtable_l3[l1_index][l2_index][l3_index];
}

/**
 * @brief Unmaps level3 table entry in virtual address.
 *
 * Obtains the level3 table entry by using hmm_get_l3_table_entry function.
 * And disables the valid bit and table bit of the entry.
 *
 * @param virt Virtual address.
 * @param npages Number of pages.
 * @return void
 */
#if 0 /* unused */
static void host_memory_umap(unsigned long virt, unsigned long npages)
{
    int  i;
    union lpaed *map_table_p = host_memory_get_l3_table_entry(virt, npages);
    for (i = 0; i < npages; i++)
        lpaed_guest_stage1_disable_l3_table(&map_table_p[i]);
    host_memory_flushTLB();
}
#endif

/**
 * @brief Maps virtual address to level 3 table entries.
 *
 * Mapping the physical address to level 3 table entries.
 * This entries are obtained by given virtual address.
 * And then configure this entries valid bit to 1.
 *
 * @param phys Target physical address.
 * @param virt Target virtal address.
 * @param npages Number of pages.
 * @return void
 */
static void host_memory_map(unsigned long phys, unsigned long virt,
        unsigned long npages)
{
    int i;
    union lpaed *map_table_p = host_memory_get_l3_table_entry(virt, npages);
    for (i = 0; i < npages; i++)
        lpaed_guest_stage1_conf_l3_table(&map_table_p[i], (uint64_t)phys, 1);
    host_memory_flushTLB();
}

/**
 * @brief General-purpose sbrk, basic memory management system calls.
 *
 * General-purpose sbrk, basic memory management system calls.
 * If there was no heap memory space, returns the value -1.
 *
 * @param  incr Size of memory wanted.
 * @return Pointer of allocated heap memory.
 */
static void *host_memory_sbrk(unsigned int incr)
{
    uint64_t required_addr;
    uint64_t virt;
    unsigned long required_pages = 0;

    mm_prev_break = mm_break;
    virt = mm_break;
    mm_break += incr;
    if (mm_break > last_valid_address) {
        required_addr = mm_break - last_valid_address;
        for (; required_addr > 0x0; required_addr -= 0x1000) {
            if (last_valid_address + 0x1000 > HEAP_END_ADDR) {
                printh("%s[%d] required address is exceeded heap memory size\n",
                        __func__, __LINE__);
                return (void *)-1;
            }
            last_valid_address += 0x1000;
            required_pages++;
        }
        host_memory_map(virt, virt, required_pages);
    }
    return (void *)mm_prev_break;
}

/**
 * @brief Unmaps level3 table entry in virtual address.
 *
 * Obtains the level3 table entry by using host_memory_get_l3_table_entry
 * function. And disables the valid bit and table bit of the entry.
 *
 * @param virt Virtual address.
 * @param npages Number of pages.
 * @return void
 */
static void host_memory_free(void *ap)
{
    union header *bp, *p;
    bp = (union header *)ap - 1; /* point to block header */
    for (p = freep; !(bp > p && bp  < p->s.ptr); p = p->s.ptr) {
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break; /* freed block at start or end of arena */
    }
    if (bp + bp->s.size == p->s.ptr) { /* join to upper nbr */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) {      /* join to lower nbr */
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    freep = p;
}

/**
 * @brief Obtains the storage from the heap memory.
 *
 * If wanted size of block is not bigger then NALLOC value, this function
 * obtains minimally size of memory.(1024 Bytes)
 * Makes free memory area by calling free function after obtain the memory.
 *
 * @param nu Number of units, allocate size.
 * @return Pointer of free list block header & memory area.
 *         The size of this memory is bigger than 1KB.
 */
static union header *morecore(unsigned int nu)
{
    char *cp;
    union header *up;
    if (nu < NALLOC)
        nu = NALLOC;
    cp = host_memory_sbrk(nu * sizeof(union header));
    if (cp == (char *) -1) /* no space at all */
        return 0;
    up = (union header *)cp;
    up->s.size = nu;
    host_memory_free((void *)(up+1));
    return freep;
}

/**
 * @brief Hyp mode general-purpose storage allocator.
 *
 * Obtains the storage from hyp mode heap memory.
 * - First, search free block list.
 * - Second, if free block list has no list or not enough size of list,
 *   Obtains storage from heap memory.
 * - Third, there are list has enough size, return list's pointer.
 *
 * @param size Size of space
 * @return Allocated space
 */
static void *host_memory_malloc(unsigned long size)
{
    union header *p, *prevp;
    unsigned int nunits;
    nunits = (size + sizeof(union header) - 1)/sizeof(union header) + 1;
    if (nunits < 2)
        return 0;
    prevp = freep;
    if ((prevp) == 0) { /* no free list yet */
        freep_base.s.ptr = freep = prevp = &freep_base;
        freep_base.s.size = 0;
    }
    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) { /* big enough */
            if (p->s.size == nunits) /* exactly */
                prevp->s.ptr = p->s.ptr;
            else {                    /* allocate tail end */
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p+1);
        }
        if (p == freep) {  /* wrapped around free list */
            p = morecore(nunits);
            if (p == 0)
                return 0;  /* none avaliable memory left */
        }
    }
}

/**
 * @brief Initialize delivered ttbl3 descriptors.
 *
 * Initialize ttbl3 descriptors and unmap the descriptors.
 * Finally, map the ttbl3 descriptors by memory map descriptors.
 *
 * @param *ttbl3 Level 3 translation table descriptor.
 * @param *md Device memory map descriptor.
 * @return void
 */
static void guest_memory_init_ttbl3(union lpaed *ttbl3,
        struct memmap_desc md)
{
    int l3_idx;

    l3_idx = (md.va & L3_ENTRY_MASK) >> L3_SHIFT;
    while(md.size)
    {
        if(ttbl3[l3_idx].pt.valid) {
            md.size -= SZ_4K;
            md.pa += SZ_4K;
            md.va += SZ_4K;
            l3_idx++;
            continue;
        }
        lpaed_guest_stage2_map_page ( &ttbl3[l3_idx], md.pa, md.attr);
        md.size -= SZ_4K;
        md.pa += SZ_4K;
        md.va += SZ_4K;
        l3_idx++;
    }
}


/**
 * @brief Initialize delivered ttbl2 descriptors.
 *
 * Initialize ttbl2 descriptors and unmap the descriptors.
 * Finally, map the ttbl2 descriptors by memory map descriptors.
 *
 * @param *ttbl2 Level 2 translation table descriptor.
 * @param *md Device memory map descriptor.
 * @return void
 */
static void guest_memory_init_ttbl2(union lpaed *ttbl2,
        struct memmap_desc md, int l1_idx,int gid)
{
    int l2_idx, l2_remain;
    union lpaed *ttbl3;
    HVMM_TRACE_ENTER();
    if(gid)
    printh(" - ttbl2:%x\n", (uint64_t) ttbl2);
    if (((uint64_t)((uint64_t) ttbl2)) & 0x0FFFULL)
        printh(" - error: invalid ttbl2 address alignment\n");
    l2_idx = (md.va & L2_ENTRY_MASK) >> L2_SHIFT;
    printh("l2_idx:%d\n", l2_idx);

    if (md.va < CFG_MEMMAP_PHYS_START) // device area
        ttbl3 = &_ttbl3_guest_dev[gid][l1_idx*512*512];
    else // memory area
        ttbl3 = &_ttbl3_guest_mem[gid][l1_idx*512*512];

    while (md.size) {
        l2_remain = (md.va & ~L2_REMAIN_MASK) + SZ_2M - md.va;
        if( md.size < l2_remain) {
            lpaed_guest_stage2_conf_l2_table(&ttbl2[l2_idx],
                    (uint64_t) ((uint64_t) &ttbl3[l2_idx *512]), 1);
            guest_memory_init_ttbl3 (&ttbl3[l2_idx * 512],
                    md);

            md.size -= md.size;
        } else if (l2_remain == SZ_2M)
        { // fit on L2 block, l2_remain >= size
            lpaed_guest_stage2_map_page (
                    &ttbl2[l2_idx], md.pa, md.attr);
            ttbl2[l2_idx].pt.table = 0; // l2 block

            // sync descriptor
            md.va += l2_remain;
            md.pa += l2_remain;
            md.size -= l2_remain;
        } else { // bigger then L2 block
            struct memmap_desc temp_md = md;
            temp_md.size = l2_remain;
            lpaed_guest_stage2_conf_l2_table(&ttbl2[l2_idx],
                    (uint64_t)((uint64_t) &ttbl3[l2_idx*512]), 1);
            guest_memory_init_ttbl3 (&ttbl3[l2_idx * 512],
                    temp_md);

            // sync descriptor
            md.va += l2_remain;
            md.pa += l2_remain;
            md.size -= l2_remain;
        }
        l2_idx++;
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Configure stage-2 translation table descriptors of guest.
 *
 * Configures the level 1 translation table based on the memory descriptor.
 *
 * @param *ttbl1 Target level1 translation table descriptor.
 * @param *mdlist[] Memory map descriptor list.
 * @return void
 */
static void guest_memory_init_ttbl1(union lpaed *ttbl1,
            struct memmap_desc *mdlist, int gid)
{
    int i = 0;
    HVMM_TRACE_ENTER();
    while (mdlist[i].label) {
        struct memmap_desc md = mdlist[i];
        uint32_t l1_idx, l1_remain;
        union lpaed *ttbl2;

        // fit to minimum size
        if ( md.size < SZ_4K)
            md.size = SZ_4K;
        //set alignment
        md.va &= ~ENTRY_MASK;
        md.pa &= ~ENTRY_MASK;

        if (mdlist[i].label == 0) // end of md list
            break;

        l1_idx = (md.va & L1_ENTRY_MASK) >> L1_SHIFT;

        printh("label:%s, l1_idx:%d\n",md.label, l1_idx);
        if (md.va < CFG_MEMMAP_PHYS_START) // device area
        {
            ttbl2 = _ttbl2_guest_dev[gid];

            while (md.size) {
                l1_remain = (md.va & ~L1_REMAIN_MASK) + SZ_1G - md.va;
                printh("l1_remain : %x, size: %x\n", l1_remain, md.size);
                if (md.size < l1_remain ) {
                    lpaed_guest_stage2_conf_l1_table(&ttbl1[l1_idx],
                            (uint64_t) ((uint64_t) &ttbl2[l1_idx*512]), 1);
                    guest_memory_init_ttbl2 (&ttbl2[l1_idx*512],
                            md, l1_idx, gid);

                    md.size -= md.size;
                } else if ( l1_remain == SZ_1G)
                {// fit on L1 block, l1_remain >= size
                    lpaed_guest_stage2_map_page (
                            &ttbl1[l1_idx], md.pa, md.attr);
                    ttbl1[l1_idx].pt.table = 0; // l1 block

                    // sync descriptor
                    md.va += l1_remain;
                    md.pa += l1_remain;
                    md.size -= l1_remain;
                } else { // bigger then L1 block
                    struct memmap_desc temp_md = md;
                    temp_md.size = l1_remain;
                    lpaed_guest_stage2_conf_l1_table(&ttbl1[l1_idx],
                            (uint64_t) ((uint64_t) &ttbl2[l1_idx*512]), 1);
                    guest_memory_init_ttbl2 (&ttbl2[l1_idx*512],
                            temp_md, l1_idx, gid);

                    // sync descriptor
                    md.va += l1_remain;
                    md.pa += l1_remain;
                    md.size -= l1_remain;
                }
                l1_idx++;
            }
        }
        else {// memory area
            ttbl2 = _ttbl2_guest_mem[gid];
            while (md.size) {
                l1_remain = (md.va & ~L1_REMAIN_MASK) + SZ_1G - md.va;
                printh("l1_remain : %x, size: %x\n", l1_remain, md.size);
                if (md.size <l1_remain ) {
                    lpaed_guest_stage2_conf_l1_table(&ttbl1[l1_idx],
                            (uint64_t)((uint64_t) 
                    &ttbl2[(l1_idx-(CFG_MEMMAP_PHYS_START/SZ_1G))*512]), 1);
                    guest_memory_init_ttbl2
                        (&ttbl2[(l1_idx-(CFG_MEMMAP_PHYS_START/SZ_1G))*512],
                         md, (l1_idx-(CFG_MEMMAP_PHYS_START/SZ_1G)), gid);

                    md.size -= md.size;
                } else if ( l1_remain == SZ_1G) {// fit on L1 block
                    lpaed_guest_stage2_map_page (
                            &ttbl1[l1_idx], md.pa, md.attr);
                    ttbl1[l1_idx].pt.table = 0; // l1 block

                    // sync descriptor
                    md.va += l1_remain;
                    md.pa += l1_remain;
                    md.size -= l1_remain;
                } else { // bigger then L1 block
                    struct memmap_desc temp_md = md;
                    temp_md.size = l1_remain;
                    lpaed_guest_stage2_conf_l1_table(&ttbl1[l1_idx],
                            (uint64_t) ((uint64_t)
                    &ttbl2[(l1_idx-(CFG_MEMMAP_PHYS_START/SZ_1G))*512]), 1);
                    guest_memory_init_ttbl2
                        (&ttbl2[(l1_idx-(CFG_MEMMAP_PHYS_START/SZ_1G))*512],
                         temp_md, (l1_idx-(CFG_MEMMAP_PHYS_START/SZ_1G)), gid);

                    // sync descriptor
                    md.va += l1_remain;
                    md.pa += l1_remain;
                    md.size -= l1_remain;
                }
                l1_idx++;
            }
        }
        i++;
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Configure stage-2 translation table descriptors of guest.
 *
 * Configures the translation table based on the memory descriptor list.
 * It have to be fixed to control larger address space level0 ttbl.
 *
 * @param *ttbl0 Target level 0 translation table descriptor.
 * @param *mdlist[] Memory map descriptor list.
 * @param gid Guest identifier
 * @return void
 */

static void guest_memory_init_ttbl(union lpaed *ttbl0,
        struct memmap_desc mdlist[], int gid)
{
    lpaed_guest_stage2_conf_l0_table(ttbl0,
            (uint64_t)(uint64_t)_ttbl1_guest[gid], 1);
    guest_memory_init_ttbl1(_ttbl1_guest[gid], mdlist, gid);
}

/**
 * @brief Configures Virtualization Translation Control Register(VTCR).
 *
 * Configure translation control bits by modifing the VTCR.
 * \ref VTCR
 * .
 * - Configuration.
 *   - SL0 - Start at first level
 *   - ORGN0 - Normal memory, outer write-back no write allocate cacheable.
 *   - IRGN0 - Normal memory, inner write-back no write allocate cacheable.
 *
 * @return void
 */
struct vtcr_cfg {
    uint8_t sl0;
    uint8_t t0sz;
};

/* ps */            /* PA Size bits, Size */
struct vtcr_cfg vtcr_info[] = {
/* 0 */    {1, 32}, // 32, 4G
/* 1 */    {1, 28}, // 36, 64G
/* 2 */    {1, 24}, // 40, 1T
/* 3 */    {1, 25}, // 42, 4T
/* 4 */    {2, 20}, // 44, 16T
/* 5 */    {2, 16}, // 48, 256T
};

static void guest_memory_init_mmu(void)
{
    uint32_t vtcr_el2;
    uint64_t vttbr_el2;
    uint32_t vtcr_ps = 0;
    HVMM_TRACE_ENTER();
    vtcr_el2 = read_vtcr();
    uart_print("vtcr_el2:");
    uart_print_hex32(vtcr_el2);
    uart_print("\n\r");
    vtcr_ps = id_aa64mmfr0_el1 & ID_AA64MMFR0_PARange;
    printH("ps: %d\n",vtcr_ps);

    vtcr_el2 = VTCR_INITVAL;
    vtcr_el2 &= ~VTCR_ORGN0_MASK;
    vtcr_el2 |= (0x3 << VTCR_ORGN0_SHIFT) & VTCR_ORGN0_MASK;
    vtcr_el2 &= ~VTCR_IRGN0_MASK;
    vtcr_el2 |= (0x3 << VTCR_IRGN0_SHIFT) & VTCR_IRGN0_MASK;

    vtcr_el2 &= ~VTCR_PS_MASK;
    vtcr_el2 |= (vtcr_ps << VTCR_PS_SHIFT) & VTCR_PS_MASK;
    vtcr_el2 &= ~VTCR_TG0_MASK;
    vtcr_el2 |= (VTCR_TG0_4K << VTCR_TG0_SHIFT) & VTCR_TG0_MASK;
    vtcr_el2 &= ~VTCR_SL0_MASK;
    vtcr_el2 |= (vtcr_info[vtcr_ps].sl0 << VTCR_SL0_SHIFT) & VTCR_SL0_MASK;
    vtcr_el2 &= ~VTCR_T0SZ_MASK;
    vtcr_el2 |= (vtcr_info[vtcr_ps].t0sz << VTCR_T0SZ_SHIFT) & VTCR_T0SZ_MASK;

    write_vtcr(vtcr_el2);
    vtcr_el2 = read_vtcr();
    uart_print("vtcr_el2:");
    uart_print_hex32(vtcr_el2);
    uart_print("\n\r");
    {
        uint64_t baddr_x = 1<<((64-vtcr_info[vtcr_ps].t0sz)-26);
        uart_print("vttbr.baddr.x:");
        uart_print_hex(baddr_x);
        uart_print("\n\r");
    }
    /* VTTBR_EL2 */
    vttbr_el2 = read_vttbr();
    uart_print("vttbr_el2:");
    uart_print_hex64(vttbr_el2);
    uart_print("\n\r");
    HVMM_TRACE_EXIT();
}

/**
 * @brief Enables or disables the stage-2 MMU.
 *
 * Configures Hyper Configuration Register(HCR) to enable or disable the
 * virtualization MMU.
 *
 * @param enable Enable or disable the MMU.
 *        - 1 : Enable the MMU.
 *        - 0 : Disable the MMU.
 * @return void
 */
static void guest_memory_stage2_enable(int enable)
{
    uint64_t hcr;
    /* HCR.VM[0] = enable */
    /* uart_print( "hcr:"); uart_print_hex32(hcr); uart_print("\n\r"); */
    hcr = read_hcr();
    if (enable)
        hcr |= HCR_VM;
    else
        hcr &= ~HCR_VM;

    write_hcr(hcr);
}

/**
 * @brief Changes the stage-2 translation table base address by configuring
 *        VTTBR.
 *
 * Configures Virtualization Translation Table Base Register(VTTBR) to change
 * the guest. Change vmid and base address from received vmid and ttbl
 * address.
 *
 * @param vmid Received vmid.
 * @param ttbl Level 1 translation table of the guest.
 * @return HVMM_STATUS_SUCCESS only.
 */
static hvmm_status_t guest_memory_set_vmid_ttbl(vmid_t vmid, union lpaed *ttbl)
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
    vttbr |= (uint64_t) ttbl & VTTBR_BADDR_MASK;
    write_vttbr(vttbr);
    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    uart_print("changed vttbr:");
    uart_print_hex64(vttbr);
    uart_print("\n\r");
#endif
    return HVMM_STATUS_SUCCESS;
}

static int memory_enable(void)
{
/*
 *    MAIR_EL1
 *    MAIR_EL2
 *    TCR_EL2
 *    HCR_EL2
 *    TTBR0_EL2
 *    SCTLR_EL2
 */
    uint32_t tcr_el2, sctlr_el2;
    uint64_t mair_el1, mair_el2;
    uint64_t ttbr0_el2, hcr_el2;

    /* MAIR_EL1/MAIR_EL2 */
    uart_print(" --- MAIR ----\n\r");
    mair_el1 = read_mair();
    uart_print("mair_el1:");
    uart_print_hex64(mair_el1);
    uart_print("\n\r");

    mair_el2 = read_hmair();
    uart_print("mair_el2:");
    uart_print_hex64(mair_el2);
    uart_print("\n\r");

    write_mair(INITIAL_MAIRVAL);
    write_hmair(INITIAL_MAIRVAL);

    mair_el1 = read_mair();
    uart_print("mair_el1:");
    uart_print_hex64(mair_el1);
    uart_print("\n\r");

    mair_el2 = read_hmair();
    uart_print("mair_el2:");
    uart_print_hex64(mair_el2);
    uart_print("\n\r");

    /* TCR_EL2 */
    uart_print(" --- TCR_EL2 ----\n\r");
    tcr_el2= read_htcr();
    uart_print("tcr_el2:");
    uart_print_hex32(tcr_el2);
    uart_print("\n\r");

    /*
     * TCR_EL2 (HTCR) Configuration
     * TBI : Top Byte used in the address calculation
     * PS : AArch64_Memory Management Featre Register0.PRange bits (b011)
     * TG0 : 4KB Granule (b00)
     * SH0 : Outer Shareable (b10)
     * ORGN0 : Normal, Outer Write-Through Cacheable (b01)
     * IRGN0 : Normal, Inner Write-Through Cacheable (b01)
     * T0SZ : 0x22 (4 level translation)
     */

    tcr_el2 &= TCR_EL2_INITVAL;
    tcr_el2 |= ((id_aa64mmfr0_el1 & 0x7) << TCR_EL2_PS_SHIFT) & TCR_EL2_PS_MASK;
    tcr_el2 |= (0x3 << TCR_EL2_SH0_SHIFT) & TCR_EL2_SH0_MASK;
    tcr_el2 |= (0x1 << TCR_EL2_ORGN0_SHIFT) & TCR_EL2_ORGN0_MASK;
    tcr_el2 |= (0x1 << TCR_EL2_IRGN0_SHIFT) & TCR_EL2_IRGN0_MASK;
    tcr_el2 |= (0x16 << TCR_EL2_T0SZ_SHIFT) & TCR_EL2_T0SZ_MASK;

    write_htcr(tcr_el2);
    tcr_el2= read_htcr();
    uart_print("tcr_el2:");
    uart_print_hex32(tcr_el2);
    uart_print("\n\r");

    /* SCTLR_EL2 */
    /* i-Cache */
    /* MMU, D-cache, Write-implies-XN, Low-latency IRQs Disabled */

    sctlr_el2 = read_hsctlr();
    uart_print("sctlr_el2:");
    uart_print_hex32(sctlr_el2);
    uart_print("\n\r");
    sctlr_el2 = SCTLR_EL2_BASE;
    write_hsctlr(sctlr_el2);
    sctlr_el2 = read_hsctlr();
    uart_print("sctlr_el2:");
    uart_print_hex32(sctlr_el2);
    uart_print("\n\r");


    /* HCR_EL2 */
    hcr_el2 = read_hcr();
    uart_print("hcr_el2:");
    uart_print_hex64(hcr_el2);
    uart_print("\n\r");

    /* TTBR0_EL2 = &__hmm_pgtable */
    ttbr0_el2 = read_httbr();
    uart_print("ttbr0_el2:");
    uart_print_hex64(ttbr0_el2);
    uart_print("\n\r");
    ttbr0_el2 &= 0xFFFF000000000000ULL;
    ttbr0_el2 |= (uint64_t) &_hmm_pgtable;
    ttbr0_el2 &= TTBR0_EL2_BADDR_MASK;
    uart_print("writing ttbr0_el2:");
    uart_print_hex64(ttbr0_el2);
    uart_print("\n\r");
    write_httbr(ttbr0_el2);
    ttbr0_el2 = read_httbr();
    uart_print("read back ttbr0_el2:");
    uart_print_hex64(ttbr0_el2);
    uart_print("\n\r");

    /* Enable EL2 Stage 1 MMU */

    sctlr_el2 = read_hsctlr();
    uart_print("sctlr_el2:");
    uart_print_hex32(sctlr_el2);
    uart_print("\n\r");

    /* HSCTLR Enable MMU and D-cache */
    sctlr_el2 |= (SCTLR_M | SCTLR_C);

    write_hsctlr(sctlr_el2);

    /* Flush iCache */
    dsb(sy);
    isb();
    //asm("tlbi alle2");
    dsb(sy);
    isb();

    asm volatile(
            "ldr x0, =hyp_init_vectors;"
            "msr vbar_el2, x0"
            );

    sctlr_el2 = read_hsctlr();
    uart_print("sctlr_el2:");
    uart_print_hex32(sctlr_el2);
    uart_print("\n\r");

    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Initializes the hyp mode memory management.
 *
 * Initialize and setting the translation table descriptors of the Hyp mode.
 * - Target environment.
 *   - EL2, stage-1 translation table.
 *   - Virtual address -> Physical address
 * - Generate configurations.
 *   - Translation table level1, level2, and level3.
 * <pre>
 * Temporary Partitioning
 * Name         Physical address range    Location     Attribute Index Setting
 * Partition 0: 0x0000000000 ~ 0x007FFFFFFF - Peripheral - ATTR_IDX_DEV_SHARED
 * Partition 1: 0x0080000000 ~ 0x39FFFFFFFF - Unused     - ATTR_IDX_UNCACHED
 * Partition 2: 0x4000000000 ~ 0x41FFFFFFFF - Memory     - ATTR_IDX_UNCACHED
 * </pre>
 *
 * @return void
 */
static void host_memory_init(void)
{
    int i, j, k;
    uint64_t pa = 0x0000000000ULL;
    _hmm_pgtable = lpaed_host_l0_table((uint64_t) _hmm_pgtable_l1);

    uart_print("&_hmm_pgtable:");
    uart_print_hex64((uint64_t) &_hmm_pgtable);
    uart_print("\n\r");
    uart_print("lpaed:");
    uart_print_hex64(_hmm_pgtable.bits);
    uart_print("\n\r");

    /* _hmm_pgtable_l1 device area blocks */
    for (i = 0 ; i < 2; i++) {
        _hmm_pgtable_l1[i] = lpaed_host_l1_block(pa, ATTR_IDX_DEV_SHARED);
        pa += SZ_1G;
    }

    /* _hmm_pgtable_l1 empty area blocks, invalid */
    for ( ; i <(CFG_MEMMAP_PHYS_START/SZ_1G) ; i++) {
        //_hmm_pgtable_l1[i] = lpaed_host_l1_block(pa, ATTR_IDX_WRITEALLOC);
        _hmm_pgtable_l1[i].pt.valid = 0;
        //pa += SZ_1G;
    }
       
    pa = CFG_MEMMAP_PHYS_START;
    /* _hmm_pgtable_l1 mem refers Lv2 page table address. */
    for (; i <((CFG_MEMMAP_PHYS_START + CFG_MEMMAP_PHYS_SIZE)/SZ_1G); i++) {
//        _hmm_pgtable_l1[i] = lpaed_host_l1_block(pa, ATTR_IDX_WRITEALLOC);
//        pa += SZ_1G;
        _hmm_pgtable_l1[i] = \
                 lpaed_host_l1_table((uint64_t) _hmm_pgtable_l2[i-256]);
        uart_print("&_hmm_pgtable_l1[");
        uart_print_hex(i);
        uart_print("]:");
        uart_print_hex64((uint64_t) &_hmm_pgtable_l1[i]);
        uart_print("\n\r");
        uart_print("lpaed:");
        uart_print_hex64(_hmm_pgtable_l1[i].bits);
        uart_print("\n\r");
    }
    for (; i < HMM_L1_PTE_NUM; i++)
        _hmm_pgtable_l1[i].pt.valid = 0;

    for (i = 0; i < CFG_MEMMAP_PHYS_SIZE / SZ_1G; i++) {
        /*
         * _hvmm_pgtable_lv2[i] refers Lv3 page table address.
         * each element correspond 2MB
         */
        for (j = 0; j < HMM_L2_PTE_NUM; j++) {
            _hmm_pgtable_l2[i][j] =
                lpaed_host_l2_table((uint64_t) _hmm_pgtable_l3[i][j]);
            /* _hvmm_pgtable_lv3[i][j] refers page, that size is 4KB */
            for (k = 0; k < HMM_L3_PTE_NUM; pa += SZ_4K, k++) {
                if (pa >= HEAP_ADDR && pa < HEAP_ADDR + HEAP_SIZE) {
                    _hmm_pgtable_l3[i][j][k] =
                        lpaed_host_l3_table(pa, ATTR_IDX_WRITEALLOC, 0);
                } else { // heap area configuration
                    _hmm_pgtable_l3[i][j][k] =
                        lpaed_host_l3_table(pa, ATTR_IDX_UNCACHED, 1);
                }
            }
        }
    }
}

/*
 * @brief Initializes the virtual mode(guest mode) memory management
 * stage-2 translation.
 *
 * Configure translation tables of guests for stage-2 translation (IPA -> PA).
 *
 * - First, maps physical address of guest start address to descriptors.
 * - And configure the translation table descriptors based on the memory
 *   map descriptor lists.
 * - Last, initializes mmu.
 *
 * @return void
 */
static void guest_memory_init(struct memmap_desc *guest_map, int gid)
{
    /*
     * Initializes Translation Table for Stage2 Translation (IPA -> PA)
     */
    uint32_t sl0 = vtcr_info[id_aa64mmfr0_el1 & ID_AA64MMFR0_PARange].sl0;

    HVMM_TRACE_ENTER();

    switch (sl0)
    {
        case 0 : // start level 2, unimplemented
            printH("Unimplemented\n");
            break;
        case 1: // start level 1
            _vmid_ttbl[gid] = _ttbl1_guest[gid];
            guest_memory_init_ttbl1(_ttbl1_guest[gid], guest_map, gid);
            break;
        case 2: // start level 0
            _vmid_ttbl[gid] = _ttbl0_guest[gid];
            guest_memory_init_ttbl(_ttbl0_guest[gid], guest_map, gid);
            break;
        default:
            printh("Invalid start level\n");
    }
    HVMM_TRACE_EXIT();
}

#if 0
void temp_check()
{
    uint32_t gid = 0;
    int i, j;
    for(i=0; i< 2; i++) {
        gid = i;
        printH("_ttbl0_guest[%d] : %x value : %x\n",
                gid, _ttbl0_guest[gid], *_ttbl0_guest[gid]);
    }

    for(i=0; i< 2; i++){
        gid = i;
        printH("_ttbl1_guest[%d][0] : %x value : %x\n",
                gid, &_ttbl1_guest[gid][0], _ttbl1_guest[gid][0]);
        printH("_ttbl1_guest[%d][1] : %x value : %x\n",
                gid, &_ttbl1_guest[gid][1], _ttbl1_guest[gid][1]);
        for (j=256; j< 258; j++) {
            printH("_ttbl1_guest[%d][%d] : %x value : %x\n",
                    gid, j, &_ttbl1_guest[gid][j], _ttbl1_guest[gid][j]);
            int k;
            for(k=0; k< 2;k++) {
                printH("_ttbl2_guest_mem[%d][%d] : %x value : %x\n",
                        gid, k+ (VMM_L2_PTE_NUM * (j-256)),
                        &_ttbl2_guest_mem[gid][k +(VMM_L2_PTE_NUM * (j-256))], 
                        _ttbl2_guest_mem[gid][k +(VMM_L2_PTE_NUM * (j-256))]);
                printH("_ttbl3_guest_mem[%d][%d] : %x value : %x\n",
                        gid, 512 * (j-256) * k,
                        &_ttbl3_guest_mem[gid][512 * (j-256)* k], 
                        _ttbl3_guest_mem[gid][512 * (j-256)* k]);
            }
        }
    }
}
#endif
/**
 * @brief Configures the memory management features.
 *
 * Configure all features of the memory management.
 *
 * - Generate and confgirue hyp mode & virtual mode translation tables.
 * - Configure MAIRx, HMAIRx register.
 *   - \ref Memory_Attribute_Indirection_Register.
 *   - \ref Attribute_Indexes.
 * - Configures Hyp Translation Control Register(HTCR).
 *   - Setting
 *     - Out shareable.
 *     - Normal memory, outer write-through, cacheable.
 *     - Normal memory, inner write-back write-allocate cacheable.
 *     - T0SZ is zero.
 *   - \ref HTCR
 * - Configures System Control Register EL2(SCTLR_EL2).
 *   - i-cache and alignment checking enable.
 *   - mmu, d-cache, write-implies-xn, low-latency, IRQs disable.
 *   - \ref SCTLR
 * - Configures Hyp Translation Table Base Register(HTTBR).
 *   - Writes the _hmm_pgtable value to base address bits.
 *   - \ref HTTBR
 * - Enable MMU and D-cache in SCTLR_EL2.
 * - Initialize heap area.
 *
 * @return HVMM_STATUS_SUCCESS, Always success.
 */
static int memory_hw_init(struct memmap_desc *guest0,
            struct memmap_desc *guest1)
{
    uint32_t cpu = smp_processor_id();
    uart_print("[memory] memory_init: enter\n\r");

    id_aa64mmfr0_el1 = read_sr64(id_aa64mmfr0_el1);

    guest_memory_init(guest0, 0);
    guest_memory_init(guest1, 1);

    guest_memory_init_mmu();

    if (!cpu)
        host_memory_init();

    memory_enable();

#if 0
    temp_check();
#endif

    uart_print("[memory] memory_init: exit\n\r");
    if (!cpu) {
        uart_print("[memory] host_memory_heap_init\n\r");
        host_memory_heap_init();
    }

    return HVMM_STATUS_SUCCESS;
}

static void *memory_hw_alloc(unsigned long size)
{
    return host_memory_malloc(size);
}

static void memory_hw_free(void *ap)
{
    host_memory_free(ap);
}

/**
 * @brief Stops stage-2 translation by disabling mmu.
 *
 * We assume VTCR has been configured and initialized in the memory
 * management module.
 * - Disables stage-2 translation by HCR.vm = 0.
 */
static hvmm_status_t memory_hw_save(void)
{
    /*
     * We assume VTCR has been configured and initialized
     * in the memory management module
     */
    /* Disable Stage 2 Translation: HCR.VM = 0 */
    guest_memory_stage2_enable(0);

    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Restores translation table for the next guest and enable stage-2 mmu.
 *
 * - Chagne stage-2 translation table and vmid.
 * - Eanbles stage-2 MMU.
 *
 * @param guest Context of the next guest.
 */
static hvmm_status_t memory_hw_restore(vmid_t vmid)
{
    /*
     * Restore Translation Table for the next guest and
     * Enable Stage 2 Translation
     */
    guest_memory_set_vmid_ttbl(vmid, _vmid_ttbl[vmid]);

    guest_memory_stage2_enable(1);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t memory_hw_dump(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct memory_ops _memory_ops = {
    .init = memory_hw_init,
//    .alloc = memory_hw_alloc,
//    .free = memory_hw_free,
    .save = memory_hw_save,
    .restore = memory_hw_restore,
    .dump = memory_hw_dump,
};

struct memory_module _memory_module = {
    .name = "K-Hypervisor Memory Module",
    .author = "Kookmin Univ.",
    .ops = &_memory_ops,
};

