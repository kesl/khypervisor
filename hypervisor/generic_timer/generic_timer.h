#ifndef __GENERIC_TIMER_H__
#define __GENERIC_TIMER_H__

#include "armv7_p15.h"
#include "gic.h"
#include <timer.h>
#include <log/uart_print.h>
#include <asm-arm_inline.h>

enum generic_timer_type {
    GENERIC_TIMER_HYP,      /* IRQ 26 */
    GENERIC_TIMER_VIR,      /* IRQ 27 */
    GENERIC_TIMER_NSP,      /* IRQ 30 */
    GENERIC_TIMER_NUM_TYPES
};

enum {
    GENERIC_TIMER_REG_FREQ,
    GENERIC_TIMER_REG_HCTL,
    GENERIC_TIMER_REG_KCTL,
    GENERIC_TIMER_REG_HYP_CTRL,
    GENERIC_TIMER_REG_HYP_TVAL,
    GENERIC_TIMER_REG_HYP_CVAL,
    GENERIC_TIMER_REG_PHYS_CTRL,
    GENERIC_TIMER_REG_PHYS_TVAL,
    GENERIC_TIMER_REG_PHYS_CVAL,
    GENERIC_TIMER_REG_VIRT_CTRL,
    GENERIC_TIMER_REG_VIRT_TVAL,
    GENERIC_TIMER_REG_VIRT_CVAL,
    GENERIC_TIMER_REG_VIRT_OFF,
};

/** @brief Registers generic timer irqs such as hypervisor timer event
 *  (GENERIC_TIMER_HYP), non-secure physical timer event(GENERIC_TIMER_NSP),
 *  and virtual timer event(GENERIC_TIMER_NSP).*
 *  Those interrupts are actice-LOW level-sensitive.
 *  @return Returns HVMM_STAUS_SUCESS only.
 *  @todo Have to implement VIR, NSP.
 */
hvmm_status_t generic_timer_init();

/** @brief Enables the timer interrupt such as hypervisor timer event
 *  by PL2 physical timer control register.
 *  @return Returns HVMM_STATUS_SUCCESS only.
 *  @todo Have to implement VIR, NSP.
 */
hvmm_status_t generic_timer_enable_int();

/** @brief Disable the timer interrupt such as hypervisor timer event
 *  by PL2 physical timer control register.
 *  @return Returns HVMM_STATUS_SUCCESS only.
 *  @todo Have to implement VIR, NSP.
 */
hvmm_status_t generic_timer_disable_int();

/** @brief Configures time interval by PL2 physical timerValue register.
 *  Convets measurement from microseconds to count.
 *  @param tval The timer value
 *  @return Returns HVMM_STATUS_SUCCESS only.
 *  @todo Have to implement VIR, NSP.
 */
hvmm_status_t generic_timer_set_tval(uint32_t tval);

/** @brief Enables irq.
 *  @return Returns HVMM_STATUS_SUCESS only
 *  @todo Have to implement VIR, NSP.
 */
hvmm_status_t generic_timer_enable_irq();

/** @brief Adds callback function. Called when occur timer interrupt.
 *  @param callback It is timer.
 *  @param void* The user
 *  @return Returns HVMM_STATUS_SUCCESS only.
 *  @todo Apply parameter "void*" to function because it's unused parameter.
 */
hvmm_status_t generic_timer_set_callback(timer_callback_t callback, void *);

#define GENERIC_TIMER_CTRL_ENABLE       (1 << 0)
#define GENERIC_TIMER_CTRL_IMASK        (1 << 1)
#define GENERIC_TIMER_CTRL_ISTATUS      (1 << 2)
#define generic_timer_pcounter_read()   read_cntpct()
#define generic_timer_vcounter_read()   read_cntvct()

#endif

