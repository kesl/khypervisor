/*
 * timer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */
#include "timer.h"
#include "generic_timer.h"

#include <config/cfg_platform.h>
#include <k-hypervisor-config.h>

static struct timer_channel _channels[TIMER_NUM_MAX_CHANNELS];

static void _timer_each_callback_channel(enum timer_channel_type channel,
                void *param)
{
    int i;
    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++)
        if (_channels[channel].callbacks[i])
            _channels[channel].callbacks[i](param);
}

static int _timer_channel_num_callbacks(enum timer_channel_type channel)
{
    int i, count = 0;
    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++)
        if (_channels[channel].callbacks[i])
            count++;
    return count;
}

static void _timer_hw_callback(void *pdata)
{
    generic_timer_disable_int(GENERIC_TIMER_HYP);
    _timer_each_callback_channel(timer_sched, pdata);
    generic_timer_set_tval(GENERIC_TIMER_HYP,
            _channels[timer_sched].interval_us);
    generic_timer_enable_int(GENERIC_TIMER_HYP);
}

hvmm_status_t timer_init(enum timer_channel_type channel)
{
    int i;
    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++)
        _channels[channel].callbacks[i] = 0;
    generic_timer_init();
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_start(enum timer_channel_type channel)
{
    if (_timer_channel_num_callbacks(channel) > 0) {
        generic_timer_set_callback(GENERIC_TIMER_HYP, &_timer_hw_callback);
        generic_timer_set_tval(GENERIC_TIMER_HYP,
                _channels[channel].interval_us);
        generic_timer_enable_irq(GENERIC_TIMER_HYP);
        generic_timer_enable_int(GENERIC_TIMER_HYP);
    }
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_stop(enum timer_channel_type channel)
{
    generic_timer_disable_int(GENERIC_TIMER_HYP);
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_set_interval(enum timer_channel_type channel,
                uint32_t interval_us)
{
    _channels[channel].interval_us = timer_t2c(interval_us);
    return HVMM_STATUS_SUCCESS;
}

uint32_t timer_get_interval(enum timer_channel_type channel)
{
    return _channels[channel].interval_us;
}

hvmm_status_t timer_add_callback(enum timer_channel_type channel,
                    timer_callback_t callback)
{
    int i;
    hvmm_status_t result = HVMM_STATUS_BUSY;
    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++) {
        if (_channels[channel].callbacks[i] == 0) {
            _channels[channel].callbacks[i] = callback;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }
    return result;
}

hvmm_status_t timer_remove_callback(enum timer_channel_type channel,
                timer_callback_t callback)
{
    int i;
    hvmm_status_t result = HVMM_STATUS_NOT_FOUND;
    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++) {
        if (_channels[channel].callbacks[i] == callback) {
            _channels[channel].callbacks[i] = 0;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }
    return result;
}

uint64_t timer_t2c(uint64_t time_us)
{
    return time_us * COUNT_PER_USEC;
}
