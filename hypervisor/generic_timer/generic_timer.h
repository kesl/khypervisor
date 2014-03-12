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

/**
* @brief Register generic timer irqs such as HYP, NSP, VIR
*
* DEVICE : IRQ number
* HYP_TIMER : 26
* NSP_TIMER : 27
* VIR_TIMER : 30
*
* @return Returns success only.
* Use MACRO instead of SUCCESS.
*/
hvmm_status_t generic_timer_init();
/* Enable the timer interrupt. Specified by timer type */
hvmm_status_t generic_timer_enable_int();
/* Disable the timer. Specified by timer type */
hvmm_status_t generic_timer_disable_int();
/*
 * Sets time interval. Converts from microseconds
 * to count and sets time interval.
 */
hvmm_status_t generic_timer_set_tval(uint32_t tval);
/* Enables timer irq.  */
hvmm_status_t generic_timer_enable_irq();
/* Adds callback funtion. Called when occur timer interrupt */
hvmm_status_t generic_timer_set_callback(timer_callback_t callback, void *);

#define GENERIC_TIMER_CTRL_ENABLE       (1 << 0)
#define GENERIC_TIMER_CTRL_IMASK        (1 << 1)
#define GENERIC_TIMER_CTRL_ISTATUS      (1 << 2)
#define generic_timer_pcounter_read()   read_cntpct()
#define generic_timer_vcounter_read()   read_cntvct()

#endif

