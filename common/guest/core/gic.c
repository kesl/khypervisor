#include "gic.h"
#include "gic_regs.h"
#include <log/uart_print.h>
#include <hvmm_trace.h>
#include <armv7_p15.h>
/* #ifdef _SMP_ */
#include <smp.h>
/* #endif */

#define CBAR_PERIPHBASE_MSB_MASK    0x000000FF

#define ARM_CPUID_CORTEXA15   0x412fc0f1

#define MIDR_MASK_PPN        (0x0FFF << 4)
#define MIDR_PPN_CORTEXA15    (0xC0F << 4)


#define GIC_INT_PRIORITY_DEFAULT_WORD    ((GIC_INT_PRIORITY_DEFAULT << 24) \
                                         |(GIC_INT_PRIORITY_DEFAULT << 16) \
                                         |(GIC_INT_PRIORITY_DEFAULT << 8) \
                                         |(GIC_INT_PRIORITY_DEFAULT))
#define GIC_NUM_MAX_IRQS    102

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
#ifdef ARNDALE
#define GIC_BASEADDR_GUEST                (0x10480000)
#else
#define GIC_BASEADDR_GUEST                (0x2C000000)
#endif

struct gic {
    uint32_t baseaddr;
    volatile uint32_t *ba_gicd;
    volatile uint32_t *ba_gicc;
    uint32_t lines;
    uint32_t cpus;
    gic_irq_handler_t handlers[GIC_NUM_MAX_IRQS];
    uint32_t initialized;
};

static struct gic _gic;

static void gic_test_vdev_access(void)
{
    volatile uint8_t *s_reg8;
    HVMM_TRACE_ENTER();
    _gic.ba_gicd[GICD_ISENABLER + 33 / 32] = (1u << (33 % 32));
    _gic.ba_gicd[GICD_ICENABLER + 33 / 32] = (1u << (33 % 32));
    _gic.ba_gicd[GICD_ISENABLER + 1 / 32] = (1u << (1 % 32));
    _gic.ba_gicd[GICD_ICENABLER + 1 / 32] = (1u << (1 % 32));
    _gic.ba_gicd[GICD_ISENABLER + 31 / 32] = (1u << (31 % 32));
    _gic.ba_gicd[GICD_ICENABLER + 31 / 32] = (1u << (31 % 32));
    /* 8/16 bit access test */
    s_reg8 = (uint8_t *) &(_gic.ba_gicd[GICD_ISENABLER]);
    s_reg8[2] = 0xf;
    uart_print_hex32(_gic.ba_gicd[GICD_ISENABLER]);
    uart_print("\n\r");
    s_reg8 = (uint8_t *) &(_gic.ba_gicd[GICD_ICENABLER]);
    s_reg8[2] = 0xf;
    uart_print_hex32(_gic.ba_gicd[GICD_ISENABLER]);
    uart_print("\n\r");
    HVMM_TRACE_EXIT();
}

static void gic_dump_registers(void)
{
    uint32_t midr;
    HVMM_TRACE_ENTER();
    midr = read_midr();
    uart_print("midr:");
    uart_print_hex32(midr);
    uart_print("\n\r");
    if ((midr & MIDR_MASK_PPN) == MIDR_PPN_CORTEXA15) {
        uint32_t value;
        uart_print("gic baseaddr:");
        uart_print_hex32(_gic.baseaddr);
        uart_print("\n\r");
        uart_print("ba_gicd:");
        uart_print_hex32((uint32_t)_gic.ba_gicd);
        uart_print("\n\r");
        uart_print("GICD_TYPER:");
        uart_print_hex32(_gic.ba_gicd[GICD_TYPER]);
        uart_print("\n\r");
        uart_print("ba_gicc:");
        uart_print_hex32((uint32_t)_gic.ba_gicc);
        uart_print("\n\r");
        uart_print("GICC_CTLR:");
        uart_print_hex32(_gic.ba_gicc[GICC_CTLR]);
        uart_print("\n\r");
        uart_print(" GICC_PMR:");
        uart_print_hex32(_gic.ba_gicc[GICC_PMR]);
        uart_print("\n\r");
        uart_print(" GICC_BPR:");
        uart_print_hex32(_gic.ba_gicc[GICC_BPR]);
        uart_print("\n\r");
        uart_print(" GICC_RPR:");
        uart_print_hex32(_gic.ba_gicc[(0x0014 / 4)]);
        uart_print("\n\r");
        uart_print("GICC_HPPIR:");
        uart_print_hex32(_gic.ba_gicc[(0x0018 / 4)]);
        uart_print("\n\r");
        uart_print("GICC_IIDR:");
        uart_print_hex32(_gic.ba_gicc[(0x00FC / 4)]);
        uart_print("\n\r");
        /* Test to see if VGICD on monitor side works as we expect */
        gic_test_vdev_access();
    }
    HVMM_TRACE_EXIT();
}

