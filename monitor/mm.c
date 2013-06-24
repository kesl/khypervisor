
#include "hyp_config.h"
#include "mm.h"
#include "uart_print.h"
#include "armv7_p15.h"
#include "arch_types.h"

/* LPAE Memory region attributes, to match Linux's (non-LPAE) choices.
 * Indexed by the AttrIndex bits of a LPAE entry;
 * the 8-bit fields are packed little-endian into MAIR0 and MAIR1
 *
 *                 ai    encoding
 *   UNCACHED      000   0000 0000  -- Strongly Ordered
 *   BUFFERABLE    001   0100 0100  -- Non-Cacheable
 *   WRITETHROUGH  010   1010 1010  -- Write-through
 *   WRITEBACK     011   1110 1110  -- Write-back
 *   DEV_SHARED    100   0000 0100  -- Device
 *   ??            101
 *   reserved      110
 *   WRITEALLOC    111   1111 1111  -- Write-back write-allocate
 *      
 *   DEV_NONSHARED 100   (== DEV_SHARED)
 *   DEV_WC        001   (== BUFFERABLE)
 *   DEV_CACHED    011   (== WRITEBACK)
 */ 
#define INITIAL_MAIR0VAL 0xeeaa4400
#define INITIAL_MAIR1VAL 0xff000004
#define INITIAL_MAIRVAL (INITIAL_MAIR0VAL|INITIAL_MAIR1VAL<<32)

/*
 * Attribute Indexes.
 *
 * These are valid in the AttrIndx[2:0] field of an LPAE stage 1 page
 * table entry. They are indexes into the bytes of the MAIR*
 * registers, as defined above.
 *
 */
#define UNCACHED      0x0
#define BUFFERABLE    0x1
#define WRITETHROUGH  0x2
#define WRITEBACK     0x3
#define DEV_SHARED    0x4
#define WRITEALLOC    0x7
#define DEV_NONSHARED DEV_SHARED
#define DEV_WC        BUFFERABLE
#define DEV_CACHED    WRITEBACK

/* SCTLR System Control Register. */
/* HSCTLR is a subset of this. */
#define SCTLR_TE        (1<<30)
#define SCTLR_AFE       (1<<29)
#define SCTLR_TRE       (1<<28)
#define SCTLR_NMFI      (1<<27)
#define SCTLR_EE        (1<<25)
#define SCTLR_VE        (1<<24)
#define SCTLR_U         (1<<22)
#define SCTLR_FI        (1<<21)
#define SCTLR_WXN       (1<<19)
#define SCTLR_HA        (1<<17)
#define SCTLR_RR        (1<<14)
#define SCTLR_V         (1<<13)
#define SCTLR_I         (1<<12)
#define SCTLR_Z         (1<<11)
#define SCTLR_SW        (1<<10)
#define SCTLR_B         (1<<7)
#define SCTLR_C         (1<<2)
#define SCTLR_A         (1<<1)
#define SCTLR_M         (1<<0)
#define SCTLR_BASE        0x00c50078
#define HSCTLR_BASE       0x30c51878

/* HTTBR */
#define HTTBR_INITVAL                                   0x0000000000000000ULL
#define HTTBR_BADDR_MASK                                0x000000FFFFFFF000ULL
#define HTTBR_BADDR_SHIFT                               12

/* HTCR */
#define HTCR_INITVAL                                    0x80000000
#define HTCR_SH0_MASK                                   0x00003000
#define HTCR_SH0_SHIFT                                  12
#define HTCR_ORGN0_MASK                                 0x00000C00
#define HTCR_ORGN0_SHIFT                                10
#define HTCR_IRGN0_MASK                                 0x00000300
#define HTCR_IRGN0_SHIFT                                8
#define HTCR_T0SZ_MASK                                  0x00000003
#define HTCR_T0SZ_SHIFT                                 0

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

/* Long-descriptor translation table format */
#define TTBL_L1_OUTADDR_MASK	0x000000FFEC000000ULL
#define TTBL_L2_OUTADDR_MASK	0x000000FFFFE00000ULL

/* Stage 2 Level 2 */
#define VMM_L2_PAGETABLE_ENTRIES		512

/* PL2 Stage 1 Level 1 */
#define HMM_L1_PAGETABLE_ENTRIES		512

static lpaed_t _hmm_pgtable[HMM_L1_PAGETABLE_ENTRIES] __attribute((__aligned__(4096)));

