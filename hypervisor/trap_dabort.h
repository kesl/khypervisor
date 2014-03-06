#ifndef __TRAP_DABORT_H__
#define __TRAP_DABORT_H__
#include <hvmm_types.h>
/**
 * @brief Handles data abort case trapped into hvc, not dabort
 * @param iss register' bit HSR 0 ~ 24 bit
 * @param currnt registers' values
 * @return value of hvmm status
 */
hvmm_status_t trap_hvc_dabort(unsigned int iss, struct arch_regs *regs);

#endif
