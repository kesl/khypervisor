#ifndef _VERSION_H__
#define _VERSION_H__

#include "armv7_p15.h"
#include "context.h"
#include "timer.h"
#include <log/uart_print.h>
#include <hvmm_types.h>

vmid_t sched_policy_determ_next(void);
hvmm_status_t scheduler_next_event(int irq, void *pdata);

void guest_schedule(void);
void guest_switch_to_next_guest(void *pdata);

void start_guest(void);

#endif
