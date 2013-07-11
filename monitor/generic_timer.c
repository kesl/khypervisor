#include "timer.h"
#include "generic_timer.h"

hvmm_status_t generic_timer_init(void)
{
	return HVMM_STATUS_SUCCESS;
}
hvmm_status_t generic_timer_start(void)
{
	return HVMM_STATUS_SUCCESS;
}
hvmm_status_t generic_timer_stop(void)
{
	return HVMM_STATUS_SUCCESS;
}
hvmm_status_t generic_timer_add_tick_callback(int irq, gic_irq_handler_t handler)
{
	return HVMM_STATUS_SUCCESS;
}
void generic_timer_get_time(struct timeval* timeval)
{

}

