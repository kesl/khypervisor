#include <hyp_config.h>
#include "virqmap.h"
#include "print.h"
#include "context.h"
#include "hvmm_types.h"
#include "vdev/vdev_gicd.h"
#include <asm-arm_inline.h>

/* return the bit position of the first bit set from msb
 * for example, firstbit32(0x7F = 111 1111) returns 7
 */
#define firstbit32(word) ( 31 - asm_clz(word) )
#define NUM_MAX_VIRQS   128
#define NUM_STATUS_WORDS    (NUM_MAX_VIRQS / 32)

static struct virqmap_entry _virqmap[GIC_NUM_MAX_IRQS];
static uint32_t old_vgicd_status[NUM_STATUS_WORDS] = {0, };        // old status

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

    vgicd_set_callback_changed_istatus(&virqmap_vgicd_changed_istatus_callback_handler);

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


// vgicd_changed_istatus callback in handler_ISCENABLER
void virqmap_vgicd_changed_istatus_callback_handler( vmid_t vmid, uint32_t istatus, uint8_t word_offset )
{
    uint32_t cstatus;                          // changed bits only
    uint32_t minirq;
    int bit;

    minirq = word_offset * 32;                 /* irq range: 0~31 + word_offset * size_of_istatus_in_bits */
    cstatus = old_vgicd_status[word_offset] ^ istatus;   // find changed bits

    while(cstatus) {
        uint32_t virq;
        uint32_t pirq;
        bit = firstbit32(cstatus);

        virq = minirq + bit;
        pirq = virqmap_pirq(vmid, virq);

        if ( pirq != PIRQ_INVALID ) {
            /* changed bit */
            if ( istatus & (1 << bit) ) {
                printh("[%s : %d] enabled irq num is %d\n", __FUNCTION__, __LINE__, bit + minirq);
                gic_enable_irq(pirq);
            } else {
                printh("[%s : %d] disabled irq num is %d\n",__FUNCTION__, __LINE__, bit + minirq);
                gic_disable_irq(pirq);
            }
        } else {
            printh( "WARNING: Ignoring virq %d for guest %d has no mapped pirq\n", virq, vmid );
        }

        cstatus &= ~(1<< bit);
    }

    old_vgicd_status[word_offset] = istatus;
}
