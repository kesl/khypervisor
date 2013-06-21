#include "gic.h"
#include "a15_cp15_sysregs.h"

#define ARM_CPUID_CORTEXA15   0x412fc0f1

#define MIDR_MASK_PPN		(0x0FFF <<4)
#define MIDR_PPN_CORTEXA15	(0xC0F << 4)

#define GIC_OFFSET_GICD		0x1000
#define GIC_OFFSET_GICC		0x2000
#define GIC_OFFSET_GICH		0x4000
#define GIC_OFFSET_GICV		0x5000
#define GIC_OFFSET_GICVI	0x6000

#define GICD_CTLR	0x000
#define GICD_TYPER	0x004
#define GICD_IIDR	0x008

void gic_dump_registers(void)
{
// MIDR[15:4], CRn:c0, Op1:0, CRm:c0, Op2:0  == 0xC0F (Cortex-A15)
// Cortex-A15 C15 System Control, C15 Registers
// Name: Op1, CRm, Op2
// --------------------
// CBAR:   4,  c0,   0
// MRC p15, 4, <Rt>, c15, c0, 0; Read Configuration Base Address Register

/*
	if ( CPUID == CORTEX_A15) {
		baseaddr = CBAR.PERIPHBASE[31:15];
		baseaddr_gicd = baseaddr + GIC_OFFSET_GICD;	// 0x1000
		baseaddr_gicc = baseaddr + GIC_OFFSET_GICC;	// 0x2000
		baseaddr_gich = baseaddr + GIC_OFFSET_GICH;	// 0x4000
		baseaddr_gicv = baseaddr + GIC_OFFSET_GICV;	// 0x5000
		baseaddr_gicvv= baseaddr + GIC_OFFSET_GICVV;	// 0x6000
	}
 */
	uint32_t midr;
	uint32_t baseaddr;
	uint32_t *ba_gicd;
	uint32_t *ba_gicc;
	uint32_t *ba_gich;
	uint32_t *ba_gicv;
	uint32_t *ba_gicvi;

	midr = read_midr();
	uart_print( "midr:"); uart_print_hex32(midr); uart_print("\n\r");

	if ( (midr & MIDR_MASK_PPN) == MIDR_PPN_CORTEXA15) {
		uint32_t value;
		baseaddr = read_cbar();
		uart_print( "cbar:"); uart_print_hex32(baseaddr); uart_print("\n\r");
		ba_gicd = (uint32_t *) (baseaddr + GIC_OFFSET_GICD);
		ba_gicc = (uint32_t *) (baseaddr + GIC_OFFSET_GICC);
		ba_gich = (uint32_t *) (baseaddr + GIC_OFFSET_GICH);
		ba_gicv = (uint32_t *) (baseaddr + GIC_OFFSET_GICV);
		ba_gicvi = (uint32_t *) (baseaddr + GIC_OFFSET_GICVI);
		uart_print( "ba_gicd:"); uart_print_hex32(ba_gicd); uart_print("\n\r");
		uart_print( "ba_gicc:"); uart_print_hex32(ba_gicc); uart_print("\n\r");
		uart_print( "ba_gich:"); uart_print_hex32(ba_gich); uart_print("\n\r");
		uart_print( "ba_gicv:"); uart_print_hex32(ba_gicv); uart_print("\n\r");
		uart_print( "ba_gicvi:"); uart_print_hex32(ba_gicvi); uart_print("\n\r");
		value = ba_gicd[GICD_CTLR]; uart_print( "GICD_CTLR:"); uart_print_hex32(value); uart_print("\n\r");
		value = ba_gicd[GICD_TYPER]; uart_print( "GICD_TYPER:"); uart_print_hex32(value); uart_print("\n\r");
		value = ba_gicd[GICD_IIDR]; uart_print( "GICD_IIDR:"); uart_print_hex32(value); uart_print("\n\r");
	}
}

hvmm_status_t gic_init(void)
{
	gic_dump_registers();
	return HVMM_STATUS_SUCCESS;
}
