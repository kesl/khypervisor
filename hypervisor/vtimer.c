/*
 * vtimer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */

#include <arch_types.h>
#include <k-hypervisor-config.h>
#include <hvmm_trace.h>
#include <vtimer.h>

static struct vtimer_channel _channels[TIMER_NUM_MAX_CHANNELS];
static struct vtimer_ops *_vtimer_ops;

static void _vtimer_each_callback_channel(enum vtimer_channel_type channel,
                void *param)
{
    int i;

    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++)
        if (_channels[channel].callbacks[i])
            _channels[channel].callbacks[i](param);
}

static int _vtimer_channel_num_callbacks(enum vtimer_channel_type channel)
{
    int i, count = 0;

    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++)
        if (_channels[channel].callbacks[i])
            count++;

    return count;
}

static void _vtimer_hw_callback(void *pdata)
{
    _vtimer_ops->diable();
    _vtimer_each_callback_channel(VTIMER_SCHED, pdata);
    _vtimer_ops->set_interval(_channels[VTIMER_SCHED].interval_us);
    _vtimer_ops->enable();
}

hvmm_status_t vtimer_init(enum vtimer_channel_type channel)
{
    int i;

    for (i = 0; i < TIMER_MAX_CHANNEL_CALLBACKS; i++)
        _channels[channel].callbacks[i] = 0;

    _vtimer_ops = _vtimer_module.ops;
    _vtimer_ops->init();

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vtimer_start(enum vtimer_channel_type channel)
{
    if (_vtimer_channel_num_callbacks(channel) > 0) {
        _vtimer_ops->set_callbacks(&_vtimer_hw_callback, (void *)0);
        _vtimer_ops->set_interval(_channels[channel].interval_us);
        _vtimer_ops->request_irq();
        _vtimer_ops->enable();
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vtimer_stop(enum vtimer_channel_type channel)
{
    _vtimer_ops->diable();

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vtimer_set_interval(enum vtimer_channel_type channel,
                uint32_t interval_us)
{
    _channels[channel].interval_us = vtimer_t2c(interval_us);

    return HVMM_STATUS_SUCCESS;
}

uint32_t vtimer_get_interval(enum vtimer_channel_type channel)
{
    return _channels[channel].interval_us;
}

hvmm_status_t vtimer_add_callback(enum vtimer_channel_type channel,
                    vtimer_callback_t callback)
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

hvmm_status_t vtimer_remove_callback(enum vtimer_channel_type channel,
                vtimer_callback_t callback)
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

uint64_t vtimer_t2c(uint64_t time_us)
{
    return time_us * COUNT_PER_USEC;
}
