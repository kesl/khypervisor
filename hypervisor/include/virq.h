#ifndef __VIRQ_H__
#define __VIRQ_H__
#include <hvmm_types.h>

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


#define VIRQMAP_ENTRY_NOTFOUND  0

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

hvmm_status_t virq_inject(vmid_t vmid, uint32_t virq,
        uint32_t pirq, uint8_t hw);
/**
 * @brief   Initializes virq_entry structure and
            Sets callback function about injection of queued VIRQs.
 * @return  Always returns "success".
 */
hvmm_status_t virq_init(void);
#endif
