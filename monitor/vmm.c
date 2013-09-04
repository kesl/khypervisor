/* Virtual Machine Memory Management Module */

#include "hyp_config.h"
#include <arch_types.h>
#include "vmm.h"
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include "print.h"

/* Stage 2 Level 2 */
#define VMM_L2_PAGETABLE_ENTRIES		512

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

/*
 * Stage 2 Translation Table, look up begins at second level
 * VTTBR.BADDR[31:x]: x=14, VTCR.T0SZ = 0, 2^32 input address range, VTCR.SL0 = 0(2nd), 16KB aligned base address
 * Statically allocated for now
 */
static lpaed_t _vttbr_pte_guest0[VMM_L2_PAGETABLE_ENTRIES] __attribute((__aligned__(16384)));
static lpaed_t _vttbr_pte_guest1[VMM_L2_PAGETABLE_ENTRIES] __attribute((__aligned__(16384)));
static lpaed_t *_vmid_ttbl[NUM_GUESTS_STATIC];

static void vmm_init_mmu(void)
{
    uint32_t vtcr, vttbr;

    HVMM_TRACE_ENTER();

	vtcr = read_vtcr(); uart_print( "vtcr:"); uart_print_hex32(vtcr); uart_print("\n\r");

	// start lookup at level 2 table
	vtcr &= ~VTCR_SL0_MASK;
	vtcr |= (0x0 << VTCR_SL0_SHIFT) & VTCR_SL0_MASK;
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

	_vmid_ttbl[0] = &_vttbr_pte_guest0[0];
	_vmid_ttbl[1] = &_vttbr_pte_guest1[1];

	/*
	 * VA: 0x00000000 ~ 0x3FFFFFFF, 1GB
	 * PA: 0xA0000000 ~ 0xDFFFFFFF	guest_bin_start
	 * PA: 0xB0000000 ~ 0xEFFFFFFF	guest2_bin_start
	 */
	{
		extern uint32_t guest_bin_start;
		extern uint32_t guest2_bin_start;

		uint64_t pa1 = (uint32_t) &guest_bin_start;
		uint64_t pa1_end = pa1 + 0x40000000;
		uint64_t pa2 = (uint32_t) &guest2_bin_start;
		lpaed_t lpaed;

		uart_print( "pa:"); uart_print_hex64(pa1); uart_print("\n\r");
		uart_print( "pa_end:"); uart_print_hex64(pa1_end); uart_print("\n\r");
		uart_print( "_vmid_ttbl[0]:"); uart_print_hex32((uint32_t) _vmid_ttbl[0]); uart_print("\n\r");

		/* 2MB blocks per each entry */
		for(i = 0; pa1 < pa1_end; i++, pa1 += 0x200000, pa2 += 0x200000 ) {
		    uart_print( "pa_end-pa1:"); uart_print_hex64(pa1_end-pa1); uart_print("\n\r");
            if ( (pa1_end - pa1) == 0x200000 ) {
                /* GIC_BASEADDR_GUEST: 0x3FE00000 */
                /* Enable access from guest to GIC Virtual CPU Interface */
			    lpaed = hvmm_mm_lpaed_l2_block(0x2C000000, LPAED_STAGE2_MEMATTR_DM);
			    _vttbr_pte_guest0[i] = lpaed;

			    lpaed = hvmm_mm_lpaed_l2_block(0x2C000000, LPAED_STAGE2_MEMATTR_DM);
			    _vttbr_pte_guest1[i] = lpaed;
            } else if ( (pa1_end - pa1) == 0x400000 ) {
                /* UART: 0x3FCA0000 */
			    lpaed = hvmm_mm_lpaed_l2_block(0x1C000000, LPAED_STAGE2_MEMATTR_DM);
			    _vttbr_pte_guest0[i] = lpaed;
			    lpaed = hvmm_mm_lpaed_l2_block(0x1C000000, LPAED_STAGE2_MEMATTR_DM);
			    _vttbr_pte_guest1[i] = lpaed;
            } else {
			    /* Guest 0 */
			    lpaed = hvmm_mm_lpaed_l2_block(pa1, LPAED_STAGE2_MEMATTR_NORMAL_OWT | LPAED_STAGE2_MEMATTR_NORMAL_IWT);
			    _vttbr_pte_guest0[i] = lpaed;

			    /* Guest 1 */
			    lpaed = hvmm_mm_lpaed_l2_block(pa2, LPAED_STAGE2_MEMATTR_NORMAL_OWT | LPAED_STAGE2_MEMATTR_NORMAL_IWT);
			    _vttbr_pte_guest1[i] = lpaed;
            }
		}
	}

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
	vttbr = read_vttbr(); uart_print( "current vttbr:" ); uart_print_hex64(vttbr); uart_print("\n\r");
	vttbr &= ~(VTTBR_VMID_MASK);
	vttbr |= ((uint64_t)vmid << VTTBR_VMID_SHIFT) & VTTBR_VMID_MASK;

	vttbr &= ~(VTTBR_BADDR_MASK);
	vttbr |= (uint32_t) ttbl & VTTBR_BADDR_MASK;
	write_vttbr(vttbr);

	vttbr = read_vttbr(); uart_print( "changed vttbr:" ); uart_print_hex64(vttbr); uart_print("\n\r");
	return HVMM_STATUS_SUCCESS;
}
