#include "scheduler.h"


void scheduler_test_switch_to_next_guest(void *pdata){
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

void scheduler_test_scheduling(){
	void scheduler_test_switch_to_next_guest(void *pdata);
	timer_init(timer_sched);
	/* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
	timer_set_interval(timer_sched, 1000000);	
	timer_add_callback(timer_sched, &scheduler_test_switch_to_next_guest);
	timer_start(timer_sched);
}
