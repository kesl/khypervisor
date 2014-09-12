#ifndef __TIMER_H__
#define __TIMER_H__

#include "hvmm_types.h"
#include "arch_types.h"


#define GUEST_TIMER 0
#define HOST_TIMER 1

typedef void(*timer_callback_t)(void *pdata);

struct timer_val {
    uint32_t interval_us;
    timer_callback_t callback;
};

struct timer_ops {
    /** The init function should only be used in the entire system */
    hvmm_status_t (*init)(void);

    /** Enable the timer interrupt */
    hvmm_status_t (*enable)(void);

    /** Disable the timer interrupt */
    hvmm_status_t (*disable)(void);

    /** Set timer duration */
    hvmm_status_t (*set_interval)(uint64_t);

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
/*
 * Calling this function is required only once in the entire system
 * prior to calls to other functions of Timer module.
 */
hvmm_status_t timer_init(uint32_t irq);
hvmm_status_t timer_set(struct timer_val *timer, uint32_t host);

#endif
