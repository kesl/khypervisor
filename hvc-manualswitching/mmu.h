#ifndef __MMU_H__
#define __MMU_H__
#include "arch_types.h"
#include "lpae.h"

/* --- VMM.H --- */
typedef uint8_t vmid_t; 
typedef enum {
	VMM_STATUS_SUCCESS = 0,
	VMM_STATUS_UNKNOWN_ERROR = -1,
} vmm_status_t;


lpaed_t *vmm_vmid_ttbl(vmid_t vmid);
void vmm_stage2_enable(int enable);
vmm_status_t vmm_set_vmid_ttbl( vmid_t vmid, lpaed_t *ttbl );


int mmu_init(void);

#endif
