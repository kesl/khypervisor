#ifndef __TIMER_H__
#define __TIMER_H__

#include "hvmm_types.h"
#include "arch_types.h"

/*
 * Implements Timer functionality such as,
 *
 * - timer channel: periodic callback at a given time interval
 * - current time: time since boot-up
 *
 * ==Example Usage==
 *
 * Initialize the timer module
 *  timer_init(timer_sched);
 *
 * Set interval 10 to timer_sched channel
 *  timer_set_interval( timer_sched, 10 );
 *
 * Set cb1 callback function to timer_scehd channel.
 * cb1 call after 10 interval.
 *  timer_set_callback( timer_sched, &cb1 );
 *
 * Start timer_sched channel. cb1 call after 10 interval.
 *  timer_start( timer_sched );
 */

#define TIMER_MAX_CHANNEL_CALLBACKS 8

enum timer_channel_type {
    timer_sched = 0,
    TIMER_NUM_MAX_CHANNELS
};

typedef void(*timer_callback_t)(void *pdata);

struct timer_channel {
    uint32_t interval_us;
    timer_callback_t callbacks[TIMER_MAX_CHANNEL_CALLBACKS];
};

/*
 * Calling this function is required only once in the entire system
 * prior to calls to other functions of Timer module.
 */
hvmm_status_t timer_init(enum timer_channel_type channel);

/*
 * Starts the timer channel specified by 'channel'. The callback,
 * if set, will be periodically called until it's unset or the channel stops by
 * 'timer_stop(timer_channel)'
 */
hvmm_status_t timer_start(enum timer_channel_type channel);

/*
 *  Stops the timer channel specified by 'channel'
 */
hvmm_status_t timer_stop(enum timer_channel_type channel);

/*
 * Sets time interval, in microseconds, for the timer channel.
 * If the channel has been started and a callback function is set,
 * it will be called in the next interval
 */
hvmm_status_t timer_set_interval(enum timer_channel_type channel,
                uint32_t interval_us);

/*
 * Returns the time interval for the timer channel if it was set previously.
 * Unknown value is returned otherwise.
 */
uint32_t timer_get_interval(enum timer_channel_type channel);

/*
 * Adds a callback function for the timer channel.
 */
hvmm_status_t timer_add_callback(enum timer_channel_type channel,
                timer_callback_t handler);

/*
 * Removes the callback function from the timer channel's
 * registered callback function list
 * if it previously has been added.
 */
hvmm_status_t timer_remove_callback(enum timer_channel_type channel,
        timer_callback_t handler);

/*
 * Converts from microseconds to system counter.
 */
uint64_t timer_t2c(uint64_t time_us);

#endif
