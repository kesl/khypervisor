#ifndef __MM_H__
#define __MM_H__

#include <hvmm_types.h>
#include "arch_types.h"
#include "lpae.h"

lpaed_t *hvmm_mm_vmid_ttbl(vmid_t vmid);
void hvmm_mm_stage2_enable(int enable);
hvmm_status_t hvmm_mm_set_vmid_ttbl( vmid_t vmid, lpaed_t *ttbl );


int hvmm_mm_init(void);

#endif
