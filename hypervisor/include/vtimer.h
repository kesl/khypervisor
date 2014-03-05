#ifndef __TIMER_H__
#define __TIMER_H__

#include "hvmm_types.h"
#include "arch_types.h"

/*
 * Implements Timer functionality such as,
 *
 * - vtimer channel: periodic callback at a given time interval
 * - current time: time since boot-up
 *
 * ==Example Usage==
 *
 * Initialize the vtimer module
 *  vtimer_init(VTIMER_SCHED);
 *
 * Set interval 10 to vtimer_sched channel
 *  vtimer_set_interval( VTIMER_SCHED, 10 );
 *
 * Set cb1 callback function to vtimer_scehd channel.
 * cb1 call after 10 interval.
 *  vtimer_set_callback( VTIMER_SCHED, &cb1 );
 *
 * Start vtimer_sched channel. cb1 call after 10 interval.
 *  vtimer_start( VTIMER_SCHED );
 */

#define TIMER_MAX_CHANNEL_CALLBACKS 8

enum vtimer_channel_type {
    VTIMER_SCHED = 0,
    TIMER_NUM_MAX_CHANNELS
};

typedef void(*vtimer_callback_t)(void *pdata);

struct vtimer_channel {
    uint32_t interval_us;
    vtimer_callback_t callbacks[TIMER_MAX_CHANNEL_CALLBACKS];
};

/*
 * Calling this function is required only once in the entire system
 * prior to calls to other functions of Timer module.
 */
hvmm_status_t vtimer_init(enum vtimer_channel_type channel);

/*
 * Starts the vtimer channel specified by 'channel'. The callback,
 * if set, will be periodically called until it's unset or the channel stops by
 * 'vtimer_stop(vtimer_channel)'
 */
hvmm_status_t vtimer_start(enum vtimer_channel_type channel);

/*
 *  Stops the vtimer channel specified by 'channel'
 */
hvmm_status_t vtimer_stop(enum vtimer_channel_type channel);

/*
 * Sets time interval, in microseconds, for the vtimer channel.
 * If the channel has been started and a callback function is set,
 * it will be called in the next interval
 */
hvmm_status_t vtimer_set_interval(enum vtimer_channel_type channel,
                uint32_t interval_us);

/*
 * Returns the time interval for the vtimer channel if it was set previously.
 * Unknown value is returned otherwise.
 */
uint32_t vtimer_get_interval(enum vtimer_channel_type channel);

/*
 * Adds a callback function for the vtimer channel.
 */
hvmm_status_t vtimer_add_callback(enum vtimer_channel_type channel,
                vtimer_callback_t handler);

/*
 * Removes the callback function from the vtimer channel's
 * registered callback function list
 * if it previously has been added.
 */
hvmm_status_t vtimer_remove_callback(enum vtimer_channel_type channel,
        vtimer_callback_t handler);

/*
 * Converts from microseconds to system counter.
 */
uint64_t vtimer_t2c(uint64_t time_us);


struct vtimer_ops {
    /** Set the vtimer expire callbacks */
    hvmm_status_t (*set_callbacks)(vtimer_callback_t,
            void *user);

    /** Calling this function is required only once in the entire system. */
    hvmm_status_t (*init)(void);

    /** Enable the vtimer interrupt */
    hvmm_status_t (*enable)(void);

    /** Disable the vtimer interrupt */
    hvmm_status_t (*diable)(void);

    /** Register the timer interrupt */
    hvmm_status_t (*request_irq)(void);

    /** Free the timer interrupt */
    hvmm_status_t (*free_irq)(void);

    /** Set vtimer duration */
    hvmm_status_t (*set_interval)(uint32_t);

    /** Dump state of the vtimer */
    hvmm_status_t (*dump)(void);

};

struct vtimer_module {
    /** tag must be initialized to HAL_TAG */
    uint32_t tag;

    /**
     * Version of the module-specific device API. This value is used by
     * the derived-module user to manage different device implementations.
     *
     * The module user is responsible for checking the module_api_version
     * and device version fields to ensure that the user is capable of
     * communicating with the specific module implementation.
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
    struct vtimer_ops *ops;

};

extern struct vtimer_module _vtimer_module;

#endif
