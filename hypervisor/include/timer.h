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
 *  timer_init(TIMER_SCHED);
 *
 * Set interval 10 to timer_sched channel
 *  timer_set_interval( TIMER_SCHED, 10 );
 *
 * Set cb1 callback function to timer_scehd channel.
 * cb1 call after 10 interval.
 *  timer_set_callback( TIMER_SCHED, &cb1 );
 *
 * Start timer_sched channel. cb1 call after 10 interval.
 *  timer_start( TIMER_SCHED );
 */

#define TIMER_MAX_CHANNEL_CALLBACKS 8

enum timer_channel_type {
    TIMER_SCHED = 0,
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


struct timer_ops {
    /** Set the callback function */
    hvmm_status_t (*set_callbacks)(timer_callback_t, void *user);

    /** The init function should only be used in the entire system */
    hvmm_status_t (*init)(void);

    /** Enable the timer interrupt */
    hvmm_status_t (*enable)(void);

    /** Disable the timer interrupt */
    hvmm_status_t (*disable)(void);

    /** Register the timer interrupt */
    hvmm_status_t (*request_irq)(void);

    /** Free the timer interrupt */
    hvmm_status_t (*free_irq)(void);

    /** Set timer duration */
    hvmm_status_t (*set_interval)(uint32_t);

    /** Dump state of the timer */
    hvmm_status_t (*dump)(void);

};

struct timer_module {
    /** tag must be initialized to HAL_TAG */
    uint32_t tag;

    /**
     * Version of the module-specific device API. This value is used by
     * the derived-module user to manage different device implementations.
     * The user who uses this module is responsible for checking
     * the module_api_version and device version fields to ensure that
     * the user is capable of communicating with the specific module
     * implementation.
     *
     */
    uint32_t version;

    /** Identifier of module */
    const char *id;

    /** Name of this module */
    const char *name;

    /** Author/owner/implementor of the module */
    const char *author;

    /** Timer Operation */
    struct timer_ops *ops;

};

extern struct timer_module _timer_module;

#endif
