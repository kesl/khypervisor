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

uint64_t oldertime = 0;


void timer_switch_next_guest(void *pdata){
	struct arch_regs *regs = pdata;

	struct timeval timeval;
	uint64_t pct = read_cntpct();
	uint32_t tval = read_cnthp_tval();
	timer_get_time(&timeval);
	
	uart_print( "timer_c2t:"); uart_print_hex64(timer_c2t(pct)); uart_print("\n\r");
	uart_print( "oldertime:"); uart_print_hex64(oldertime); uart_print("\n\r");
	uart_print( "newtime:"); uart_print_hex64(timeval.tv_sec); uart_print("\n\r");
	uart_print( "compare timer:"); uart_print_hex64(timeval.tv_sec - oldertime); uart_print("\n\r");
	oldertime = timeval.tv_sec;
	
	uart_print( "cntpct:"); uart_print_hex64(pct); uart_print("\n\r");
	uart_print( "cnth_tval:"); uart_print_hex32(tval); uart_print("\n\r");

	uart_print( "passing time:"); uart_print_hex64(timeval.tv_sec);uart_print("\n\r");
	/* Test guest context switch */
	if ( (regs->cpsr & 0x1F) != 0x1A ) {
    	/* Not from Hyp, switch the guest context */
        context_switch_to_next_guest( regs );
	}
}

void timer_test_scheduling(){
	void timer_switch_next_guest(void *pdata);
	timer_init(timer_sched);
	/* 100Mhz -> 1 count == 10ns */
	/* 0x5F5E100 -> 1seconds */
	/* 0x3B9ACA00 -> 10seconds */
	/* 1000 milliseconds -> 1seconds */	
	timer_set_interval(timer_sched, 1000);	
	timer_add_callback(timer_sched, &timer_switch_next_guest);
	timer_start(timer_sched);
}
