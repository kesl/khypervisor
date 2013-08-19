#include "gic.h"
#include "gic_regs.h"
#include "uart_print.h"
#include <hvmm_trace.h>
#include <a15_cp15_sysregs.h>
#include <armv7_p15.h>

#define CBAR_PERIPHBASE_MSB_MASK    0x000000FF

#define ARM_CPUID_CORTEXA15   0x412fc0f1

#define MIDR_MASK_PPN        (0x0FFF <<4)
#define MIDR_PPN_CORTEXA15    (0xC0F << 4)


#define GIC_INT_PRIORITY_DEFAULT_WORD    ( (GIC_INT_PRIORITY_DEFAULT << 24 ) \
                                         |(GIC_INT_PRIORITY_DEFAULT << 16 ) \
                                         |(GIC_INT_PRIORITY_DEFAULT << 8 ) \
                                         |(GIC_INT_PRIORITY_DEFAULT ) )
#define GIC_NUM_MAX_IRQS    1024

#define GIC_SIGNATURE_INITIALIZED   0x5108EAD7

/*
 * Note:
 * The proper implementation of GIC in a guest is to access GIC Distributor 
 * and GIC CPU Interface as if it were accessing the physical GIC registers
 * and the Hypervisor traps access to GIC Distributor from guests and remap 
 * GIC CPU Interface register address to GIC Virtual CPU Interface.
 *
 * However, in the current implementation below, we simply access
 * GIC Virtual CPU Interface directly
 * - simon
 */

/* Determined by Hypervisor's Stage2 Address Translation Table */
#define GIC_BASEADDR_GUEST                (0x3FE00000)

struct gic {
    uint32_t baseaddr;
    volatile uint32_t *ba_gicv;
    uint32_t lines;
    uint32_t cpus;
    gic_irq_handler_t handlers[GIC_NUM_MAX_IRQS];
    uint32_t initialized;
};

static struct gic _gic;

static void gic_dump_registers(void)
{

    uint32_t midr;

    HVMM_TRACE_ENTER();

    midr = read_midr();
    uart_print( "midr:"); uart_print_hex32(midr); uart_print("\n\r");

    if ( (midr & MIDR_MASK_PPN) == MIDR_PPN_CORTEXA15) {
        uint32_t value;
        uart_print( "gic baseaddr:"); uart_print_hex32(_gic.baseaddr); uart_print("\n\r");
        uart_print( "ba_gicv:"); uart_print_hex32((uint32_t)_gic.ba_gicv); uart_print("\n\r");
        uart_print( "GICV_CTLR:"); uart_print_hex32(_gic.ba_gicv[GICC_CTLR]); uart_print("\n\r");
        uart_print( " GICV_PMR:"); uart_print_hex32(_gic.ba_gicv[GICC_PMR]); uart_print("\n\r");
        uart_print( " GICV_BPR:"); uart_print_hex32(_gic.ba_gicv[GICC_BPR]); uart_print("\n\r");
        uart_print( " GICV_RPR:"); uart_print_hex32(_gic.ba_gicv[(0x0014/4)]); uart_print("\n\r");
        uart_print( "GICV_HPPIR:"); uart_print_hex32(_gic.ba_gicv[(0x0018/4)]); uart_print("\n\r");
        uart_print( "GICV_IIDR:"); uart_print_hex32(_gic.ba_gicv[(0x00FC/4)]); uart_print("\n\r");
    }
    HVMM_TRACE_EXIT();
}

static hvmm_status_t gic_init_baseaddr(uint32_t *va_base)
{
    uint32_t midr;
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    HVMM_TRACE_ENTER();

    midr = read_midr();
    uart_print( "midr:"); uart_print_hex32(midr); uart_print("\n\r");

    /* 
     * Note:
     * We currently support GICv2 with Cortex-A15 only. 
     * Other architectures with GICv2 support will be further listed and added for support later
     */
    if ( (midr & MIDR_MASK_PPN) == MIDR_PPN_CORTEXA15) {
        _gic.baseaddr = (uint32_t) va_base;
        _gic.ba_gicv = (uint32_t *) (_gic.baseaddr + GIC_OFFSET_GICVI);

        result = HVMM_STATUS_SUCCESS;
    } else {
        uart_print( "GICv2 Unsupported\n\r" );
        uart_print( "midr.ppn:"); uart_print_hex32(midr & MIDR_MASK_PPN); uart_print("\n\r");

        result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    }

    HVMM_TRACE_EXIT();
    return result;
}


/* API functions */
hvmm_status_t gic_enable_irq(uint32_t irq)
{
    /* TODO: Control GIC Distributor and let the Hypervisor trap and know */
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_disable_irq(uint32_t irq)
{
    /* TODO: Control GIC Distributor and let the Hypervisor trap and know */
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_set_irq_handler(int irq, gic_irq_handler_t handler, void *pdata )
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    if ( irq < GIC_NUM_MAX_IRQS ) {
        _gic.handlers[irq] = handler;
        result = HVMM_STATUS_SUCCESS;
    }
    return result;
}

hvmm_status_t gic_init(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    int i;

    HVMM_TRACE_ENTER();

    for( i = 0; i < GIC_NUM_MAX_IRQS; i++) {
        _gic.handlers[i] = 0;
    }

    result = gic_init_baseaddr((uint32_t *) GIC_BASEADDR_GUEST);

    /* enable group0 and group1 interrupts */
    _gic.ba_gicv[GICV_CTLR] |= 0x213;

    /* no priority masking */
    _gic.ba_gicv[GICV_PMR] = 0xFF;

    if ( result == HVMM_STATUS_SUCCESS ) {
        gic_dump_registers();
    }
    _gic.lines = 1022;

    result = HVMM_STATUS_SUCCESS;

    if ( result == HVMM_STATUS_SUCCESS ) {
        _gic.initialized = GIC_SIGNATURE_INITIALIZED;
    }
    HVMM_TRACE_EXIT();
    return result;
}

void gic_interrupt(int fiq, void *pregs)
{
    /*
     * 1. ACK - CPU Interface - GICC_IAR read
     * 2. Completion - CPU Interface - GICC_EOIR
     * 2.1 Deactivation - CPU Interface - GICC_DIR
     */
    uint32_t iar;
    uint32_t irq;
    struct arch_regs *regs = pregs;

    HVMM_TRACE_ENTER();

    do {
        /* ACK */
        iar = _gic.ba_gicv[GICV_IAR];
        irq = iar & GICC_IAR_INTID_MASK;
        if ( irq < _gic.lines ) {
            uart_print( "irq:"); uart_print_hex32(irq); uart_print("\n\r");

            /* ISR */
            if ( _gic.handlers[irq] ) {
                _gic.handlers[irq]( irq, regs, 0 );
            }

            /* Completion & Deactivation */
            _gic.ba_gicv[GICC_EOIR] = irq;
            _gic.ba_gicv[GICC_DIR] = irq;
        } else {
            uart_print( "end of irq(no pending):"); uart_print_hex32(irq); uart_print("\n\r");
            break;
        }
    } while(1);
    HVMM_TRACE_EXIT();
}
