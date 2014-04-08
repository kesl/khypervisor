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

struct timer_struct {
    struct timer timer;
    int32_t count_per_irq;
};

static struct timer_struct timers[MAX_TIMERS_COUNT];
uint32_t timers_index;
static struct timer_ops *ops;

/*
 *  Stops the timer.
 */
static hvmm_status_t timer_stop(void)
{
    if (ops->disable)
        return ops->disable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * Calculates count per IRQ using interval value.
 */
static inline int32_t calculate_count_per_irq(int32_t interval_us)
{
    int32_t count;

    if (interval_us <= 0)
        return ERROR_COUNT;

    count = interval_us / COUNT_PER_USEC;

    /* needs to compensate count value. Because zero count value is invalid. */
    return count == 0 ? 1 : count;
}

/*
 * Checks each timers and calls its callback when interval was expired.
 */
static void check_each_timers(const void *pregs)
{
    uint32_t i;

    for (i = 0; i < timers_index && i < MAX_TIMERS_COUNT; i++) {
        /* if interval_us is negative value, skip it. */
        if (timers[i].timer.interval_us > 0) {
            /* consumes count_per_irq value. */
            timers[i].count_per_irq--;

            if (timers[i].count_per_irq < 0) {
                /* calls callback with pregs */
                timers[i].timer.callback(pregs);

                /* re-calculates count_per_irq. */
                timers[i].count_per_irq = calculate_count_per_irq(
                        timers[i].timer.interval_us);
            }
        }
    }
}

/*
 * Converts from microseconds to system counter.
 */
static inline uint64_t convert_us_to_system_counter(uint64_t time_us)
{
    return time_us * COUNT_PER_USEC;
}

/*
 * Sets the timer interval(microsecond).
 */
static hvmm_status_t set_interval(uint32_t interval_us)
{
    if (ops->set_interval)
        return ops->set_interval(
                convert_us_to_system_counter(interval_us));

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * Starts the timer.
 */
static hvmm_status_t timer_start(void)
{
    if (ops->enable)
        return ops->enable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * Handles IRQ response.
 */
static void timer_handler(int irq, const void *pregs, const void *pdata)
{
    timer_stop();
    check_each_timers(pregs);
    set_interval(COUNT_PER_USEC);
    timer_start();
}

/*
 * Requests IRQ.
 */
static hvmm_status_t requset_irq(uint32_t irq)
{
    if (interrupt_request(irq, &timer_handler))
        return HVMM_STATUS_UNSUPPORTED_FEATURE;

    return interrupt_host_configure(irq);
}

/*
 * Checks whether the array of timers is full or not.
 */
static inline uint32_t is_timers_full(void)
{
    return timers_index >= MAX_TIMERS_COUNT ? TRUE : FALSE;
}

hvmm_status_t timer_init(uint32_t irq)
{
    ops = _timer_module.ops;

    timers_index = 0;

    if (ops->init)
        ops->init();

    requset_irq(irq);

    timer_start();

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_set(const struct timer *timer)
{
    /* checks it first. */
    if (is_timers_full())
        return HVMM_STATUS_UNSUPPORTED_FEATURE;

    struct timer_struct new_timer;
    new_timer.timer.callback = timer->callback;
    new_timer.timer.interval_us = timer->interval_us;
    new_timer.count_per_irq = calculate_count_per_irq(timer->interval_us);

    /*
     * TODO: Storing new_time into array of timers needs a lock mechanism
     * for preventing concurrent access.
     */
    timers[timers_index++] = new_timer;

    return HVMM_STATUS_SUCCESS;
}
