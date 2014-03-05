/*
 * timer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */

#include <arch_types.h>
#include <k-hypervisor-config.h>
#include <hvmm_trace.h>
#include <timer.h>

static struct timer_channel _channels[TIMER_NUM_MAX_CHANNELS];
static struct timer_ops *_timer_ops;

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
    _timer_ops->disable();
    _timer_each_callback_channel(TIMER_SCHED, pdata);
    _timer_ops->set_interval(_channels[TIMER_SCHED].interval_us);
    _timer_ops->enable();
}

hvmm_status_t timer_init(enum timer_channel_type channel)
{
    int i;

    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++)
        _channels[channel].callbacks[i] = 0;

    _timer_ops = _timer_module.ops;
    _timer_ops->init();

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_start(enum timer_channel_type channel)
{
    if (_timer_channel_num_callbacks(channel) > 0) {
        _timer_ops->set_callbacks(&_timer_hw_callback, (void *)0);
        _timer_ops->set_interval(_channels[channel].interval_us);
        _timer_ops->request_irq();
        _timer_ops->enable();
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_stop(enum timer_channel_type channel)
{
    _timer_ops->disable();

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
