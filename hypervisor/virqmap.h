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

#define VIRQMAP_ENTRY_NOTFOUND  0

#endif
