#ifndef __GENERIC_TIMER_H__
#define __GENERIC_TIMER_H__

#include "armv7_p15.h"
#include "uart_print.h"
#include "gic.h"

typedef enum {
    GENERIC_TIMER_HYP,      /* IRQ 26 */
    GENERIC_TIMER_VIR,      /* IRQ 27 */
    GENERIC_TIMER_NSP,      /* IRQ 30 */
    GENERIC_TIMER_NUM_TYPES
} generic_timer_type_t;

typedef void (*generic_timer_callback_t)(void *pdata);

/* Calling this function is required only once in the entire system. */
hvmm_status_t generic_timer_init();
/* Enable the timer interrupt. Specified by timer type */
hvmm_status_t generic_timer_enable_int(generic_timer_type_t type);
/* Disable the timer. Specified by timer type */
hvmm_status_t generic_timer_disable_int(generic_timer_type_t type);
/* Sets time interval. Converts from microseconds to count and sets time interval.*/
hvmm_status_t generic_timer_set_tval(generic_timer_type_t type, uint32_t tval);
/* Enables timer irq.  */
hvmm_status_t generic_timer_enable_irq(generic_timer_type_t type);
/* Adds callback funtion. Called when occur timer interrupt */
hvmm_status_t generic_timer_set_callback(generic_timer_type_t type, generic_timer_callback_t callback);

#endif

