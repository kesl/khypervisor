#ifndef _VIRQMAP_H_
#define _VIRQMAP_H_

#include <arch_types.h>
#include <hvmm_trace.h>
#include <gic.h>

/*
 * Saves a mapping information to find a virq for injection.
 * We do not consider a sharing device that's why we save only one vmid.
 *
 * TODO(wonseok): need to change a structure when we support
 * a sharing device among guests.
 */
struct virqmap_entry {
    vmid_t vmid;
    uint32_t virq;
};

const struct virqmap_entry *virqmap_for_pirq(uint32_t pirq);
hvmm_status_t virqmap_init(void);
uint32_t virqmap_pirq(vmid_t vmid, uint32_t virq);
void virqmap_vgicd_changed_istatus_callback_handler(vmid_t vmid,
        uint32_t istatus, uint8_t word_offset);

#define VIRQMAP_ENTRY_NOTFOUND  0

#endif
