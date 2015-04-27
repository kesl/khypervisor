#ifndef __GIC_H__
#define __GIC_H__

#include <hvmm_types.h>
#include <arch_types.h>
#include <smp.h>
#include <interrupt.h>

#define GIC_NUM_MAX_IRQS    1024
#define gic_cpumask_current()    (1u << smp_processor_id())
#define GIC_INT_PRIORITY_DEFAULT        0xa0

enum gic_int_polarity {
    GIC_INT_POLARITY_LEVEL = 0,
    GIC_INT_POLARITY_EDGE = 1
};

void gic_interrupt(int fiq, void *regs);
hvmm_status_t gic_enable_irq(uint32_t irq);
hvmm_status_t gic_disable_irq(uint32_t irq);
/**
 * @brief   Initializes GIC.
 * <pre>
 * Initialization sequence
 * 1. Determine GIC's base address and set it.
 * 2. Initialize and Enable GIC Distributor.
 * 3. Initialize and Enable GIC CPU Interface for this CPU.
 * </pre>
 * @return  If initialization success then return success,
 * otherwise return "unknown error".
 */
hvmm_status_t gic_init(void);
hvmm_status_t gic_deactivate_irq(uint32_t irq);
hvmm_status_t gic_completion_irq(uint32_t irq);
/**
 * @brief Returns Virtual interface control register(GICH)'s base address.
 * @return Base address of GICH
 */
volatile uint32_t *gic_vgic_baseaddr(void);

/**
 * @brief           Configures for a given IRQ.
 * @param irq       Interrupt number.
 * @param polarity  level-sensitive or edge-triggered.
 * @param cpumask   Targets processor mask for the interrupt.
 * @param priority  Priority level for each interrupt.
 * @return  If interrupt number is vaild then return success,
 *          otherwise return "unsupported feature".
 */
hvmm_status_t gic_configure_irq(uint32_t irq,
                enum gic_int_polarity polarity, uint8_t cpumask,
                uint8_t priority);

uint32_t gic_get_irq_number(void);

#endif
