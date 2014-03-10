#ifndef _VIRQMAP_H_
#define _VIRQMAP_H_

#include <arch_types.h>
#include <hvmm_trace.h>
#include <gic.h>

/**
 * @breif   Saves a mapping information to find a virq for injection.
 *
 * We do not consider a sharing device that's why we save only one vmid.
 * @todo    (wonseok): need to change a structure when we support
 *                     a sharing device among guests.
 */
struct virqmap_entry {
    vmid_t vmid;    /**< Guest vm id */
    uint32_t virq;  /**< Virtual interrupt nubmer */
};

const struct virqmap_entry *virqmap_for_pirq(uint32_t pirq);
/**
 * @breif       Initializes "virqmap_entry",mapping information to find a virq for injection,
                and registers callback function called
                when Guest changes GICD_ISENABLER/ICENABLER registers.
 * @return      Always retuns "success".
 */
hvmm_status_t virqmap_init(void);
/**
 * @breif       Returns pirq mapped to virq for vm.
 * @param vmid  Guest vm id
 * @param virq  Virtual interrupt number.
 * @return      physical interrupt number.
 */
uint32_t virqmap_pirq(vmid_t vmid, uint32_t virq);
/**
 * @breif Callback function called when Guest operates its GICD_INSENABLER/ICENABER register.
 * <pre>
 * It is called when Guest vm handles its Set-Enable registers,
 * GICD_ISENABLER or Clear-Enable Registers, GICD_ICENABLER.
 * When Guest vm sets its GICD_ISENABLER bit to 1,
 * this callback enables corresponding pirq.
 * When Guest vm sets its CICD_ICENABLER bit to 1,
 * this callback disables corresponding interrupt
 * from the Distributor to the CPU interface.
 * </pre>
 * @param vmid          Guest vm id.
 * @param istatus       Current ISENABLER status.
 * @param word_offset   The corresponding GICD_ISENABLER number n.
 */
void virqmap_vgicd_changed_istatus_callback_handler(vmid_t vmid,
        uint32_t istatus, uint8_t word_offset);

#define VIRQMAP_ENTRY_NOTFOUND  0

#endif
