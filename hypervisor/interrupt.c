#define DEBUG
#include "hvmm_types.h"
#include "gic.h"
#include "armv7_p15.h"
#include "guest.h"
#include "hvmm_trace.h"
#include "vgic.h"
#include <log/uart_print.h>
#include <interrupt.h>

#define VIRQ_MIN_VALID_PIRQ 16
#define VIRQ_NUM_MAX_PIRQS  MAX_IRQS

#define VALID_PIRQ(pirq) \
    (pirq >= VIRQ_MIN_VALID_PIRQ && pirq < VIRQ_NUM_MAX_PIRQS)

static struct virqmap_entry *_virqmap;
static struct interrupt_ops *_guest_ops;
static struct interrupt_ops *_host_ops;

/**< IRQ handler */
static interrupt_handler_t _host_handlers[MAX_IRQS];

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
    for (i = 0; i < MAX_IRQS; i++) {
        if (_virqmap[i].vmid == vmid && _virqmap[i].virq == virq) {
            pirq = i;
            break;
        }
    }
    return pirq;
}

hvmm_status_t interrupt_guest_inject(vmid_t vmid, uint32_t virq, uint32_t pirq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_guest_ops->inject)
        ret = _guest_ops->inject(vmid, virq, pirq);

    return ret;
}

hvmm_status_t interrupt_request(uint32_t irq, interrupt_handler_t handler)
{
    _host_handlers[irq] = handler;

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t interrupt_host_enable(uint32_t irq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_host_ops->enable)
        ret = _host_ops->enable(irq);

    return ret;
}

hvmm_status_t interrupt_host_disable(uint32_t irq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_host_ops->disable)
        ret = _host_ops->disable(irq);

    return ret;
}

hvmm_status_t interrupt_host_configure(uint32_t irq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_host_ops->configure)
        ret = _host_ops->configure(irq);

    return ret;
}

hvmm_status_t interrupt_guest_enable(uint32_t irq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_guest_ops->enable)
        ret =  _guest_ops->enable(irq);

    return ret;
}

hvmm_status_t interrupt_guest_disable(uint32_t irq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_guest_ops->disable)
        ret = _guest_ops->disable(irq);

    return ret;
}

void interrupt_service_routine(int irq, void *current_regs, void *pdata)
{
    struct arch_regs *regs = (struct arch_regs *)current_regs;
    const struct virqmap_entry *virq_entry;

    if (irq < MAX_IRQS) {
        virq_entry = virqmap_for_pirq(irq);
        if (virq_entry != VIRQMAP_ENTRY_NOTFOUND) {
            /* IRQ INJECTION */
            /* priority drop only for hanlding irq in guest */
            _guest_ops->end(irq);
            _guest_ops->inject(virq_entry->vmid, virq_entry->virq, irq);
        } else {
            /* host irq */
            if (_host_handlers[irq])
                _host_handlers[irq](irq, regs, 0);
            _host_ops->end(irq);
        }
    } else
        printh("interrupt:no pending irq:%x\n", irq);
}

hvmm_status_t interrupt_save(void)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_host_ops->save)
        ret =  _host_ops->save();

    if (_guest_ops->save && !ret)
        ret = _guest_ops->save();

    return ret;
}

hvmm_status_t interrupt_restore(void)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    if (_host_ops->restore)
        ret = _host_ops->restore();

    if (_guest_ops->restore && !ret)
        ret = _guest_ops->restore();

    return ret;
}

hvmm_status_t interrupt_init(struct virqmap_entry *virqmap)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;
    _host_ops = _interrupt_module.host_ops;
    _guest_ops = _interrupt_module.guest_ops;

    _virqmap = virqmap;

    if (_host_ops->init) {
        ret = _host_ops->init();
        if (ret)
            printh("host initial failed:'%s'\n", _interrupt_module.name);
    }

    if (_guest_ops->init) {
        ret = _guest_ops->init();
        if (ret)
            printh("guest initial failed:'%s'\n", _interrupt_module.name);
    }

    return ret;
}
