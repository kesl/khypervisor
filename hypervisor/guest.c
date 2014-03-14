#include <k-hypervisor-config.h>
#include <context.h>
#include <test/tests.h>
#include <version.h>
#include <log/uart_print.h>
#include <log/print.h>
#include <hvmm_trace.h>
#include <guest.h>
#include <timer.h>
#include <gic_regs.h>
#include <mm.h>
#include <interrupt.h>
#include <vdev.h>
#include <virqmap.h>

#define GUEST_SCHED_TICK 100000

vmid_t sched_policy_determ_next(void)
{
    vmid_t next = context_next_vmid(context_current_vmid());
    if (next == VMID_INVALID)
        next = context_first_vmid();
    return next;
}

void guest_schedule(void)
{
    /* Switch request, actually performed at trap exit */
    context_switchto(sched_policy_determ_next());
}

static void guest_sched_start()
{
    /* Switch to the first guest */
    context_switch_to_initial_guest();
}

void guest_switch_to_next_guest(void *pdata)
{
    struct arch_regs *regs = pdata;
#if 0 /* ignore message due to flood log message */
    uint64_t pct = read_cntpct();
    uint32_t tval = read_cnthp_tval();
    uart_print("cntpct:");
    uart_print_hex64(pct);
    uart_print("\n\r");
    uart_print("cnth_tval:");
    uart_print_hex32(tval);
    uart_print("\n\r");
#endif
    /*
     * Note: As of context_switchto() and context_perform_switch()
     * are available, no need to test if trapped from Hyp mode.
     * context_perform_switch() takes care of it
     */
    /* Test guest context switch */
    if ((regs->cpsr & 0x1F) != 0x1A)
        guest_schedule();
}

static void guest_sched_init()
{
    timer_add_callback(TIMER_SCHED, &guest_switch_to_next_guest);
    timer_start(TIMER_SCHED);
}

static void guest_timer_init(uint32_t duration)
{
    timer_init(TIMER_SCHED);
    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
    timer_set_interval(TIMER_SCHED, duration);
}

void start_guest(void)
{
    init_print();
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;
    printh("[%s : %d] Starting...\n", __func__, __LINE__);

    /* Initialize Memory Management */
    ret = hvmm_mm_init();
    if (ret != HVMM_STATUS_SUCCESS)
        uart_print("[start_guest] virtual memory initialization failed...\n\r");

    /* Initialize Interrupt Management */
    ret = hvmm_interrupt_init();
    if (ret != HVMM_STATUS_SUCCESS)
        uart_print("[start_guest] interrupt initialization failed...\n\r");

    /* Initialize Guests */
    context_init_guests();

    /* Initialize Timer */
    guest_timer_init(GUEST_SCHED_TICK);

    /* Initialize Scheduler */
    guest_sched_init();

    /* Initialize Virtual Devices */
    ret = vdev_init();
    if (ret != HVMM_STATUS_SUCCESS)
        uart_print("[start_guest] virtual device initialization failed...\n\r");

    /* Initialize PIRQ to VIRQ mapping */
    virqmap_init();

    /* Begin running test code for newly implemented features */
    hvmm_tests_main();

    /* Print Banner */
    printH("%s", BANNER_STRING);

    /* Switch to the first guest */
    guest_sched_start();

    /* The code flow must not reach here */
    uart_print("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n\r");
    hyp_abort_infinite();
}

