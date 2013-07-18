/* 
 * timer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */
#include "timer.h"
#include "generic_timer.h"

static struct timer_channel _channels[TIMER_NUM_MAX_CHANNELS];

static void _timer_hw_callback(void *pdata)
{
	generic_timer_disable_int(GENERIC_TIMER_HYP);

	_channels[timer_sched].callback(pdata);

    generic_timer_set_tval(GENERIC_TIMER_HYP, _channels[timer_sched].interval_us);
	
	generic_timer_enable_int(GENERIC_TIMER_HYP);
}

hvmm_status_t timer_init(timer_channel_t channel)
{	
	generic_timer_init();
	return HVMM_STATUS_SUCCESS;
}
	
hvmm_status_t timer_start(timer_channel_t channel)
{
	if ( _channels[channel].callback != 0 ) {

    	generic_timer_set_callback(GENERIC_TIMER_HYP, &_timer_hw_callback );
    
    	generic_timer_set_tval(GENERIC_TIMER_HYP, _channels[channel].interval_us);
    
    	generic_timer_enable_irq(GENERIC_TIMER_HYP);

    	generic_timer_enable_int(GENERIC_TIMER_HYP);
	}

	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_stop(timer_channel_t channel)
{
	generic_timer_disable_int(GENERIC_TIMER_HYP);
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_set_interval(timer_channel_t channel, uint32_t interval_us)
{
	_channels[channel].interval_us = timer_t2c(interval_us);
	return HVMM_STATUS_SUCCESS;
}

uint32_t timer_get_interval(timer_channel_t channel)
{
	return _channels[channel].interval_us;
}

hvmm_status_t timer_add_callback(timer_channel_t channel, timer_callback_t callback)
{
	_channels[channel].callback = callback;
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_remove_callback(timer_channel_t channel, timer_callback_t callback)
{
	_channels[channel].callback = 0;
	return HVMM_STATUS_SUCCESS;
}

uint64_t timer_t2c(uint64_t time_us)
{
	return time_us * COUNT_PER_USEC;
}
