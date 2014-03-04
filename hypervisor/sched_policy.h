#ifndef __SCHED_POLICY_H__
#define __SCHED_POLICY_H__

#include <hvmm_types.h>

/**
*@param void
*@return vmid
*@brief It is schedule policy : determ next vm
*/
vmid_t sched_policy_determ_next(void);

#endif