/* Statically allocated for now */
static lpaed_t _vttbr_pte_guest0[VMM_L2_PAGETABLE_ENTRIES] __attribute((__aligned__(4096)));
static lpaed_t _vttbr_pte_guest1[VMM_L2_PAGETABLE_ENTRIES] __attribute((__aligned__(4096)));
static lpaed_t *_vmid_ttbl[NUM_GUESTS_STATIC];

/* Translation Table for the specified vmid */
lpaed_t *hvmm_mm_vmid_ttbl(vmid_t vmid)
{
	lpaed_t *ttbl = 0;
	if ( vmid < NUM_GUESTS_STATIC ) {
		ttbl = _vmid_ttbl[vmid];
	}
	return ttbl;
}

/* Enable/Disable Stage2 Translation */
void hvmm_mm_stage2_enable(int enable)
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

hvmm_status_t hvmm_mm_set_vmid_ttbl( vmid_t vmid, lpaed_t *ttbl )
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


/* Level 2 Block, 2MB, entry in LPAE Descriptor format for the given physical address */
lpaed_t hvmm_mm_lpaed_l2_block( uint64_t pa )
{
	lpaed_t lpaed;

	// Valid Block Entry
	lpaed.pt.valid = 1;
	lpaed.pt.table = 0;

	lpaed.bits &= ~TTBL_L2_OUTADDR_MASK;
	lpaed.bits |= pa & TTBL_L2_OUTADDR_MASK;
	lpaed.p2m.sbz3 = 0;

	// Lower block attributes
	lpaed.p2m.mattr = 0xA;	// 0b0101: normal memory, outer non-cacheable, inner non-cacheable
	lpaed.p2m.read = 1;		// Read/Write
	lpaed.p2m.write = 1;		
	lpaed.p2m.sh = 0;	// Non-shareable
	lpaed.p2m.af = 1;	// Access Flag set to 1?
	lpaed.p2m.sbz4 = 0;

	// Upper block attributes
	lpaed.p2m.hint = 0;
	lpaed.p2m.sbz2 = 0;
	lpaed.p2m.xn = 0;	// eXecute Never = 0

	lpaed.p2m.sbz1 = 0;

	return lpaed;
}

/* Level 1 Block, 1GB, entry in LPAE Descriptor format for the given physical address */
lpaed_t hvmm_mm_lpaed_l1_block( uint64_t pa, uint8_t attr_idx )
{
	lpaed_t lpaed;

	uart_print( "[mm] hvmm_mm_lpaed_l1_block:\n\r" );
	uart_print( " pa:"); uart_print_hex64(pa); uart_print("\n\r");
	uart_print( " attr_idx:"); uart_print_hex32((uint32_t) attr_idx); uart_print("\n\r");

	// Valid Block Entry
	lpaed.pt.valid = 1;
	lpaed.pt.table = 0;

	lpaed.bits &= ~TTBL_L1_OUTADDR_MASK;
	lpaed.bits |= pa & TTBL_L1_OUTADDR_MASK;
	lpaed.pt.sbz = 0;

	// Lower block attributes
	lpaed.pt.ai = attr_idx;
	lpaed.pt.ns = 1;	// Allow Non-secure access
	lpaed.pt.user = 1;
	lpaed.pt.ro = 0;
	lpaed.pt.sh = 2;	// Outher Shareable
	lpaed.pt.af = 1;	// Access Flag set to 1?
	lpaed.pt.ng = 1;

	// Upper block attributes
	lpaed.pt.hint = 0;
	lpaed.pt.pxn = 0;
	lpaed.pt.xn = 0;	// eXecute Never = 0
	return lpaed;
}


/* 
 * Initialization of Virtual Machine Memory Management 
 * Stage 2 Translation
 */
