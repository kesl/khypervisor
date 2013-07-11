#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "hvmm_types.h"
#include "timer.h"

/*
*		
*/
hvmm_status_t scheduler_init(void);
/*
*	call back fucntion. Called when timer interrupt occur
*
*	e.g)
*	timer_add_tick_callback(30, &scheduler_next_event);
*	_gic_handlers[irq](irq, regs);
*/
hvmm_status_t scheduler_next_event(int irq, void *pdata);

#endif
