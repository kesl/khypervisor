/* 
 * timer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */
#include "timer.h"

hvmm_status_t timer_init(timer_channel_t timer_channel)
{
	return HVMM_STATUS_SUCCESS;
}
	
hvmm_status_t timer_start(timer_channel_t timer_channel)
{
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_stop(timer_channel_t timer_channel)
{
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_set_interval(timer_channel_t timer_channel, uint32_t interval)
{
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_get_interval(timer_channel_t timer_channel)
{
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_add_callback(timer_channel_t timer_channel, gic_irq_handler_t handler)
{
	return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_remove_callback(timer_channel_t timer_channel, gic_irq_handler_t handler)
{
	return HVMM_STATUS_SUCCESS;
}

void timer_get_time(struct timeval* timeval)
{

}
