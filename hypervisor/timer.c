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
#include <smp.h>

struct timer {
    struct timer_val timer_info;
    int32_t count_per_irq;
};

static struct timer _timers[NUM_CPUS][MAX_TIMER];
uint32_t _timers_index[NUM_CPUS];
static struct timer_ops *_ops;

/*
 * Converts from microseconds to system counter.
 */
static inline uint64_t timer_t2c(uint64_t time_us)
{
    return time_us * COUNT_PER_USEC;
}

/*
 * Calculates count per IRQ using interval value.
 */
static inline int32_t timer_count_per_irq(int32_t interval_us)
{
    int32_t count;

    if (interval_us <= 0)
        return -1;

    count = interval_us / COUNT_PER_USEC;

    /* needs to compensate count value. Because zero count value is invalid. */
    return count == 0 ? 1 : count;
}

/*
 * Checks array of timers.
 */
static inline uint32_t timer_is_full()
{
    uint32_t cpu = smp_processor_id();

    return (_timers_index[cpu] >= MAX_TIMER);
}

/*
 * Starts the timer.
 */
static hvmm_status_t timer_start(void)
{
    /* timer_enable() */
    if (_ops->enable)
        return _ops->enable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 *  Stops the timer.
 */
static hvmm_status_t timer_stop(void)
{
    /* timer_disable() */
    if (_ops->disable)
        return _ops->disable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * Checks each timers and calls its callback when interval was expired.
 */
static void timer_check_each_timers(void *pregs)
{
    uint32_t i;
    uint32_t cpu = smp_processor_id();

    for (i = 0; i < _timers_index[cpu] && i < MAX_TIMER; i++) {
        /* if interval_us is negative value, skip it. */
        if (_timers[cpu][i].timer_info.interval_us > 0) {
            /* consumes count_per_irq value. */
            _timers[cpu][i].count_per_irq--;

            if (_timers[cpu][i].count_per_irq < 0) {
                /* calls callback with pregs */
                _timers[cpu][i].timer_info.callback(pregs);

                /* re-calculates count_per_irq. */
                _timers[cpu][i].count_per_irq = timer_count_per_irq(
                        _timers[cpu][i].timer_info.interval_us);
            }
        }
    }
}

/*
 * Sets the timer interval(microsecond).
 */
static hvmm_status_t timer_set_interval(uint32_t interval_us)
{
    /* timer_set_tval() */
    if (_ops->set_interval)
        return _ops->set_interval(timer_t2c(interval_us));

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * This method handles all timer IRQ.
 */
static void timer_handler(int irq, void *pregs, void *pdata)
{
    uint32_t cpu = smp_processor_id();

    timer_stop();
    timer_check_each_timers(pregs);
    timer_set_interval(COUNT_PER_USEC);
    timer_start();
}

static hvmm_status_t timer_requset_irq(uint32_t irq)
{
    if (interrupt_request(irq, &timer_handler))
        return HVMM_STATUS_UNSUPPORTED_FEATURE;

    return interrupt_host_configure(irq);
}

hvmm_status_t timer_set(struct timer_val *timer)
{
    struct timer stimer;
    uint32_t cpu = smp_processor_id();

    /* checks it first. */
    if (timer_is_full())
        return HVMM_STATUS_UNSUPPORTED_FEATURE;

    stimer.timer_info.callback = timer->callback;
    stimer.timer_info.interval_us = timer->interval_us;
    stimer.count_per_irq = timer_count_per_irq(timer->interval_us);

    /*
     * TODO: Storing time_val into array of timers needs a lock mechanism
     * for preventing concurrent access.
     */
    _timers[cpu][_timers_index[cpu]++] = stimer;

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_init(uint32_t irq)
{
    uint32_t cpu = smp_processor_id();

    _ops = _timer_module.ops;

    _timers_index[cpu] = 0;

    if (_ops->init)
        _ops->init();

    timer_requset_irq(irq);

    timer_start();

    return HVMM_STATUS_SUCCESS;
}
