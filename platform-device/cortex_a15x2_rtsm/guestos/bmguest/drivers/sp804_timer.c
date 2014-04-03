#include "gic.h"
#include "hvmm_trace.h"
#include "armv7_p15.h"
#include "sp804_timer.h"
#include <log/uart_print.h>
#include <log/print.h>

#define SP804_BASE 0x1C110000

/* Register offsets */
#define SP804_LOAD            0x0
#define SP804_VALUE            0x4
#define SP804_CTRL            0x8
#define SP804_INTCLR            0xC
#define SP804_RIS            0x10
#define SP804_MIS            0x14
#define SP804_BGLOAD            0x18

#define SP804_ENABLE            (1 << 7)
#define SP804_PERIODIC            (1 << 6)
#define SP804_IRQEN            (1 << 5)
#define SP804_32BIT            (1 << 1)
#define SP804_ONESHOT            (1 << 0)

#define SP804_SECONDARY_OFFSET        0x20

/* Timer prescaling */
#define SP804_SCALE_SHIFT        2
#define SP804_SCALE_DIV16        1
#define SP804_SCALE_DIV256        2

/* Wrapping = 0, Oneshot = 1 */
#define SP804_ONESHOT            (1 << 0)

#define writec(v, a)    (*(volatile unsigned char *)(a) = (v))
#define readc(a)         (*(volatile unsigned char *)(a))
#define writes(v, a)    (*(volatile unsigned short *)(a) = (v))
#define reads(a)         (*(volatile unsigned short *)(a))
#define writel(v, a)    (*(volatile uint32_t *)(a) = (v))
#define readl(a)         (*(volatile uint32_t *)(a))

/* Load the timer with ticks value */
void sp804_load(uint32_t loadval, uint32_t sp804_base)
{
    writel(loadval, sp804_base + SP804_LOAD);
}

void sp804_init_periodic(uint32_t sp804_base, uint32_t load_value)
{
    volatile uint32_t reg;
    /* Periodic, wraparound, 32 bit, irq on wraparound */
    reg = SP804_PERIODIC | SP804_32BIT | SP804_IRQEN;
    writel(reg, sp804_base + SP804_CTRL);
    /* 1 tick per usec, 1 irq per msec */
    if (load_value)
        sp804_load(load_value, sp804_base);
    else
        sp804_load(1000, sp804_base);
}

void sp804_init_oneshot(uint32_t sp804_base)
{
    volatile uint32_t reg = readl(sp804_base + SP804_CTRL);
    /* One shot, 32 bits, no irqs */
    reg |= SP804_32BIT | SP804_ONESHOT;
    writel(reg, sp804_base + SP804_CTRL);
}
void sp804_init(uint32_t sp804_base, uint32_t load_value)
{
    sp804_init_periodic(sp804_base, load_value);
    sp804_start(sp804_base);
}

/* Enable timer with its current configuration */
void sp804_stop(uint32_t sp804_base)
{
    writel(0, sp804_base + SP804_CTRL);
}

void sp804_start(uint32_t sp804_base)
{
    volatile uint32_t reg = readl(sp804_base + SP804_CTRL);
    reg |= SP804_ENABLE;
    writel(reg, sp804_base + SP804_CTRL);
}

uint32_t sp804_read(uint32_t sp804_base)
{
    return readl(sp804_base + SP804_VALUE);
}

void sp804_irq_clear(uint32_t sp804_base)
{
    writel(1, sp804_base + SP804_INTCLR);
}

void interrupt_sp804_timer(int irq, void *pregs, void *pdata)
{
    uint32_t ctl;
    uint32_t val;
    vmid_t vmid;
    printH("=======================================\n\r");
    HVMM_TRACE_ENTER();
    val = sp804_read(SP804_BASE);
    printH("sp804:%x\n\r",val);
    /* irq clear */
    sp804_irq_clear(SP804_BASE);
    HVMM_TRACE_EXIT();
    printH("=======================================\n\r");
}

hvmm_status_t hvmm_tests_sp804_timer(void)
{
    HVMM_TRACE_ENTER();
    /* start timer */
    sp804_init(SP804_BASE, 100000);
    gic_enable_irq(34);
    gic_set_irq_handler(34, interrupt_sp804_timer, 0);
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
