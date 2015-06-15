/*
 * timer.c
 * --------------------------------------
 * Implementation of ARMv8 Generic Timer
 * Responsible: Inkyu Han
 */

#include <arch_types.h>
#include <k-hypervisor-config.h>
#include <hvmm_trace.h>
#include <armv8_processor.h>
#include <timer.h>
#include <interrupt.h>
#include <log/print.h>
#include <smp.h>

static timer_callback_t _host_callback[NUM_CPUS];
static timer_callback_t _guest_callback[NUM_CPUS];

static struct timer_ops *_ops;

/*
 * Converts from microseconds to system counter.
 */
static inline uint64_t timer_t2c(uint64_t time_us)
{
    return time_us * COUNT_PER_USEC;
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
    if (_host_callback[cpu])
        _host_callback[cpu](pregs);
    if (_guest_callback[cpu])
        _guest_callback[cpu](pregs);
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
    uint32_t cpu = smp_processor_id();

    _host_callback[cpu] = func;

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t timer_guest_set_callback(timer_callback_t func)
{
    uint32_t cpu = smp_processor_id();

    _guest_callback[cpu] = func;

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
    //uint32_t cpu = smp_processor_id();

    _ops = _timer_module.ops;

    /* timer_hw_init() */
    if (_ops->init)
        _ops->init();

    timer_requset_irq(irq);

    return HVMM_STATUS_SUCCESS;
}
uint64_t savecnt;

void set_timer_cnt(void)
{
    savecnt = read_cntpct();
}

uint64_t get_timer_savecnt(void)
{
    return savecnt;
}
uint64_t get_timer_curcnt(void)
{
    return read_cntpct();
}
uint32_t get_timer_interval_us(uint64_t after, uint64_t before)
{
    return (uint32_t)(after - before) / COUNT_PER_USEC;
}
