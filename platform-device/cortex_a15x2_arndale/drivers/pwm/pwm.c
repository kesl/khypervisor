#include <pwm.h>
#include "pwm_priv.h"
#include "gic.h"
#include "hvmm_trace.h"
#include <io-exynos.h>
uint32_t tcntb1;
static pwm_timer_callback_t _callback;


hvmm_status_t pwm_timer_enable_int()
{
    uint32_t tcon;
    uint32_t tcstat;
    HVMM_TRACE_ENTER();
    tcon = vmm_readl(TCON);
    tcstat = vmm_readl(CSTAT);
    /* auto reload set & timer start */
    tcon |= (TCON_T1RELOAD);
    tcon |= (TCON_T1START);
    vmm_writel(tcon, TCON);
    /* interrupt enable */
    tcstat |= 1 << 1;
    vmm_writel(tcstat, CSTAT);
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
hvmm_status_t pwm_timer_disable_int()
{
    uint32_t tcstat;
    uint32_t tcon;
    /* Stop pwm timer1 */
    tcon = vmm_readl(TCON);
    tcon &= ~(1 << 8);
    vmm_writel(tcon, TCON);
    /* Disable Pwm timer1 Interrupt */
    tcstat = vmm_readl(CSTAT);
    tcstat &= ~(1 << 1);
    vmm_writel(tcstat, CSTAT);
    return HVMM_STATUS_SUCCESS;
}
hvmm_status_t pwm_timer_set_interval(uint32_t interval)
{
    tcntb1 = 16500;
    return HVMM_STATUS_SUCCESS;
}
static void _pwm_timer_irq_handler(int irq, void *regs, void *pdata)
{
    _callback(regs);
}
hvmm_status_t pwm_timer_enable_irq()
{
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    /* handler */
    gic_test_set_irq_handler(69, &_pwm_timer_irq_handler, 0);
    /* configure and enable interrupt */
    gic_test_configure_irq(69,
                           GIC_INT_POLARITY_LEVEL,
                           gic_cpumask_current(),
                           GIC_INT_PRIORITY_DEFAULT);
    result = HVMM_STATUS_SUCCESS;
    return result;
}
hvmm_status_t pwm_timer_set_callback(pwm_timer_callback_t callback)
{
    _callback = callback;
    return HVMM_STATUS_SUCCESS;
}

void pwm_timer_init()
{
    uint32_t tcfg0;
    uint32_t tcfg1;
    uint32_t tcon;
    pwm_timer_set_interval(0);
    pwm_timer_enable_irq();
    tcfg0 = vmm_readl(TCFG0);
    tcfg1 = vmm_readl(TCFG1);
    tcon = vmm_readl(TCON);
    /* prescaler */
    tcfg0 &= ~(0xff);
    tcfg0 |= 250 - 1;
    vmm_writel(tcfg0, TCFG0);
    /* mux */
    tcfg1 &= ~(0xf << 4);
    tcfg1 |= (0x3 << 4);
    vmm_writel(tcfg1, TCFG1);
    /* count buffer resister */
    vmm_writel(tcntb1, TCNTB(1));
    /* count buffer resister load */
    tcon |= TCON_T1MANUALUPD;
    vmm_writel(tcon, TCON);
    tcon &= ~(TCON_T1MANUALUPD);
    vmm_writel(tcon, TCON);
}
