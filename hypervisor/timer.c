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
#include <interrupt.h>
#include <log/print.h>

static struct timer_ops *_ops;

static timer_callback_t _host_callback;
static timer_callback_t _guest_callback;

/*
 * Converts from microseconds to system counter.
 */
static inline uint64_t timer_t2c(uint64_t time_us)
{
    return time_us * COUNT_PER_USEC;
}

/*
 * Starts the timer channel specified by 'channel'. The callback,
 * if set, will be periodically called until it's unset or the channel stops by
 * 'timer_stop(timer_channel)'
 */
static hvmm_status_t timer_start(void)
{
    if (_ops->enable)
        return _ops->enable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}
/*
 *  Stops the timer channel specified by 'channel'
 */
static hvmm_status_t timer_stop(void)
{
    if (_ops->disable)
        return _ops->disable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * Sets time interval, in microseconds, for the timer channel.
 * If the channel has been started and a callback function is set,
 * it will be called in the next interval
 */
static hvmm_status_t timer_set_interval(uint32_t interval_us)
{
    if (_ops->set_interval)
        return _ops->set_interval(timer_t2c(interval_us));

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

static void timer_handler(int irq, void *pregs, void *pdata)
{
    timer_stop();
    if (_host_callback)
        _host_callback(pregs);
    if (_guest_callback)
        _guest_callback(pregs);
    timer_set_interval(GUEST_SCHED_TICK);
    timer_start();
}

static hvmm_status_t timer_requset_irq(uint32_t irq)
{
    if (interrupt_request(irq, &timer_handler))
        return HVMM_STATUS_UNSUPPORTED_FEATURE;

    return interrupt_host_configure(irq);
}

static hvmm_status_t timer_host_set_callback(timer_callback_t func)
{
    _host_callback = func;

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t timer_guest_set_callback(timer_callback_t func)
{
    _guest_callback = func;

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_set(struct timer_val *timer, uint32_t host)
{
    if (host) {
        timer_stop();
        timer_host_set_callback(timer->callback);
        timer_set_interval(timer->interval_us);
        timer_start();
    } else
        timer_guest_set_callback(timer->callback);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_init(uint32_t irq)
{
    _ops = _timer_module.ops;

    if (_ops->init)
        _ops->init();

    timer_requset_irq(irq);

    return HVMM_STATUS_SUCCESS;
}
