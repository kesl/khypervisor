#include <hyp_config.h>
#include "virqmap.h"
#include "print.h"
#include "context.h"
#include "hvmm_types.h"

static struct virqmap_entry _virqmap[GIC_NUM_MAX_IRQS];

/* 
 * Creates a mapping table between PIRQ and VIRQ.vmid/pirq/coreid.
 * Mapping of between pirq and virq is hard-coded.
 */
hvmm_status_t virqmap_init(void) 
{
    // TODO(wonseok): read file and initialize the mapping.
    HVMM_TRACE_ENTER();
    int i;

    for (i = 0; i < GIC_NUM_MAX_IRQS; i++) {
        _virqmap[i].vmid = VMID_INVALID;
        _virqmap[i].virq = 0;
    }

    // NOTE(wonseok): referenced by https://github.com/kesl/khypervisor/wiki/Hardware-Resources-of-Guest-Linux-on-FastModels-RTSM_VE-Cortex-A15x1

    // WDT: shared driver
    // IRQ 32
    _virqmap[32].virq = 32;
    _virqmap[32].vmid = 0;

    // SP804: shared driver
    // IRQ 34, 35
    _virqmap[34].virq = 34;
    _virqmap[34].vmid = 0;
    _virqmap[35].virq = 35;
    _virqmap[35].vmid = 0;

    // RTC: shared driver
    // IRQ 36
    _virqmap[36].virq = 36;
    _virqmap[36].vmid = 0;

    // UART: dedicated driver 
    // IRQ 37 for guest 0
    _virqmap[38].virq = 37;
    _virqmap[38].vmid = 0;
    // IRQ 38 for guest 1
    _virqmap[39].virq = 37;
    _virqmap[39].vmid = 1;

    // ACCI: shared driver
    // IRQ 43 
    _virqmap[43].virq = 43;
    _virqmap[43].vmid = 0; 

    // KMI: shared driver
    // IRQ 44, 45
    _virqmap[44].virq = 44;
    _virqmap[44].vmid = 0; 
    _virqmap[45].virq = 45;
    _virqmap[45].vmid = 0; 

    HVMM_TRACE_EXIT();

    return HVMM_STATUS_SUCCESS;
}

const struct virqmap_entry *virqmap_for_pirq(uint32_t pirq)
{
    const struct virqmap_entry * result = VIRQMAP_ENTRY_NOTFOUND;

    if ( _virqmap[pirq].vmid != VMID_INVALID) {
        result = &_virqmap[pirq];
    }
    return result;
}

uint32_t virqmap_pirq(vmid_t vmid, uint32_t virq)
{
    uint32_t pirq = PIRQ_INVALID;

    /* FIXME: This is ridiculously inefficient to loop up to GIC_NUM_MAX_IRQS */
    int i;
    for (i = 0; i < GIC_NUM_MAX_IRQS; i++) {
        if ( _virqmap[i].vmid == vmid && _virqmap[i].virq == virq ) {
            pirq = i;
            break;
        }
    }
    return pirq;
}

