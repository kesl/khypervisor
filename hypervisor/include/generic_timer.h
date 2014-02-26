#ifndef __GENERIC_TIMER_H__
#define __GENERIC_TIMER_H__

#include "armv7_p15.h"
#include "gic.h"

#include <log/uart_print.h>

enum generic_timer_type {
    GENERIC_TIMER_HYP,      /* IRQ 26 */
    GENERIC_TIMER_VIR,      /* IRQ 27 */
    GENERIC_TIMER_NSP,      /* IRQ 30 */
    GENERIC_TIMER_NUM_TYPES
};

typedef void (*generic_timer_callback_t)(void *pdata);

/* Calling this function is required only once in the entire system. */
hvmm_status_t generic_timer_init();
/* Enable the timer interrupt. Specified by timer type */
hvmm_status_t generic_timer_enable_int(enum generic_timer_type type);
/* Disable the timer. Specified by timer type */
hvmm_status_t generic_timer_disable_int(enum generic_timer_type type);
/*
 * Sets time interval. Converts from microseconds
 * to count and sets time interval.
 */
hvmm_status_t generic_timer_set_tval(enum generic_timer_type type,
                    uint32_t tval);
/* Enables timer irq.  */
hvmm_status_t generic_timer_enable_irq(enum generic_timer_type type);
/* Adds callback funtion. Called when occur timer interrupt */
hvmm_status_t generic_timer_set_callback(enum generic_timer_type type,
                generic_timer_callback_t callback);

#endif

