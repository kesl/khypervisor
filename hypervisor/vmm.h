/* Virtual Machine Memory Management Module */
#ifndef __VMM_H__
#define __VMM_H__
#include <hvmm_types.h>
#include "lpae.h"

/**
 * @brief Get Target Virtual Machine's Translation Table Pointer By vmid
 *
 * Get Target Virtual machien's TTBL By vmid
 * @param vmid_t vmid wish Virtual machine's ID
 * @return lpaed_t * ttbl
 */
union lpaed *vmm_vmid_ttbl(vmid_t vmid);
/**
 * @brief Change the VTTBR by vmid & ttbl
 *
 * Chagne the Virtual machine's ID & Translation Table to<br>
 * Target Virtual machien's ID & Translation Table<br>
 * <br>
 * - VTTBR.VMID = vmid<br>
 * - VTTBR.BADDR = ttbl
 * @param vmid_t vmid Virtual Machine's ID
 * @param lpaed_t *ttbl ttbl
 * @return hvmm_status_t if Success return HVMM_STATUS_SUCCESS
 */
hvmm_status_t vmm_set_vmid_ttbl(vmid_t vmid, union lpaed *ttbl);
/**
 * @brief Enableing Stage 2 MMU
 *
 * - HCR.VM[0] = enable
 * @param int enable
 * @return void
 */
void vmm_stage2_enable(int enable);
/**
 * @brief Initialization of Virtualization Machine Memory Management
 * Stage 2 Translation
 *
 * Initializes Translation Table for Stage2 Translation(IPA -> PA)<br>
 * - First, Setting Guest's Physical Address to Memory Descriptor<br>
 * - Second, Initialize Translation Table by vmm_init_ttbl()<br>
 * - Third, Initialize MMU by vmm_init_mmu()
 * @param void
 * @return void
 */
void vmm_init(void);

extern uint32_t guest_bin_start;
extern uint32_t guest2_bin_start;

#endif
