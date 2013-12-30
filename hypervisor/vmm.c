/* Virtual Machine Memory Management Module */

#include "hyp_config.h"
#include <arch_types.h>
#include "vmm.h"
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include <cfg_platform.h>
#include <gic_regs.h>
#include "print.h"

/* Stage 2 Level 1 */
#define VMM_L1_PTE_NUM          4
#define VMM_L1_PADDING_PTE_NUM   (512 - VMM_L1_PTE_NUM)
/* Stage 2 Level 2 */
#define VMM_L2_PTE_NUM          512
#define VMM_L3_PTE_NUM          512
#define VMM_L2L3_PTE_NUM_TOTAL  (VMM_L2_PTE_NUM * VMM_L3_PTE_NUM + VMM_L2_PTE_NUM)
#define VMM_PTE_NUM_TOTAL  (VMM_L1_PTE_NUM + VMM_L1_PADDING_PTE_NUM + VMM_L2L3_PTE_NUM_TOTAL \
                             * VMM_L1_PTE_NUM)
/* VTTBR */ 
#define VTTBR_INITVAL                                   0x0000000000000000ULL
#define VTTBR_VMID_MASK                                 0x00FF000000000000ULL
#define VTTBR_VMID_SHIFT                                48
#define VTTBR_BADDR_MASK                                0x000000FFFFFFF000ULL
#define VTTBR_BADDR_SHIFT                               12
        
/* VTCR */
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

extern uint32_t guest_bin_start;
extern uint32_t guest2_bin_start;

/*
 * Stage 2 Translation Table, look up begins at second level
 * VTTBR.BADDR[31:x]: x=14, VTCR.T0SZ = 0, 2^32 input address range, VTCR.SL0 = 0(2nd), 16KB aligned base address
 * Statically allocated for now
 */

static lpaed_t *_vmid_ttbl[NUM_GUESTS_STATIC];

static lpaed_t _ttbl_guest0[VMM_PTE_NUM_TOTAL] __attribute((__aligned__(4096)));
static lpaed_t _ttbl_guest1[VMM_PTE_NUM_TOTAL] __attribute((__aligned__(4096)));

struct memmap_desc {
    char *label;
    uint64_t va;
    uint64_t pa;
    uint32_t size;
    lpaed_stage2_memattr_t attr;
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
    { "start", 0x00000000,          0, 0x30000000, LPAED_STAGE2_MEMATTR_NORMAL_OWT | LPAED_STAGE2_MEMATTR_NORMAL_IWT },
    {       0, 0, 0, 0,  0},
};

