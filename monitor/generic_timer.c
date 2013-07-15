#include "generic_timer.h"
#include "gic.h"



/* Types */

/* External Declarations */

/* Local Declarations */
static void _generic_timer_hyp_irq_handler(int irq, void *regs, void *pdata);

/* Local Data */
static uint32_t _timer_irqs[GENERIC_TIMER_NUM_TYPES];
static uint32_t _tvals[GENERIC_TIMER_NUM_TYPES];

static generic_timer_callback_t _callback;

hvmm_status_t generic_timer_init(struct timer_source *ts)
{
/*
	ts->name = "gen-timer";
	ts->start = &generic_timer_start;
	ts->stop = &generic_timer_stop;
	ts->add_callback = &generic_timer_add_callback;
	ts->remove_callback = &generic_timer_remove_callback;
	ts->get_time = &generic_timer_get_time;
*/
//	timer_register(&ts);	


	_timer_irqs[GENERIC_TIMER_HYP] = 26;
	_timer_irqs[GENERIC_TIMER_NSP] = 27;
	_timer_irqs[GENERIC_TIMER_VIR] = 30;

	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t generic_timer_set_tval(generic_timer_type_t type, uint32_t tval)
{
	hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

	if ( type == GENERIC_TIMER_HYP) {
		_tvals[type] = tval;
		generic_timer_reg_write(GENERIC_TIMER_REG_HYP_TVAL, tval);
		result = HVMM_STATUS_SUCCESS;
	} else {
		/* DIY */
	}

	return result;
}

hvmm_status_t generic_timer_enable_int(generic_timer_type_t type )
{
	uint32_t ctrl;
	hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

	uart_print("[generic_timer_start]\n\r");	

	if ( type == GENERIC_TIMER_HYP ) {
		ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_HYP_CTRL);
		ctrl |= GENERIC_TIMER_CTRL_ENABLE;		
		ctrl &= ~GENERIC_TIMER_CTRL_IMASK;
	
		generic_timer_reg_write(GENERIC_TIMER_REG_HYP_CTRL, ctrl);
		result = HVMM_STATUS_SUCCESS;
	}

	return result;
}

hvmm_status_t generic_timer_disable_int(generic_timer_type_t type)
{
	uint32_t ctrl;
	hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

	uart_print("[generic_timer_stop]\n\r");

	if ( type == GENERIC_TIMER_HYP ) {
		ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_HYP_CTRL);
		ctrl &= ~GENERIC_TIMER_CTRL_ENABLE;
		ctrl |= GENERIC_TIMER_CTRL_IMASK;

		generic_timer_reg_write(GENERIC_TIMER_REG_HYP_CTRL, ctrl);
		result = HVMM_STATUS_SUCCESS;
	}

	return result;
}

static void _generic_timer_hyp_irq_handler(int irq, void *regs, void *pdata)
{
//	generic_timer_disable_int(GENERIC_TIMER_HYP);

	_callback(regs);

//	generic_timer_set_tval(GENERIC_TIMER_HYP, _tvals[GENERIC_TIMER_HYP] );

//	generic_timer_enable_int(GENERIC_TIMER_HYP);
}

hvmm_status_t generic_timer_enable_irq(generic_timer_type_t type)
{
	hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

	if ( type == GENERIC_TIMER_HYP ) {
		uint32_t irq = _timer_irqs[type];

		gic_test_set_irq_handler(irq, &_generic_timer_hyp_irq_handler, 0);

	    gic_test_configure_irq(irq,
        	GIC_INT_POLARITY_LEVEL, 
        	gic_cpumask_current(),
        	GIC_INT_PRIORITY_DEFAULT );

		result = HVMM_STATUS_SUCCESS;
	}
	return result;
}

hvmm_status_t generic_timer_set_callback(generic_timer_type_t type, generic_timer_callback_t callback)
{
	HVMM_TRACE_ENTER();

	/* TODO: set callback per Generic Timer Type */
	_callback /* [type] */ = callback;

	HVMM_TRACE_EXIT();
	return HVMM_STATUS_SUCCESS;
}

void generic_timer_get_time(struct timeval* timeval)
{
	timeval->tv_sec = generic_timer_pcounter_read(); 
	timeval->tv_usec = generic_timer_vcounter_read();
}


