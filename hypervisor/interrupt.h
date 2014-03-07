#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include "hvmm_types.h"
/**
 * @brief   Initializes GIC, VGIC status.
 * <pre>
 * Initialization sequence.
 * 1. Routes IRQ/IFQ to Hyp Exception Vector.
 * 2. Initializes GIC Distributor & CPU Interface and enable them.
 * 3. Initializes and enable Virtual Interface Control.
 * 4. Initializes physical irq, virtual irq status table, and
 *    registers injection callback function that is called 
 *    when flushs virtual irq.
 * </pre>
 * @return  If all initialization is success, then returns "success"
 *          otherwise returns "unknown error"
 */
hvmm_status_t hvmm_interrupt_init(void);
#endif