static hvmm_status_t gic_init_baseaddr(uint32_t *va_base)
{
    uint32_t midr;
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    HVMM_TRACE_ENTER();
    midr = read_midr();
    uart_print("midr:");
    uart_print_hex32(midr);
    uart_print("\n\r");
    /*
     * Note:
     * We currently support GICv2 with Cortex-A15 only.
     * Other architectures with GICv2 support will be further
     * listed and added for support later.
     */
    if ((midr & MIDR_MASK_PPN) == MIDR_PPN_CORTEXA15) {
        _gic.baseaddr = (uint32_t) va_base;
        _gic.ba_gicd = (uint32_t *)(_gic.baseaddr + GIC_OFFSET_GICD);
        _gic.ba_gicc = (uint32_t *)(_gic.baseaddr + GIC_OFFSET_GICC);
        result = HVMM_STATUS_SUCCESS;
    } else {
        uart_print("GICv2 Unsupported\n\r");
        uart_print("midr.ppn:");
        uart_print_hex32(midr & MIDR_MASK_PPN);
        uart_print("\n\r");
        result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    }
    HVMM_TRACE_EXIT();
    return result;
}


/* API functions */
hvmm_status_t gic_enable_irq(uint32_t irq)
{
    _gic.ba_gicd[GICD_ISENABLER + irq / 32] = (1u << (irq % 32));
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_disable_irq(uint32_t irq)
{
    _gic.ba_gicd[GICD_ICENABLER + irq / 32] = (1u << (irq % 32));
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_set_irq_handler(int irq, gic_irq_handler_t handler,
                void *pdata)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    if (irq < GIC_NUM_MAX_IRQS) {
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
    for (i = 0; i < GIC_NUM_MAX_IRQS; i++)
        _gic.handlers[i] = 0;

    result = gic_init_baseaddr((uint32_t *) GIC_BASEADDR_GUEST);
    /* enable group0 and group1 interrupts */
    _gic.ba_gicc[GICC_CTLR] |= 0x213;
    /* no priority masking */
    _gic.ba_gicc[GICC_PMR] = 0xFF;
    if (result == HVMM_STATUS_SUCCESS)
        gic_dump_registers();

    _gic.lines = 1022;
    result = HVMM_STATUS_SUCCESS;
    if (result == HVMM_STATUS_SUCCESS)
        _gic.initialized = GIC_SIGNATURE_INITIALIZED;

    for (i = 0; i < 4; i++)
        _gic.ba_gicd[GICD_ICENABLER + i] = 0xffffffff;
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
/* #if _SMP_ */
    uint32_t cpu = smp_processor_id();
/* #endif */
   /* ACK */
    iar = _gic.ba_gicc[GICC_IAR];
    irq = iar & GICC_IAR_INTID_MASK;
    if (irq < _gic.lines) {
        if (irq == 0) {
            uart_print("ba_gicd:");
            uart_print_hex32((uint32_t) _gic.ba_gicd);
            uart_print("\n\r");
            uart_print("ba_gicc:");
            uart_print_hex32((uint32_t) _gic.ba_gicc);
            uart_print("\n\r");
        }
        /* ISR */
        if (_gic.handlers[irq])
            _gic.handlers[irq](irq, regs, 0);

        /* Completion & Deactivation */
        _gic.ba_gicc[GICC_EOIR] = irq;
        _gic.ba_gicc[GICC_DIR] = irq;
    } else {
    /*TODO  Need to know why this part occurred*/
    #if 0
       uart_print("end of irq(no pending):");
       uart_print_hex32(irq);
       uart_print("\n\r");
    #endif
    }
}
