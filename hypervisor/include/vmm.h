/* Virtual Machine Memory Management Module */
#ifndef __VMM_H__
#define __VMM_H__
#include <hvmm_types.h>
#include <guest.h>
#include "lpae.h"

/**
 * @brief Obtains the level 1 translation table descriptor for the specified vmid.
 *
 * @param vmid Wish vmid.
 * @return Obtained level 1 translation table descriptor.
 */
union lpaed *vmm_vmid_ttbl(vmid_t vmid);
/**
 * @brief Change the stage-2 translation table bass address by configuring
 *        vttbr.
 *
 * Configures Virtualization translation table base register(VTTBR) to change
 * the guest. Modify the vmid and base address by delivered vmid and the ttbl
 * address.
 *
 * @param vmid Target guest id.
 * @param ttbl Level 1 translation table of the guest.
 * @return HVMM_STATUS_SUCCESS
 */
hvmm_status_t vmm_set_vmid_ttbl(vmid_t vmid, union lpaed *ttbl);
/**
 * @brief Enable or disable stage-2 translation(MMU).
 *
 * Configures Hyper Configuration Register(HCR) to enable or disable the
 * virtualization MMU.
 *
 * @param enable Enable or disable the MMU.
 *        - 1 Enable the MMU.
 *        - 0 Disable the MMU.
 * @return void
 */
void vmm_stage2_enable(int enable);
/**
 * @brief Initialization of the virtual mode(guest mode) memory management
 * stage-2 translation.
 *
 * configure translation tables of guests for stage-2
 * translation (IPA -> PA).
 *
 * - First, mapping physical Address of start address of guest to descriptor.
 * - And configure the translation table descriptors based on the memory
 *   mapping descriptor list.
 * - Last, initialize mmu.
 *
 * @return void
 */
void vmm_init(void);

extern uint32_t guest_bin_start;
extern uint32_t guest2_bin_start;

/**
 * @brief Stop stage-2 translation by disabling mmu.
 *
 * We assume VTCR has been configured and initialized in the memory
 * management module.
 * - Disable stage-2 translation by HCR.vm = 0.
 */
void vmm_lock(void);
/**
 * @brief Restores translation table for the next guest and enable stage-2
 * translation.
 *
 * - Chagne stage-2 translation table and vmid.
 * - Eanble stage-2 MMU.
 */
void vmm_unlock(struct guest_struct *guest);

#endif
