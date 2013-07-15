
#include "hyp_config.h"
#include "uart_print.h"
#include "mm.h"
#include "armv7_p15.h"
#include "arch_types.h"
#include "gic.h"
#include "interrupt.h"
#include "context.h"
#include "timer.h"


void hyp_abort_infinite(void)
{
	while(1);
}

/* TODO:
	- Move trap handlers to trap.c 
 */
hvmm_status_t _hyp_trap_dabort( struct arch_regs *regs )
{
	context_dump_regs( regs );
	hyp_abort_infinite();

	return HVMM_STATUS_UNKNOWN_ERROR;
}

hvmm_status_t _hyp_trap_irq( struct arch_regs *regs )
{

	HVMM_TRACE_ENTER();

	gic_interrupt(0, regs);

	HVMM_TRACE_EXIT();

	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t _hyp_trap_unhandled( struct arch_regs *regs )
{

	context_dump_regs( regs );
	hyp_abort_infinite();

	return HVMM_STATUS_UNKNOWN_ERROR;
}

/*
 * hvc #imm handler
 */
hyp_hvc_result_t _hyp_hvc_service(struct arch_regs *regs)
{
	unsigned int hsr = read_hsr();
	unsigned int iss = hsr & 0xFFFF;
	unsigned int ec = (hsr >> 26);
	uart_print("[hvc] _hyp_hvc_service: enter\n\r");

	if ( ec == 0x12 && iss == 0xFFFF ) {
		uart_print("[hvc] enter hyp\n\r");
		context_dump_regs( regs );
		return HYP_RESULT_STAY;
	}

	switch( iss ) {
		case 0xFFFE:
			/* hyp_ping */
			uart_print("[hyp] _hyp_hvc_service:ping\n\r");
			context_dump_regs( regs );
			break;
		case 0xFFFD:
			/* hsvc_yield() */
			uart_print("[hyp] _hyp_hvc_service:yield\n\r");
			context_dump_regs( regs );
			context_switch_to_next_guest(regs);
			break;
		default:
			uart_print("[hyp] _hyp_hvc_service:unknown hsr.iss="); uart_print_hex32( iss ); uart_print("\n\r" );
			uart_print("[hyp] hsr.ec="); uart_print_hex32( ec ); uart_print("\n\r" );
			uart_print("[hyp] hsr="); uart_print_hex32( hsr ); uart_print("\n\r" );
			context_dump_regs( regs );
			if ( ec == 0x20 ) {
				// Prefetch Abort routed to Hyp mode
			}
			hyp_abort_infinite();
			break;
	}
	uart_print("[hyp] _hyp_hvc_service: done\n\r");
	return HYP_RESULT_ERET;
}

void hyp_main(void)
{
	hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;
	uart_print("[hyp_main] Starting...\n\r");

	/* Initialize Memory Management */
	ret = hvmm_mm_init();

	/* Initialize Interrupt Management */
	ret = hvmm_interrupt_init();
	if ( ret != HVMM_STATUS_SUCCESS ) {
		uart_print("[hyp_main] interrupt initialization failed...\n\r");
	}

	/* Initialize Guests */
	context_init_guests();

	/* Interrupt test */
	//hvmm_interrupt_test();
	timer_test_scheduling();

	/* Switch to the first guest */
	context_switch_guest();

	/* The code flow must not reach here */
	uart_print("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n\r");
	hyp_abort_infinite();
}

