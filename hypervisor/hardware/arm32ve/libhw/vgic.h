#ifndef __VGIC_H__
#define __VGIC_H__
#include <arch_types.h>
#include <hvmm_types.h>

#define VGIC_NUM_MAX_SLOTS              64
#define VGIC_SLOT_NOTFOUND              (0xFFFFFFFF)

enum virq_state {
    VIRQ_STATE_INACTIVE = 0x00,
    VIRQ_STATE_PENDING = 0x01,
    VIRQ_STATE_ACTIVE = 0x02,
    VIRQ_STATE_PENDING_ACTIVE = 0x03,
};

struct vgic_status {
    /* restore only if saved once to avoid dealing with corrupted data */
    uint32_t saved_once;
    uint32_t lr[64];        /**< List Registers */
    uint32_t hcr;           /**< Hypervisor Control Register */
    uint32_t apr;           /**< Active Priorities Register */
    uint32_t vmcr;          /**< Virtual Machine Control Register */
};
/**
 * @brief           Enable/Disable Virtual CPU interface operation.
 *
 * If "enable" is "1", enables Virtual CPU interface operation and
 * maintenance interrupt.
 * If "enable" is "0", disables them.
 * @param enable    "1" enable, "0" disable.
 * @return          If Virtual Interface Control intialized, then returns "success",
 *                  otherwise returns "bad access".
 */
hvmm_status_t vgic_enable(uint8_t enable);
/**
 * @brief   Initializes Virtual Interface Control.
 *
 * Sets GICH's base address, number of list registers, initlizalied flag.
 * Enables Virtual Maintenace interrupt.
 * @return  Always returns "success".
 */
hvmm_status_t vgic_init(void);
/**
 * @brief           Initializes Virtual Interface Control status for each guest.
 * @param status    vgic status. Refer to vgic_status.
 * @param vmid      guest id.
 * @return          Always returns "success".
 */
hvmm_status_t vgic_init_status(struct vgic_status *status, vmid_t vmid);
hvmm_status_t vgic_save_status(struct vgic_status *status, vmid_t vmid);
hvmm_status_t vgic_restore_status(struct vgic_status *status, vmid_t vmid);
hvmm_status_t vgic_flush_virqs(vmid_t vmid);
/* returns slot index if successful, VGIC_SLOT_NOTFOUND otherwise */
uint32_t vgic_inject_virq_sw(uint32_t virq, enum virq_state state,
            uint32_t priority, uint32_t cpuid, uint8_t maintenance);
/* returns slot index if successful, VGIC_SLOT_NOTFOUND otherwise */
uint32_t vgic_inject_virq_hw(uint32_t virq, enum virq_state state,
            uint32_t priority, uint32_t pirq);
uint32_t vgic_inject_virq(uint32_t virq, uint32_t slot, enum virq_state state,
            uint32_t priority, uint8_t hw, uint32_t physrc,
            uint8_t maintenance);
hvmm_status_t vgic_setcallback_virq_flush(void (*callback)(vmid_t vmid));
hvmm_status_t vgic_injection_enable(uint8_t enable);
/**
 * @brief   Initializes all physical irq slot and virtual irq slot.
 */
void vgic_slotpirq_init(void);
void vgic_slotpirq_set(vmid_t vmid, uint32_t slot, uint32_t pirq);
uint32_t vgic_slotpirq_get(vmid_t vmid, uint32_t slot);
void vgic_slotpirq_clear(vmid_t vmid, uint32_t slot);
void vgic_slotvirq_set(vmid_t vmid, uint32_t slot, uint32_t virq);
uint32_t vgic_slotvirq_getslot(vmid_t vmid, uint32_t virq);
void vgic_slotvirq_clear(vmid_t vmid, uint32_t slot);
/**
 * @breif       Returns pirq mapped to virq for vm.
 * @param vmid  Guest vm id
 * @param virq  Virtual interrupt number.
 * @return      physical interrupt number.
 */

hvmm_status_t virq_inject(vmid_t vmid, uint32_t virq,
        uint32_t pirq, uint8_t hw);
/**
 * @brief   Initializes virq_entry structure and
            Sets callback function about injection of queued VIRQs.
 * @return  Always returns "success".
 */
hvmm_status_t virq_init(void);
#endif
