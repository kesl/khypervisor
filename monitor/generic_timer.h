#ifndef __GENERIC_TIMER_H__
#define __GENERIC_TIMER_H__

#include "hvmm_types.h"
#include "arch_types.h"
#include "timer.h"

hvmm_status_t generic_timer_init(void);
hvmm_status_t generic_timer_start(void);
hvmm_status_t generic_timer_stop(void);
hvmm_status_t generic_timer_add_tick_callback(int irq, gic_irq_handler_t handler);
void generic_timer_get_time(struct timeval* timeval);

#endif

