#define DEBUG
#include <hvmm_types.h>
#include <guest.h>
#include <hvmm_trace.h>
#include <log/uart_print.h>
#include <interrupt.h>

#define VIRQ_MIN_VALID_PIRQ 16
#define VIRQ_NUM_MAX_PIRQS  MAX_IRQS

#define VALID_PIRQ(pirq) \
    (pirq >= VIRQ_MIN_VALID_PIRQ && pirq < VIRQ_NUM_MAX_PIRQS)


static struct interrupt_ops *_guest_ops;
static struct interrupt_ops *_host_ops;

static struct guest_virqmap *_guest_virqmap;

/**< IRQ handler */
static interrupt_handler_t _host_handlers[MAX_IRQS];

const int32_t interrupt_check_guest_irq(uint32_t pirq)
{
    int i;
    struct virqmap_entry *map;

    for (i = 0; i < NUM_GUESTS_CPU0_STATIC; i++) {
        map = _guest_virqmap[i].map;
        if (map[pirq].virq != VIRQ_INVALID)
            return GUEST_IRQ;
    }

    return HOST_IRQ;
}

const uint32_t interrupt_pirq_to_virq(vmid_t vmid, uint32_t pirq)
{
    struct virqmap_entry *map = _guest_virqmap[vmid].map;

    return map[pirq].virq;
}

const uint32_t interrupt_virq_to_pirq(vmid_t vmid, uint32_t virq)
{
    struct virqmap_entry *map = _guest_virqmap[vmid].map;

    return map[virq].pirq;
}

const uint32_t interrupt_pirq_to_enabled_virq(vmid_t vmid, uint32_t pirq)
{
    uint32_t virq = VIRQ_INVALID;
    struct virqmap_entry *map = _guest_virqmap[vmid].map;

    if (map[pirq].enabled)
        virq = map[pirq].virq;

    return virq;
}

hvmm_status_t interrupt_guest_inject(vmid_t vmid, uint32_t virq, uint32_t pirq,
                uint8_t hw)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    /* guest_interrupt_inject() */
    if (_guest_ops->inject)
        ret = _guest_ops->inject(vmid, virq, pirq, hw);

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

    /* host_interrupt_enable() */
    if (_host_ops->enable)
        ret = _host_ops->enable(irq);

    return ret;
}

hvmm_status_t interrupt_host_disable(uint32_t irq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    /* host_interrupt_disable() */
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

hvmm_status_t interrupt_guest_enable(vmid_t vmid, uint32_t irq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;
    struct virqmap_entry *map = _guest_virqmap[vmid].map;

    map[irq].enabled = GUEST_IRQ_ENABLE;

    return ret;
}

hvmm_status_t interrupt_guest_disable(vmid_t vmid, uint32_t irq)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;
    struct virqmap_entry *map = _guest_virqmap[vmid].map;

    map[irq].enabled = GUEST_IRQ_DISABLE;

    return ret;
}

static void interrupt_inject_enabled_guest(int num_of_guests, uint32_t irq)
{
    int i;
    uint32_t virq;

    for (i = 0; i < num_of_guests; i++) {
        virq = interrupt_pirq_to_enabled_virq(i, irq);
        if (virq == VIRQ_INVALID)
            continue;
        interrupt_guest_inject(i, virq, irq, INJECT_HW);
    }
}

void interrupt_service_routine(int irq, void *current_regs, void *pdata)
{
    struct arch_regs *regs = (struct arch_regs *)current_regs;

    if (irq < MAX_IRQS) {
        if (interrupt_check_guest_irq(irq) == GUEST_IRQ) {
            /* IRQ INJECTION */
            /* priority drop only for hanlding irq in guest */
            /* guest_interrupt_end() */
            _guest_ops->end(irq);
            interrupt_inject_enabled_guest(NUM_GUESTS_CPU0_STATIC, irq);
        } else {
            /* host irq */
            if (_host_handlers[irq])
                _host_handlers[irq](irq, regs, 0);
            /* host_interrupt_end() */
            _host_ops->end(irq);
        }
    } else
        printh("interrupt:no pending irq:%x\n", irq);
}

hvmm_status_t interrupt_save(vmid_t vmid)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    /* guest_interrupt_save() */
    if (_guest_ops->save)
        ret = _guest_ops->save(vmid);

    return ret;
}

hvmm_status_t interrupt_restore(vmid_t vmid)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    /* guest_interrupt_restore() */
    if (_guest_ops->restore)
        ret = _guest_ops->restore(vmid);

    return ret;
}

hvmm_status_t interrupt_init(struct guest_virqmap *virqmap)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    _host_ops = _interrupt_module.host_ops;
    _guest_ops = _interrupt_module.guest_ops;

    _guest_virqmap = virqmap;

    /* host_interrupt_init() */
    if (_host_ops->init) {
        ret = _host_ops->init();
        if (ret)
            printh("host initial failed:'%s'\n", _interrupt_module.name);
    }

    /* guest_interrupt_init() */
    if (_guest_ops->init) {
        ret = _guest_ops->init();
        if (ret)
            printh("guest initial failed:'%s'\n", _interrupt_module.name);
    }

    return ret;
}
