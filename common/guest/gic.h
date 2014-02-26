#ifndef __GIC_H__
#define __GIC_H__

#include "hvmm_types.h"
#include "arch_types.h"

#define GIC_INT_PRIORITY_DEFAULT        0xa0

enum gic_int_polarity {
    GIC_INT_POLARITY_LEVEL = 0,
    GIC_INT_POLARITY_EDGE = 1
};

typedef void (*gic_irq_handler_t)(int irq, void *regs, void *pdata);

void gic_interrupt(int fiq, void *regs);
hvmm_status_t gic_enable_irq(uint32_t irq);
hvmm_status_t gic_disable_irq(uint32_t irq);
hvmm_status_t gic_init(void);
volatile uint32_t *gic_vgic_baseaddr(void);

hvmm_status_t gic_set_irq_handler(int irq, gic_irq_handler_t handler,
                void *pdata);

#endif
