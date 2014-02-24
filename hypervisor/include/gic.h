#ifndef __GIC_H__
#define __GIC_H__

#include "hvmm_types.h"
#include "arch_types.h"
#include "smp.h"

#define GIC_NUM_MAX_IRQS    1024
#define gic_cpumask_current()    (1u << smp_processor_id())
#define GIC_INT_PRIORITY_DEFAULT        0xa0

typedef enum {
    GIC_INT_POLARITY_LEVEL = 0,
    GIC_INT_POLARITY_EDGE = 1
} gic_int_polarity_t;

typedef void (*gic_irq_handler_t)(int irq, void *regs, void *pdata);

void gic_interrupt(int fiq, void *regs);
hvmm_status_t gic_enable_irq(uint32_t irq);
hvmm_status_t gic_disable_irq(uint32_t irq);
hvmm_status_t gic_init(void);
hvmm_status_t gic_deactivate_irq(uint32_t irq);
volatile uint32_t *gic_vgic_baseaddr(void);

/*
 * example:
    gic_test_configure_irq( 26,
                GIC_INT_POLARITY_LEVEL,
                (1u << smp_processor_id()),
                GIC_INT_PRIORITY_DEFAULT );
    gic_test_set_irq_handler( 26, &myhandler );
 */
hvmm_status_t gic_test_configure_irq(uint32_t irq, gic_int_polarity_t polarity,  uint8_t cpumask, uint8_t priority);
hvmm_status_t gic_test_set_irq_handler(int irq, gic_irq_handler_t handler, void *pdata);

#endif