void _vmm_init(void)
{
	/*
	 * Initializes Translation Table for Stage2 Translation (IPA -> PA)
	 */
	int i;
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
		uint64_t pa1_end = 0xE0000000;
		uint64_t pa2 = (uint32_t) &guest2_bin_start;
		lpaed_t lpaed;

		uart_print( "pa:"); uart_print_hex64(pa1); uart_print("\n\r");
		uart_print( "pa_end:"); uart_print_hex64(pa1_end); uart_print("\n\r");
		uart_print( "_vmid_ttbl[0]:"); uart_print_hex32((uint32_t) _vmid_ttbl[0]); uart_print("\n\r");

		for(i = 0; pa1 < pa1_end; i++, pa1 += 0x200000, pa2 += 0x200000 ) {
			/* 2MB blocks per each entry */

			/* Guest 0 */
			lpaed = hvmm_mm_lpaed_l2_block(pa1);
			_vttbr_pte_guest0[i] = lpaed;
			//uart_print( "lpaed:"); uart_print_hex64(lpaed.bits); uart_print("\n\r");

			/* Guest 1 */
			lpaed = hvmm_mm_lpaed_l2_block(pa2);
			_vttbr_pte_guest1[i] = lpaed;
		}
	}
}

/* 
 * Initialization of Host Monitor Memory Management 
 * PL2 Stage1 Translation
 * VA32 -> PA
 */
void _hmm_init(void)
{
	int i;
	uint64_t pa = 0x00000000ULL;
	/*
	 * Partition 0: 0x00000000 ~ 0x3FFFFFFF - Peripheral - DEV_SHARED
	 * Partition 1: 0x40000000 ~ 0x7FFFFFFF - Unused     - UNCACHED
	 * Partition 2: 0x80000000 ~ 0xBFFFFFFF	- Guest	     - UNCACHED
	 * Partition 3: 0xC0000000 ~ 0xEFFFFFFF	- Monitor    - WRITEBACK
	 */
	_hmm_pgtable[0] = hvmm_mm_lpaed_l1_block(pa, UNCACHED); pa += 0x40000000;
	uart_print( "&_hmm_pgtable[0]:"); uart_print_hex32((uint32_t) &_hmm_pgtable[0]); uart_print("\n\r");
	uart_print( "lpaed:"); uart_print_hex64(_hmm_pgtable[0].bits); uart_print("\n\r");
	_hmm_pgtable[1] = hvmm_mm_lpaed_l1_block(pa, UNCACHED); pa += 0x40000000;
	uart_print( "&_hmm_pgtable[1]:"); uart_print_hex32((uint32_t) &_hmm_pgtable[1]); uart_print("\n\r");
	uart_print( "lpaed:"); uart_print_hex64(_hmm_pgtable[1].bits); uart_print("\n\r");
	_hmm_pgtable[2] = hvmm_mm_lpaed_l1_block(pa, UNCACHED); pa += 0x40000000;
	uart_print( "&_hmm_pgtable[2]:"); uart_print_hex32((uint32_t) &_hmm_pgtable[2]); uart_print("\n\r");
	uart_print( "lpaed:"); uart_print_hex64(_hmm_pgtable[2].bits); uart_print("\n\r");
	_hmm_pgtable[3] = hvmm_mm_lpaed_l1_block(pa, UNCACHED); pa += 0x40000000;
	uart_print( "&_hmm_pgtable[3]:"); uart_print_hex32((uint32_t) &_hmm_pgtable[3]); uart_print("\n\r");
	uart_print( "lpaed:"); uart_print_hex64(_hmm_pgtable[3].bits); uart_print("\n\r");
	for ( i = 4; i < HMM_L1_PAGETABLE_ENTRIES; i++ ) {
		_hmm_pgtable[i].pt.valid = 0;
	}
}

