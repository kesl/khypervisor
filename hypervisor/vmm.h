/* Virtual Machine Memory Management Module */
#ifndef __VMM_H__
#define __VMM_H__
#include <hvmm_types.h>
#include "lpae.h"

union lpaed *vmm_vmid_ttbl(vmid_t vmid);
hvmm_status_t vmm_set_vmid_ttbl(vmid_t vmid, union lpaed *ttbl);
void vmm_stage2_enable(int enable);
void vmm_init(void);

extern uint32_t guest_bin_start;
extern uint32_t guest2_bin_start;

#endif
