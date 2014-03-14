#include <k-hypervisor-config.h>
#include "virqmap.h"
#include "context.h"
#include "hvmm_types.h"
#include <asm-arm_inline.h>

#include <config/cfg_platform.h>
#include <log/print.h>

static struct virqmap_entry _virqmap[GIC_NUM_MAX_IRQS];

/*
 * Creates a mapping table between PIRQ and VIRQ.vmid/pirq/coreid.
 * Mapping of between pirq and virq is hard-coded.
 */
hvmm_status_t virqmap_init(void)
{
    /* TODO(wonseok): read file and initialize the mapping. */
    HVMM_TRACE_ENTER();
    int i;
    for (i = 0; i < GIC_NUM_MAX_IRQS; i++) {
        _virqmap[i].vmid = VMID_INVALID;
        _virqmap[i].virq = 0;
    }
    /*
     * NOTE(wonseok):
     * referenced by
     * https://github.com/kesl/khypervisor/wiki/Hardware-Resources
     * -of-Guest-Linux-on-FastModels-RTSM_VE-Cortex-A15x1
     * */
    CFG_GUEST_VIRQMAP(_virqmap);
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

const struct virqmap_entry *virqmap_for_pirq(uint32_t pirq)
{
    const struct virqmap_entry *result = VIRQMAP_ENTRY_NOTFOUND;

    if (_virqmap[pirq].vmid != VMID_INVALID)
        result = &_virqmap[pirq];

    return result;
}

uint32_t virqmap_pirq(vmid_t vmid, uint32_t virq)
{
    uint32_t pirq = PIRQ_INVALID;
    /*
     * FIXME: This is ridiculously inefficient to
     * loop up to GIC_NUM_MAX_IRQS
     */
    int i;
    for (i = 0; i < GIC_NUM_MAX_IRQS; i++) {
        if (_virqmap[i].vmid == vmid && _virqmap[i].virq == virq) {
            pirq = i;
            break;
        }
    }
    return pirq;
}