int hvmm_mm_init(void)
{
/*
 *	MAIR0, MAIR1
 *	HMAIR0, HMAIR1
 *	HTCR
 *	HTCTLR
 *	HTTBR
 * 	HTCTLR
 */
	uint32_t mair, htcr, hsctlr, vtcr, hcr;
	uint64_t httbr, vttbr;
	uart_print( "[mm] mm_init: enter\n\r" );
	
	_vmm_init();
	_hmm_init();

	// MAIR/HMAIR
	uart_print(" --- MAIR ----\n\r" );
	mair = read_mair0(); uart_print( "mair0:"); uart_print_hex32(mair); uart_print("\n\r");
	mair = read_mair1(); uart_print( "mair1:"); uart_print_hex32(mair); uart_print("\n\r");
	mair = read_hmair0(); uart_print( "hmair0:"); uart_print_hex32(mair); uart_print("\n\r");
	mair = read_hmair1(); uart_print( "hmair1:"); uart_print_hex32(mair); uart_print("\n\r");

	write_mair0( INITIAL_MAIR0VAL );
	write_mair1( INITIAL_MAIR1VAL );
	write_hmair0( INITIAL_MAIR0VAL );
	write_hmair1( INITIAL_MAIR1VAL );

	mair = read_mair0(); uart_print( "mair0:"); uart_print_hex32(mair); uart_print("\n\r");
	mair = read_mair1(); uart_print( "mair1:"); uart_print_hex32(mair); uart_print("\n\r");
	mair = read_hmair0(); uart_print( "hmair0:"); uart_print_hex32(mair); uart_print("\n\r");
	mair = read_hmair1(); uart_print( "hmair1:"); uart_print_hex32(mair); uart_print("\n\r");

	// HTCR
	uart_print(" --- HTCR ----\n\r" );
	htcr = read_htcr(); uart_print( "htcr:"); uart_print_hex32(htcr); uart_print("\n\r");
	write_htcr( 0x80002500 );
	htcr = read_htcr(); uart_print( "htcr:"); uart_print_hex32(htcr); uart_print("\n\r");

	// HSCTLR
	// i-Cache and Alignment Checking Enabled
	// MMU, D-cache, Write-implies-XN, Low-latency IRQs Disabled
	hsctlr = read_hsctlr(); uart_print( "hsctlr:"); uart_print_hex32(hsctlr); uart_print("\n\r");
	hsctlr = HSCTLR_BASE | SCTLR_A;
	write_hsctlr( hsctlr );
	hsctlr = read_hsctlr(); uart_print( "hsctlr:"); uart_print_hex32(hsctlr); uart_print("\n\r");

// VTCR
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

// HCR
	hcr = read_hcr(); uart_print( "hcr:"); uart_print_hex32(hcr); uart_print("\n\r");

	

// HTCR
	// Shareability - SH0[13:12] = 0 - Not shared
	// Outer Cacheability - ORGN0[11:10] = 11b - Write Back no Write Allocate Cacheable
	// Inner Cacheability - IRGN0[9:8] = 11b - Same
	// T0SZ[2:0] = 0 - 2^32 Input Address 
#if 0
	htcr = read_htcr(); uart_print( "htcr:"); uart_print_hex32(htcr); uart_print("\n\r");
	htcr &= ~HTCR_SH0_MASK;
	htcr |= (0x0 << HTCR_SH0_SHIFT) & HTCR_SH0_MASK;
	htcr &= ~HTCR_ORGN0_MASK;
	htcr |= (0x3 << HTCR_ORGN0_SHIFT) & HTCR_ORGN0_MASK;
	htcr &= ~VTCR_IRGN0_MASK;
	htcr |= (0x3 << HTCR_IRGN0_SHIFT) & HTCR_IRGN0_MASK;
	htcr &= ~VTCR_T0SZ_MASK;
	htcr |= (0x0 << HTCR_T0SZ_SHIFT) & HTCR_T0SZ_MASK;
	write_htcr( htcr );
	htcr = read_htcr(); uart_print( "htcr:"); uart_print_hex32(htcr); uart_print("\n\r");
#endif

// HTTBR = &__hmm_pgtable
	httbr = read_httbr(); uart_print( "httbr:" ); uart_print_hex64(httbr); uart_print("\n\r");
	httbr &= 0xFFFFFFFF00000000ULL;
	httbr |= (uint32_t) &_hmm_pgtable;
	httbr &= HTTBR_BADDR_MASK;
	uart_print( "writing httbr:" ); uart_print_hex64(httbr); uart_print("\n\r");
	write_httbr( httbr );
	httbr = read_httbr(); uart_print( "read back httbr:" ); uart_print_hex64(httbr); uart_print("\n\r");

// TODO: Write PTE to __hmm_pgtable
#if 1
	/* Enable PL2 Stage 1 MMU */
	// HSCTLR Enable MMU and D-cache
	hsctlr = read_hsctlr(); uart_print( "hsctlr:"); uart_print_hex32(hsctlr); uart_print("\n\r");
	// hsctlr |= (SCTLR_M |SCTLR_C);
	hsctlr |= (SCTLR_M);
	
	// Flush PTE writes
	asm("dsb");
	write_hsctlr( hsctlr );
	// Flush iCache
	asm("isb");
	hsctlr = read_hsctlr(); uart_print( "hsctlr:"); uart_print_hex32(hsctlr); uart_print("\n\r");
#endif

	uart_print( "[mm] mm_init: exit\n\r" );
	
}
