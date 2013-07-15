#include "scheduler.h"


hvmm_status_t scheduler_init(void)
{
	/* Not Implement */
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t scheduler_next_event(int irq, void *pdata)
{
	/* Not Implement */
	return HVMM_STATUS_SUCCESS;
}

void timer_switch_next_guest(void *pdata){
   struct arch_regs *regs = pdata;

   uint64_t pct = read_cntpct();
   uint32_t tval = read_cnthp_tval();
   uart_print( "cntpct:"); uart_print_hex64(pct); uart_print("\n\r");
   uart_print( "cnth_tval:"); uart_print_hex32(tval); uart_print("\n\r");

   /* Test guest context switch */
   if ( (regs->cpsr & 0x1F) != 0x1A ) {
       /* Not from Hyp, switch the guest context */
       context_switch_to_next_guest( regs );
	}
}

void timer_test_scheduling(){
    void timer_switch_next_guest(void *pdata);
    timer_init(timer_sched);
    timer_set_interval(timer_sched, 0x8000000);
    timer_add_callback(timer_sched, &timer_switch_next_guest);
    timer_start(timer_sched);
}
