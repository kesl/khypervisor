#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "armv7_p15.h"
#include "context.h"
#include "timer.h"

#include <log/uart_print.h>

/**
*@param int irq, void *pdata
*@todo Have to implement
*/
hvmm_status_t scheduler_next_event(int irq, void *pdata);

/**
*@param void
*@return void
*@brief Test scheduling
*/
void scheduler_test_scheduling(void);

/**
*@param void
*@return void
*@brief Request switch when exit trap
*/
void scheduler_schedule(void);

/**
*@param void
*@return void
*@brief Test context switch to next guest
*/
void scheduler_test_switch_to_next_guest(void *pdata);

#endif
