/* 
 * timer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */
#include "timer.h"
#include "generic_timer.h"

//TODO modify list
//struct timer_source timer_sources;
static struct timer_channel _channels[TIMER_NUM_MAX_CHANNELS];

/*
 * Called by the hardware timer every 'interval' period
 * Does the hardware timer specific interrupt disable/enable (if necessary)
 */
static void _timer_hw_callback(void *pdata)
{
	/* Generic Timer: disable interrupt */
	generic_timer_disable_int(GENERIC_TIMER_HYP);

	/* Callback */
	/* TODO:
	 * Invoke callbacks in the channel table selectively and conditionally
	 * i.e. depending on channel's interval value
	 */
	_channels[timer_sched].callback(pdata);

    generic_timer_set_tval(GENERIC_TIMER_HYP, _channels[timer_sched].interval);
	
	/* Generic Timer: enable interrupt */
	generic_timer_enable_int(GENERIC_TIMER_HYP);
}

hvmm_status_t timer_register(struct timer_source* ts)
{
/*	
	timer_sources.name = ts->name;
	timer_sources.start = ts->start;
	timer_sources.stop = ts->stop;
	timer_sources.add_callback = ts->add_callback;
	timer_sources.remove_callback = ts->remove_callback;
	timer_sources.get_time = ts->get_time;
*/
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_init(timer_channel_t channel)
{	
	// TODO need such add generic_timer element. before call this function.
	generic_timer_init(&_channels[channel].ts);
	return HVMM_STATUS_SUCCESS;
}
	
hvmm_status_t timer_start(timer_channel_t channel)
{
/*
	//timer_sources.start(timer_sources.interval);
	channel[channel].ts.start(channel[channel].interval);
*/

	if ( _channels[channel].callback != 0 ) {
    	/* Generic Timer: pass _timer_hw_callback */
    	generic_timer_set_callback(GENERIC_TIMER_HYP, &_timer_hw_callback );
    
    	/* Generic Timer: set interval in 'count' */
    	/* interval_ms -> count conversion */
    	generic_timer_set_tval(GENERIC_TIMER_HYP, _channels[channel].interval);
    
		/* TODO: Firgure out how to avoid making redundant calls more than once */
    	generic_timer_enable_irq(GENERIC_TIMER_HYP);

    	/* Generic Timer: enable interupt */
    	generic_timer_enable_int(GENERIC_TIMER_HYP);
	}

	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_stop(timer_channel_t channel)
{
/*
	channel[channel].ts.stop();
*/

	/* Generic Timer: disable interupt */
	generic_timer_disable_int(GENERIC_TIMER_HYP);
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_set_interval(timer_channel_t channel, uint32_t interval)
{
	_channels[channel].interval = interval;
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_get_interval(timer_channel_t channel, uint32_t *interval)
{
	*interval = _channels[channel].interval;
	return HVMM_STATUS_SUCCESS;
}


hvmm_status_t timer_add_callback(timer_channel_t channel, timer_callback_t callback)
{
/*
	_channels[channel].ts.add_callback(channel, callback);
*/

	/* TODO:
	 * Support adding multiple callbacks for a given channel
	 */
	_channels[channel].callback = callback;

	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_remove_callback(timer_channel_t channel, timer_callback_t callback)
{
/*
	channel[channel].ts.remove_callback(0, 0);
*/

	/* TODO:
	 * Implement removing THE callback from the list of the channel
	 */
	_channels[channel].callback = 0;

	return HVMM_STATUS_SUCCESS;
}

void timer_get_time(struct timeval* timeval)
{
/*
	_channels[0].ts.get_time(timeval);
*/
}

