#include <gic.h>
#include <vgic.h>
#include "context.h"
#include "hvmm_trace.h"
#include "armv7_p15.h"
#include "timer.h"
#include "guest.h"
#if defined(CFG_BOARD_ARNDALE)
#include "pwm.h"
#endif
#include "virq.h"
#include "vdev/vdev_timer.h"

#include <config/cfg_platform.h>
#include <log/print.h>

static void test_start_timer(void)
{
    uint32_t ctl;
    uint32_t tval;
    uint64_t pct;
    HVMM_TRACE_ENTER();
    /* every second */
    tval = read_cntfrq();
    write_cntp_tval(tval);
    pct = read_cntpct();
    uart_print("cntpct:");
    uart_print_hex64(pct);
    uart_print("\n\r");
    uart_print("cntp_tval:");
    uart_print_hex32(tval);
    uart_print("\n\r");
    /* enable timer */
    ctl = read_cntp_ctl();
    ctl |= 0x1;
    write_cntp_ctl(ctl);
    HVMM_TRACE_EXIT();
}

void interrupt_nsptimer(int irq, void *pregs, void *pdata)
{
    uint32_t ctl;
    struct arch_regs *regs = pregs;
    uart_print("=======================================\n\r");
    HVMM_TRACE_ENTER();
    /* Disable NS Physical Timer Interrupt */
    ctl = read_cntp_ctl();
    ctl &= ~(0x1);
    write_cntp_ctl(ctl);
    /* Trigger another interrupt */
    test_start_timer();
    /* Test guest context switch */
    if ((regs->cpsr & 0x1F) != 0x1A) {
        /* Not from Hyp, switch the guest context */
        context_dump_regs(regs);
        context_switchto(sched_policy_determ_next());
    }
    HVMM_TRACE_EXIT();
    uart_print("=======================================\n\r");
}
#if defined(CFG_BOARD_ARNDALE)
void interrupt_pwmtimer(void *pdata)
{
    pwm_timer_disable_int();
    uart_print("=======================================\n\r");
    HVMM_TRACE_ENTER();
    HVMM_TRACE_EXIT();
    uart_print("=======================================\n\r");
    pwm_timer_enable_int();
}

hvmm_status_t hvmm_tests_gic_pwm_timer(void)
{
    /* Testing pwm timer event (timer1, Interrupt ID : 69),
     * Cortex-A15 exynos5250
     * - Periodically triggers timer interrupt
     * - Just print uart_print
     */
    HVMM_TRACE_ENTER();
    pwm_timer_init();
    pwm_timer_set_callback(&interrupt_pwmtimer);
    pwm_timer_enable_int();
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
#endif

hvmm_status_t hvmm_tests_gic_timer(void)
{
    /* Testing Non-secure Physical Timer Event
     * (PPI2, Interrupt ID:30), Cortex-A15
     * - Periodically triggers timer interrupt
     * - switches guest context at every timer interrupt
     */
    HVMM_TRACE_ENTER();
    /* handler */
    gic_test_set_irq_handler(30, &interrupt_nsptimer, 0);
    /* configure and enable interrupt */
    gic_test_configure_irq(30,
                           GIC_INT_POLARITY_LEVEL,
                           gic_cpumask_current(),
                           GIC_INT_PRIORITY_DEFAULT);
    /* start timer */
    test_start_timer();
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

static int _timer_status[NUM_GUESTS_STATIC] = {0, };

void _timer_injection_changed_status(vmid_t vmid, uint32_t status)
{
    _timer_status[vmid] = status;
}

void callback_timer(void *pdata)
{
    vmid_t vmid;
    HVMM_TRACE_ENTER();
    vmid = context_current_vmid();
    printh("Injecting IRQ 30 to Guest:%d\n", vmid);
    /*
     * vgic_inject_virq_sw( 30, VIRQ_STATE_PENDING,
     *      GIC_INT_PRIORITY_DEFAULT, smp_processor_id(), 1);
     */
    /* SW VIRQ, No PIRQ */
    if (_timer_status[vmid] == 0)
        virq_inject(vmid, 30, 0, 0);
    HVMM_TRACE_EXIT();
}

hvmm_status_t hvmm_tests_vgic(void)
{
    /* VGIC test
     *  - Implementation Not Complete
     *  - TODO: specify guest to receive the virtual IRQ
     *  - Once the guest responds to the IRQ, Virtual Maintenance
     *    Interrupt service routine should be called
     *      -> ISR implementation is empty for the moment
     *      -> This should handle completion of deactivation and further
     *         injection if there is any pending virtual IRQ
     */
    int i;
    vtimer_set_callback_chagned_status(&_timer_injection_changed_status);
    for (i = 0; i < NUM_GUESTS_STATIC; i++)
        _timer_status[i] = 1;

    timer_add_callback(TIMER_SCHED, &callback_timer);
    return HVMM_STATUS_SUCCESS;
}
