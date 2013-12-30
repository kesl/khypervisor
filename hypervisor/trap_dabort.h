#ifndef __TRAP_DABORT_H__
#define __TRAP_DABORT_H__
#include <hvmm_types.h>

hvmm_status_t trap_hvc_dabort(unsigned int iss, struct arch_regs *regs);

#endif