static struct memmap_desc guest_memory_md1[] = {
    /* 256MB */
    { "start", 0x00000000,          0, 0x10000000, LPAED_STAGE2_MEMATTR_NORMAL_OWT | LPAED_STAGE2_MEMATTR_NORMAL_IWT },
    {       0, 0, 0, 0,  0},
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

/* Returns address of L3 TTBL at 'l2index' entry of L2 
    lpaed_t *TTBL_L3(lpaed_t *ttbl_l2, uint32_t index_l2);
 */
#define TTBL_L3(ttbl_l2, index_l2) (&ttbl_l2[VMM_L2_PTE_NUM + (VMM_L3_PTE_NUM * (index_l2))])
#define TTBL_L2(ttbl_l1, index_l1) (&ttbl_l1[(VMM_L1_PTE_NUM + VMM_L1_PADDING_PTE_NUM) + (VMM_L2L3_PTE_NUM_TOTAL * (index_l1))])


static void vmm_ttbl3_map( lpaed_t *ttbl3, uint64_t offset, uint32_t pages, uint64_t pa,
                            lpaed_stage2_memattr_t mattr ) 
{
    int index_l3 = 0;
    int index_l3_last = 0;

 
    printh( "%s[%d]: ttbl3:%x offset:%x pte:%x pages:%d, pa:%x\n", __FUNCTION__, __LINE__, (uint32_t) ttbl3, (uint32_t) offset, &ttbl3[offset], pages, (uint32_t) pa);
    /* Initialize the address spaces with 'invalid' state */

    index_l3 = offset;
    index_l3_last = index_l3 + pages;

    for( ; index_l3 < index_l3_last; index_l3++ ) {
        lpaed_stage2_map_page(&ttbl3[index_l3], pa, mattr );
        pa += LPAE_PAGE_SIZE;
    }

}

static void vmm_ttbl3_unmap( lpaed_t *ttbl3, uint64_t offset, uint32_t pages)
{
    int index_l3 = 0;
    int index_l3_last = 0;

    /* Initialize the address spaces with 'invalid' state */

    index_l3 = offset >> LPAE_PAGE_SHIFT;
    index_l3_last = index_l3 + pages;

    for( ; index_l3 < index_l3_last; index_l3++ ) {
        ttbl3[index_l3].pt.valid = 0;
    }
}

/*
 * va_offset: 0 ~ (1GB - size), start contiguous virtual address within level 1 block (1GB), 
 *      L2 lock size(2MB) aligned
 * size: <= 1GB
 *      page size aligned
 */
static void vmm_ttbl2_unmap( lpaed_t *ttbl2, uint64_t va_offset, uint32_t size)
{
    int index_l2 = 0;
    int index_l2_last = 0;
    int num_blocks = 0;

    /* Initialize the address spaces with 'invalid' state */

    num_blocks = size >> LPAE_BLOCK_L2_SHIFT;
    index_l2 = va_offset >> LPAE_BLOCK_L2_SHIFT;
    index_l2_last = num_blocks;

    for( ; index_l2 < index_l2_last; index_l2++ ) {
        ttbl2[index_l2].pt.valid = 0;
    }

    size &= LPAE_BLOCK_L2_MASK;
    if ( size ) {
        // last partial block
        lpaed_t *ttbl3 = TTBL_L3(ttbl2, index_l2);
        vmm_ttbl3_unmap( ttbl3, 0x00000000, size >> LPAE_PAGE_SHIFT);
    }
}

static void vmm_ttbl2_map(lpaed_t *ttbl2, uint64_t va_offset, uint64_t pa, uint32_t size, lpaed_stage2_memattr_t mattr)
{
    uint64_t block_offset;
    uint32_t index_l2;
    uint32_t index_l2_last;
    uint32_t num_blocks;
    uint32_t pages;
    lpaed_t *ttbl3;
    int i;

    HVMM_TRACE_ENTER();
    printh( "ttbl2:%x va_offset:%x pa:%x size:%d\n", (uint32_t) ttbl2, (uint32_t) va_offset, (uint32_t) pa, size);

    index_l2 = va_offset >> LPAE_BLOCK_L2_SHIFT;
    block_offset = va_offset & LPAE_BLOCK_L2_MASK;
    printh( "- index_l2:%d block_offset:%x\n", index_l2, (uint32_t) block_offset);
    /* head < BLOCK */
    if ( block_offset ) {
        uint64_t offset;
        offset = block_offset >> LPAE_PAGE_SHIFT;
        pages = size >> LPAE_PAGE_SHIFT;
        if ( pages > VMM_L3_PTE_NUM ) {
            pages = VMM_L3_PTE_NUM;
        }
        ttbl3 = TTBL_L3(ttbl2, index_l2 );
        vmm_ttbl3_map( ttbl3, offset, pages, pa, mattr );
        lpaed_stage2_enable_l2_table( &ttbl2[index_l2] );

        va_offset |= ~LPAE_BLOCK_L2_MASK;
        size -= pages * LPAE_PAGE_SIZE;
        pa += pages * LPAE_PAGE_SIZE;
        index_l2 ++;
    }

    /* body : n BLOCKS */
    if ( size > 0 ) {
        num_blocks = size >> LPAE_BLOCK_L2_SHIFT;
        index_l2_last = index_l2 + num_blocks;
        printh( "- index_l2_last:%d num_blocks:%d size:%d\n", index_l2_last, (uint32_t) num_blocks, size);

        for( i = index_l2; i < index_l2_last; i++ ) {
            lpaed_stage2_enable_l2_table( &ttbl2[i] );
            vmm_ttbl3_map( TTBL_L3(ttbl2, i), 0, VMM_L3_PTE_NUM, pa, mattr );
            pa += LPAE_BLOCK_L2_SIZE;
            size -= LPAE_BLOCK_L2_SIZE;
        }
    }

    /* tail < BLOCK */
    if ( size > 0) {
        pages = size >> LPAE_PAGE_SHIFT;
        printh( "- pages:%d size:%d\n", pages, size);
        if ( pages ) {
            ttbl3 = TTBL_L3(ttbl2, index_l2_last);
            vmm_ttbl3_map( ttbl3, 0, pages, pa, mattr );
            lpaed_stage2_enable_l2_table( &ttbl2[index_l2_last] );
        }
    }
    HVMM_TRACE_EXIT();
}

static void vmm_ttbl2_init_entries(lpaed_t *ttbl2)
{
    int i, j;
    HVMM_TRACE_ENTER();

    lpaed_t *ttbl3;
    for( i = 0; i < VMM_L2_PTE_NUM; i++ ) {
        ttbl3 = TTBL_L3(ttbl2, i);
        printh("ttbl2[%d]:%x ttbl3[]:%x\n", i, &ttbl2[i], ttbl3 );
        lpaed_stage2_conf_l2_table( &ttbl2[i], (uint64_t) ((uint32_t) ttbl3), 0);
        for( j = 0; j < VMM_L2_PTE_NUM; j++) {
            ttbl3[j].pt.valid = 0;
        }
    }        

    HVMM_TRACE_EXIT();
}


static void vmm_init_ttbl2(lpaed_t *ttbl2, struct memmap_desc *md)
{
    int i = 0;
    HVMM_TRACE_ENTER();
    printh( " - ttbl2:%x\n", (uint32_t) ttbl2 );
    if ( ((uint64_t) ( (uint32_t) ttbl2) ) & 0x0FFFULL ) {
        printh( " - error: invalid ttbl2 address alignment\n" );
    }

    /* construct l2-l3 table hirerachy with invalid pages */
    vmm_ttbl2_init_entries(ttbl2);

    vmm_ttbl2_unmap( ttbl2, 0x00000000, 0x40000000 );

    while(md[i].label != 0) {
        vmm_ttbl2_map(ttbl2, md[i].va, md[i].pa, md[i].size, md[i].attr );
        i++;
    }
    HVMM_TRACE_EXIT();
}

static void vmm_init_ttbl(lpaed_t *ttbl, struct memmap_desc *mdlist[])
{
    int i = 0;
    HVMM_TRACE_ENTER();

    while(mdlist[i]) {
        struct memmap_desc *md = mdlist[i];

        if ( md[0].label == 0 ) {
            lpaed_stage2_conf_l1_table(&ttbl[i], 0, 0);
        } else {
            lpaed_stage2_conf_l1_table(&ttbl[i], (uint64_t) ((uint32_t) TTBL_L2(ttbl, i)), 1);
            vmm_init_ttbl2(TTBL_L2(ttbl, i), md);
        }
        i++;
    }

    HVMM_TRACE_EXIT();
}


static void vmm_init_mmu(void)
{
    uint32_t vtcr, vttbr;

    HVMM_TRACE_ENTER();

    vtcr = read_vtcr(); uart_print( "vtcr:"); uart_print_hex32(vtcr); uart_print("\n\r");

    // start lookup at level 1 table
    vtcr &= ~VTCR_SL0_MASK;
    vtcr |= (0x01 << VTCR_SL0_SHIFT) & VTCR_SL0_MASK;
    vtcr &= ~VTCR_ORGN0_MASK;
    vtcr |= (0x3 << VTCR_ORGN0_SHIFT) & VTCR_ORGN0_MASK;
    vtcr &= ~VTCR_IRGN0_MASK;
    vtcr |= (0x3 << VTCR_IRGN0_SHIFT) & VTCR_IRGN0_MASK;
    write_vtcr(vtcr);
    vtcr = read_vtcr(); uart_print( "vtcr:"); uart_print_hex32(vtcr); uart_print("\n\r");
    {
        uint32_t sl0 = (vtcr & VTCR_SL0_MASK) >> VTCR_SL0_SHIFT;
        uint32_t t0sz = vtcr & 0xF;
        uint32_t baddr_x = (sl0 == 0 ? 14 - t0sz : 5 - t0sz);
        uart_print( "vttbr.baddr.x:"); uart_print_hex32(baddr_x); uart_print("\n\r");
    }
// VTTBR
    vttbr = read_vttbr(); uart_print( "vttbr:" ); uart_print_hex64(vttbr); uart_print("\n\r");

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
    for( i = 0; i < NUM_GUESTS_STATIC; i++ ) {
        _vmid_ttbl[i] = 0;
    }

    _vmid_ttbl[0] = &_ttbl_guest0[0];
    _vmid_ttbl[1] = &_ttbl_guest1[0];


    /*
     * VA: 0x00000000 ~ 0x3FFFFFFF,   1GB
     * PA: 0xA0000000 ~ 0xDFFFFFFF    guest_bin_start
     * PA: 0xB0000000 ~ 0xEFFFFFFF    guest2_bin_start
     */

    guest_memory_md0[0].pa = (uint64_t) ((uint32_t) &guest_bin_start);
    guest_memory_md1[0].pa = (uint64_t) ((uint32_t) &guest2_bin_start);

    vmm_init_ttbl(&_ttbl_guest0[0], &guest_mdlist0[0]);
    vmm_init_ttbl(&_ttbl_guest1[0], &guest_mdlist1[0]);
   
    vmm_init_mmu();

    HVMM_TRACE_EXIT();
}

/* Translation Table for the specified vmid */
lpaed_t *vmm_vmid_ttbl(vmid_t vmid)
{
    lpaed_t *ttbl = 0;
    if ( vmid < NUM_GUESTS_STATIC ) {
        ttbl = _vmid_ttbl[vmid];
    }
    return ttbl;
}

/* Enable/Disable Stage2 Translation */
void vmm_stage2_enable(int enable)
{
    uint32_t hcr;

    // HCR.VM[0] = enable
    hcr = read_hcr(); //uart_print( "hcr:"); uart_print_hex32(hcr); uart_print("\n\r");
    if ( enable ) {
        hcr |= (0x1);
    } else {
        hcr &= ~(0x1);
    }
    write_hcr( hcr );
}

hvmm_status_t vmm_set_vmid_ttbl( vmid_t vmid, lpaed_t *ttbl )
{
    uint64_t vttbr;

    /* 
     * VTTBR.VMID = vmid
     * VTTBR.BADDR = ttbl
     */
    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    uart_print( "current vttbr:" ); uart_print_hex64(vttbr); uart_print("\n\r");
#endif
    vttbr &= ~(VTTBR_VMID_MASK);
    vttbr |= ((uint64_t)vmid << VTTBR_VMID_SHIFT) & VTTBR_VMID_MASK;

    vttbr &= ~(VTTBR_BADDR_MASK);
    vttbr |= (uint32_t) ttbl & VTTBR_BADDR_MASK;
    write_vttbr(vttbr);

    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    uart_print( "changed vttbr:" ); uart_print_hex64(vttbr); uart_print("\n\r");
#endif
    return HVMM_STATUS_SUCCESS;
}
