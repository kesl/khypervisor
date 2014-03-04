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

/**
* @brief Numbering generic timer each level : HYP, NSP, VIR
* Calling this function is required only once in the entire system.
* @return HVMM_STATUS_SUCCESS (It takes "0")
*/
hvmm_status_t generic_timer_init();

/**
* @brief Enable timer interrupt. Specified by timer type
* @param enum generic_timer_type type
* @return HVMM_STATUS_UNSUPPORTED_FEATURE or HVMM_STATUS_SUCCESS
*/
hvmm_status_t generic_timer_enable_int(enum generic_timer_type type);

/**
* @brief Disable the timer. Specified by timer type
* @param enum generic_timer_type type
* @return HVMM_STATUS_UNSUPPORTED_FEATURE or HVMM_STATUS_SUCCESS
*/
hvmm_status_t generic_timer_disable_int(enum generic_timer_type type);

/*
 * Sets time interval. Converts from microseconds
 * to count and sets time interval.
 */


/**
* @brief Setting generic timer value
* @param enum generic_timer_type type, uint32_t tval
* @return HVMM_STATUS_SUCCESS or HVMM_STATUS_UNSUPPORTED_FEATURE
*/
hvmm_status_t generic_timer_set_tval(enum generic_timer_type type,
                    uint32_t tval);

/**
* @brief Enable generic timer irq
* @param enum generic_timer_type type
* @return HVMM_STATUS_UNSUPPORTED_FEATURE or HVMM_STATUS_SUCCESS
*/
hvmm_status_t generic_timer_enable_irq(enum generic_timer_type type);

/* Adds callback funtion. Called when occur timer interrupt */

/**
* @brief Adds callback funtion. Called when occur timer interrupt
* @param enum generic_timer_type type, generic_timer_callback_t callback
* @return HVMM_STATUS_SUCCESS
*/
hvmm_status_t generic_timer_set_callback(enum generic_timer_type type,
                generic_timer_callback_t callback);

#endif

