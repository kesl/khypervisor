#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "armv7_p15.h"
#include "context.h"
#include <vtimer.h>

#include <log/uart_print.h>

hvmm_status_t scheduler_next_event(int irq, void *pdata);

/* Test Code */
void scheduler_test_scheduling(void);

/*
 * Schedules guest context switch according to the default
 * scheduling policy (sched_policy.c)
 */
void scheduler_schedule(void);
void scheduler_test_switch_to_next_guest(void *pdata);

#endif
