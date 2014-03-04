#ifndef __TRAP_DABORT_H__
#define __TRAP_DABORT_H__
#include <hvmm_types.h>
/**
 * @brief Handles data abort case trapped into hvc, not dabort
 * @param iss HSR 0 ~ 24 bit
 * @param struct arch_regs *regs currnt registers' values
 * @return hvmm_status_t value of hvmm status
 */
hvmm_status_t trap_hvc_dabort(unsigned int iss, struct arch_regs *regs);

#endif
