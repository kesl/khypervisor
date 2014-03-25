/* Virtual Machine Memory Management Module */
#ifndef __VMM_H__
#define __VMM_H__
#include <hvmm_types.h>
#include <guest.h>
#include "lpae.h"

#define MMU_ON 1
#define MMU_OFF 0

/**
 * @brief Obtains the level 1 translation table descriptor for the specified
 * vmid.
 * @return Level 1 translation table descriptor.
 */
union lpaed *vmm_vmid_ttbl(vmid_t vmid);
/**
 * @brief Changes the stage-2 translation table base address by configuring
 *        VTTBR.
 *
 * Configures Virtualization Translation Table Base Register(VTTBR) to change
 * the guest. Change vmid and base address from received vmid and ttbl
 * address.
 *
 * @param vmid Received vmid.
 * @param ttbl Level 1 translation table of the guest.
 * @return HVMM_STATUS_SUCCESS only.
 */
hvmm_status_t vmm_set_vmid_ttbl(vmid_t vmid, union lpaed *ttbl);
/**
 * @brief Initializes the virtual mode(guest mode) memory management
 * stage-2 translation.
 *
 * Configure translation tables of guests for stage-2 translation (IPA -> PA).
 *
 * - First, maps physical address of guest start address to descriptors.
 * - And configure the translation table descriptors based on the memory
 *   map descriptor lists.
 * - Last, initializes mmu.
 *
 * @return void
 */
void vmm_init(void);

extern uint32_t guest_bin_start;
extern uint32_t guest2_bin_start;

/**
 * @brief Stops stage-2 translation by disabling mmu.
 *
 * We assume VTCR has been configured and initialized in the memory
 * management module.
 * - Disables stage-2 translation by HCR.vm = 0.
 */
void vmm_lock(void);
/**
 * @brief Restores translation table for the next guest and enable stage-2 mmu.
 *
 * - Chagne stage-2 translation table and vmid.
 * - Eanbles stage-2 MMU.
 *
 * @param guest Context of the next guest.
 */
void vmm_unlock(struct guest_struct *guest);

#endif
