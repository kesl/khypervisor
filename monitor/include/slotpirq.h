#ifndef __SLOTPIRQ_H__
#define __SLOTPIRQ_H__
#include <arch_types.h>
#include <hvmm_types.h>

#define PIRQ_INVALID    0xFFFFFFFF
void slotpirq_init(void);
void slotpirq_set( vmid_t vmid, uint32_t slot, uint32_t pirq );
uint32_t slotpirq_get(vmid_t vmid, uint32_t slot);
void slotpirq_clear(vmid_t vmid, uint32_t slot);
#endif
