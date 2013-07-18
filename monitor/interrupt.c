#include "hvmm_types.h"
#include "gic.h"
#include "uart_print.h"
#include "armv7_p15.h"
#include "context.h"

hvmm_status_t hvmm_interrupt_init(void)
{
	hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

	/* Route IRQ/IFQ to Hyp Exception Vector */
	{
		uint32_t hcr;

		hcr = read_hcr(); uart_print( "hcr:"); uart_print_hex32(hcr); uart_print("\n\r");
		hcr |= HCR_IMO | HCR_FMO;
		write_hcr( hcr );
		hcr = read_hcr(); uart_print( "hcr:"); uart_print_hex32(hcr); uart_print("\n\r");
	} 
	ret = gic_init();
	return ret;
}


void interrupt_test_start_timer(void)
{
	uint32_t ctl;
	uint32_t tval;
	uint64_t pct;

	HVMM_TRACE_ENTER();


	/* every second */
	tval = read_cntfrq();
	write_cntp_tval(tval);

	pct = read_cntpct();
	uart_print( "cntpct:"); uart_print_hex64(pct); uart_print("\n\r");
	uart_print( "cntp_tval:"); uart_print_hex32(tval); uart_print("\n\r");

	/* enable timer */
	ctl = read_cntp_ctl();
	ctl |= 0x1;
	write_cntp_ctl(ctl);

	HVMM_TRACE_EXIT();
}

void interrupt_nsptimer(int irq, void *pregs, void *pdata )
{
	uint32_t ctl;
	struct arch_regs *regs = pregs;

	uart_print( "=======================================\n\r" );
	HVMM_TRACE_ENTER();

	/* Disable NS Physical Timer Interrupt */
	ctl = read_cntp_ctl();
	ctl &= ~(0x1);
	write_cntp_ctl(ctl);
	

	/* Trigger another interrupt */
	interrupt_test_start_timer();

	/* Test guest context switch */
	if ( (regs->cpsr & 0x1F) != 0x1A ) {
		/* Not from Hyp, switch the guest context */
		context_dump_regs( regs );
		context_switch_to_next_guest( regs );
	}

	HVMM_TRACE_EXIT();
	uart_print( "=======================================\n\r" );
}

hvmm_status_t hvmm_interrupt_test(void)
{
	/* Testing Non-secure Physical Timer Event (PPI2), Cortex-A15 
	 * - Periodically triggers timer interrupt
	 * - switches guest context at every timer interrupt
	 */

	HVMM_TRACE_ENTER();

	/* handler */
	gic_test_set_irq_handler( 30, &interrupt_nsptimer, 0 );

	/* configure and enable interrupt */
	gic_test_configure_irq(30, 
		GIC_INT_POLARITY_LEVEL, 
		gic_cpumask_current(), 
		GIC_INT_PRIORITY_DEFAULT );

	/* start timer */
	interrupt_test_start_timer();


	HVMM_TRACE_EXIT();
	return HVMM_STATUS_SUCCESS;
}
