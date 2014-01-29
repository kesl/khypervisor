#ifndef __PWM_H__
#define __PWM_H__
#include "arch_types.h"
#include "hvmm_types.h"
typedef void (*pwm_timer_callback_t)(void *pdata);

/* Initialize pwm timer 1 */
void pwm_timer_init();
/* enable the timer.   */
hvmm_status_t pwm_timer_enable_int();
/* Disable the timer.  */
hvmm_status_t pwm_timer_disable_int();
/* Sets time interval. Converts from microseconds to count and sets time interval.*/
hvmm_status_t pwm_timer_set_interval(uint32_t tval);
/* Enables timer irq.  */
hvmm_status_t pwm_timer_enable_irq();
/* Adds callback funtion. Called when occur timer interrupt */
hvmm_status_t pwm_timer_set_callback(pwm_timer_callback_t callback);

#endif
