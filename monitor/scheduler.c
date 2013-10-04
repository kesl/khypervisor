#include "scheduler.h"
#include "hvmm_trace.h"
#include "sched_policy.h"

void scheduler_schedule(void)
{
    /* Switch request, actually performed at trap exit */
    context_switchto(sched_policy_determ_next());
}

void scheduler_test_switch_to_next_guest(void *pdata){
    struct arch_regs *regs = pdata;
    uint64_t pct = read_cntpct();
    uint32_t tval = read_cnthp_tval();
    uart_print( "cntpct:"); uart_print_hex64(pct); uart_print("\n\r");
    uart_print( "cnth_tval:"); uart_print_hex32(tval); uart_print("\n\r");

    /* Note: As of context_switchto() and context_perform_switch() are available,
       no need to test if trapped from Hyp mode.
       context_perform_switch() takes care of it
     */
    /* Test guest context switch */
    if ( (regs->cpsr & 0x1F) != 0x1A ) {
        scheduler_schedule();
    }
}

void extra_timer_callback(void *pdata)
{

}

void scheduler_test_scheduling(){
    void scheduler_test_switch_to_next_guest(void *pdata);
    timer_init(timer_sched);
    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
    timer_set_interval(timer_sched, 1000000);   
    timer_add_callback(timer_sched, &scheduler_test_switch_to_next_guest);
    timer_start(timer_sched);


    timer_add_callback(timer_sched, &extra_timer_callback);
}
