#ifndef __TIMER_H__
#define __TIMER_H__

#include "hvmm_types.h"
#include "arch_types.h"
#include "gic.h"
/**
 *  Timer Interface.
 *  Call appropriate timer function.
 *
 *	==Example Usage==
 *
 *	Initialize the timer module
 *		timer_init(timer_sched);
 *
 *	Set interval 10 to timer_sched channel
 *		timer_set_interval( timer_sched, 10 );
 *
 *	Set cb1 callback function to timer_scehd channel.
 *	cb1 call after 10 interval.
 *		timer_set_callback( timer_sched, &cb1 );
 *
 *	Start timer_sched channel. cb1 call after 10 interval.
 *		timer_start( timer_sched );
 *
 *		...
 *
 *	Remove the call back
 *		timer_remove_callback(timer_sched, &cb1);
*/


/*
*	Timer channel
*	Have own's timer interrupt handler.
*/
typedef enum{
    timer_sched = 0,
} timer_channel_t;

/*
*   tv_sec : seconds
*   tv_usec : microseconds
*/
struct timeval{
    long tv_sec;
    long tv_usec;
};

/**
 * Initialize timer.
 * Write timer name.
 * Not clear what information need.
 * Before call timer_start
 */
	hvmm_status_t timer_init(timer_channel_t timer_channel);

/**
 * Start timer_channel.
 * Call before occur timer interrupt
 */
	hvmm_status_t timer_start(timer_channel_t timer_channel);

/**
 * Stop timer_channel.
 * When enter timer interrupt handler. Call this function.
 */
	hvmm_status_t timer_stop(timer_channel_t timer_channel);

/**
 * set interval.
 * timer interrupt occur next interval
 */
	hvmm_status_t timer_set_interval(timer_channel_t timer_channel, uint32_t interval);

/**
 * get interval.
 */
	hvmm_status_t timer_get_interval(timer_channel_t timer_channel);

/**
 * Register callback function. Call this funtion When timer interrupt occur.
 * handler : callback function pointer
 */
	hvmm_status_t timer_add_callback(timer_channel_t timer_channel, gic_irq_handler_t handler);
/**
 * Remove callback function.
 * handler : callback function pointer
 */
	hvmm_status_t timer_remove_callback(timer_channel_t timer_channel, gic_irq_handler_t handler);
/**
 * Get current time.
 */
void timer_get_time(struct timeval* timeval);

#endif
