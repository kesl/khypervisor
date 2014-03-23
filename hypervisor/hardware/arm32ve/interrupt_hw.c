#include <interrupt.h>
#include <gic.h>
#include <vgic.h>
#include <log/print.h>
#include <log/uart_print.h>

static hvmm_status_t host_interrupt_init(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    /* Route IRQ/IFQ to Hyp Exception Vector */
    {
        uint32_t hcr;
        hcr = read_hcr();
        uart_print("hcr:");
        uart_print_hex32(hcr);
        uart_print("\n\r");
        hcr |= HCR_IMO | HCR_FMO;
        write_hcr(hcr);
        hcr = read_hcr();
        uart_print("hcr:");
        uart_print_hex32(hcr);
        uart_print("\n\r");
    }

    /* Physical Interrupt: GIC Distributor & CPU Interface */
    result = gic_init();

    return result;
}

static hvmm_status_t host_interrupt_enable(uint32_t irq)
{
    return gic_enable_irq(irq);
}

static hvmm_status_t host_interrupt_disable(uint32_t irq)
{
    return gic_disable_irq(irq);
}

static hvmm_status_t host_interrupt_configure(uint32_t irq)
{
    return gic_configure_irq(irq, GIC_INT_POLARITY_LEVEL,
            gic_cpumask_current(), GIC_INT_PRIORITY_DEFAULT);
}

static hvmm_status_t host_interrupt_end(uint32_t irq)
{
    /* Completion & Deactivation */
    gic_completion_irq(irq);
    gic_deactivate_irq(irq);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t host_interrupt_save(void)
{
    /* TODO : save interrupt infomation */
    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t host_interrupt_restore(void)
{
    /* TODO : restore interrupt infomation */
    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t host_interrupt_dump(void)
{
    /* TODO : dumpping the interrupt status & count */
    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_interrupt_init(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    /* Virtual Interrupt: GIC Virtual Interface Control */
    result = vgic_init();
    if (result == HVMM_STATUS_SUCCESS)
        result = vgic_enable(1);

    virq_init();

    return result;
}

static hvmm_status_t guest_interrupt_enable(uint32_t irq)
{
    /* TODO : enable injection */
    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_interrupt_disable(uint32_t irq)
{
    /* TODO : disable injection */
    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_interrupt_end(uint32_t irq)
{
    return gic_completion_irq(irq);
}

static hvmm_status_t guest_interrupt_inject(vmid_t vmid, uint32_t virq,
                        uint32_t pirq)
{
    /* TODO : checking the injected bitmap */
    return virq_inject(vmid, virq, pirq, 1);
}

static hvmm_status_t guest_interrupt_save(void)
{
    /* TODO : save guest interrupt infomation */
    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_interrupt_restore(void)
{
    /* TODO : restore guest interrupt infomation */
    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_interrupt_dump(void)
{
    /* TODO : dumpping the injected bitmap */
    return HVMM_STATUS_SUCCESS;
}

struct interrupt_ops _host_interrupt_ops = {
    .init = host_interrupt_init,
    .enable = host_interrupt_enable,
    .disable = host_interrupt_disable,
    .configure = host_interrupt_configure,
    .end = host_interrupt_end,
    .save = host_interrupt_save,
    .restore = host_interrupt_restore,
    .dump = host_interrupt_dump,
};

struct interrupt_ops _guest_interrupt_ops = {
    .init = guest_interrupt_init,
    .enable = guest_interrupt_enable,
    .disable = guest_interrupt_disable,
    .end = guest_interrupt_end,
    .inject = guest_interrupt_inject,
    .save = guest_interrupt_save,
    .restore = guest_interrupt_restore,
    .dump = guest_interrupt_dump,
};

struct interrupt_module _interrupt_module = {
    .name = "K-Hypervisor Interrupt Module",
    .author = "Kookmin Univ.",
    .host_ops = &_host_interrupt_ops,
    .guest_ops = &_guest_interrupt_ops,
};
