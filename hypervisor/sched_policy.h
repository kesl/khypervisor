#ifndef __SCHED_POLICY_H__
#define __SCHED_POLICY_H__

#include <hvmm_types.h>

/**
*@brief It is schedule policy : determ next vm
*@param void
*@return vmid
*/
vmid_t sched_policy_determ_next(void);

#endif
