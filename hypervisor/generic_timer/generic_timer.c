#include "generic_timer.h"
#include <hvmm_trace.h>

#include <config/cfg_platform.h>

static void _generic_timer_hyp_irq_handler(int irq, void *regs, void *pdata);

static uint32_t _timer_irqs[GENERIC_TIMER_NUM_TYPES];
static uint32_t _tvals[GENERIC_TIMER_NUM_TYPES];
static timer_callback_t _callback[GENERIC_TIMER_NUM_TYPES];
static enum generic_timer_type _timer_type = GENERIC_TIMER_HYP;

static inline void generic_timer_reg_write(int reg, uint32_t val)
{
    switch (reg) {
    case GENERIC_TIMER_REG_FREQ:
        write_cntfrq(val);
        break;
    case GENERIC_TIMER_REG_HCTL:
        write_cnthctl(val);
        break;
    case GENERIC_TIMER_REG_KCTL:
        write_cntkctl(val);
        break;
    case GENERIC_TIMER_REG_HYP_CTRL:
        write_cnthp_ctl(val);
        break;
    case GENERIC_TIMER_REG_HYP_TVAL:
        write_cnthp_tval(val);
        break;
    case GENERIC_TIMER_REG_PHYS_CTRL:
        write_cntp_ctl(val);
        break;
    case GENERIC_TIMER_REG_PHYS_TVAL:
        write_cntp_tval(val);
        break;
    case GENERIC_TIMER_REG_VIRT_CTRL:
        write_cntv_ctl(val);
        break;
    case GENERIC_TIMER_REG_VIRT_TVAL:
        write_cntv_tval(val);
        break;
    default:
        uart_print("Trying to write invalid generic-timer register\n\r");
        break;
    }
    isb();
}

static inline uint32_t generic_timer_reg_read(int reg)
{
    uint32_t val;
    switch (reg) {
    case GENERIC_TIMER_REG_FREQ:
        val = read_cntfrq();
        break;
    case GENERIC_TIMER_REG_HCTL:
        val = read_cnthctl();
        break;
    case GENERIC_TIMER_REG_KCTL:
        val = read_cntkctl();
        break;
    case GENERIC_TIMER_REG_HYP_CTRL:
        val = read_cnthp_ctl();
        break;
    case GENERIC_TIMER_REG_HYP_TVAL:
        val = read_cnthp_tval();
        break;
    case GENERIC_TIMER_REG_PHYS_CTRL:
        val = read_cntp_ctl();
        break;
    case GENERIC_TIMER_REG_PHYS_TVAL:
        val = read_cntp_tval();
        break;
    case GENERIC_TIMER_REG_VIRT_CTRL:
        val = read_cntv_ctl();
        break;
    case GENERIC_TIMER_REG_VIRT_TVAL:
        val = read_cntv_tval();
        break;
    default:
        uart_print("Trying to read invalid generic-timer register\n\r");
        break;
    }
    return val;
}

static inline void generic_timer_reg_write64(int reg, uint64_t val)
{
    switch (reg) {
    case GENERIC_TIMER_REG_HYP_CVAL:
        write_cnthp_cval(val);
        break;
    case GENERIC_TIMER_REG_PHYS_CVAL:
        write_cntp_cval(val);
        break;
    case GENERIC_TIMER_REG_VIRT_CVAL:
        write_cntv_cval(val);
        break;
    case GENERIC_TIMER_REG_VIRT_OFF:
        write_cntvoff(val);
        break;
    default:
        uart_print("Trying to write invalid generic-timer register\n\r");
        break;
    }
    isb();
}

static inline uint64_t generic_timer_reg_read64(int reg)
{
    uint64_t val;
    switch (reg) {
    case GENERIC_TIMER_REG_HYP_CVAL:
        val = read_cnthp_cval();
        break;
    case GENERIC_TIMER_REG_PHYS_CVAL:
        val = read_cntp_tval();
        break;
    case GENERIC_TIMER_REG_VIRT_CVAL:
        val = read_cntv_cval();
        break;
    case GENERIC_TIMER_REG_VIRT_OFF:
        val = read_cntvoff();
        break;
    default:
        uart_print("Trying to read invalid generic-timer register\n\r");
        break;
    }
    return val;
}

/**
 * @brief Each interrup source is identified by a unique ID.
 *
 * DEVICE : IRQ number
 * GENERIC_TIMER_HYP : 26
 * GENERIC_TIMER_NSP : 30
 * GENERIC_TIMER_VIR : 27
 */
hvmm_status_t generic_timer_init()
{
    _timer_irqs[GENERIC_TIMER_HYP] = 26;
    _timer_irqs[GENERIC_TIMER_NSP] = 30;
    _timer_irqs[GENERIC_TIMER_VIR] = 27;

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t generic_timer_set_tval(uint32_t tval)
{
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

    if (_timer_type == GENERIC_TIMER_HYP) {
        _tvals[_timer_type] = tval;
        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_TVAL, tval);
        result = HVMM_STATUS_SUCCESS;
    }

    return result;
}

hvmm_status_t generic_timer_enable_int(void)
{
    uint32_t ctrl;
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

    if (_timer_type == GENERIC_TIMER_HYP) {
        ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_HYP_CTRL);
        ctrl |= GENERIC_TIMER_CTRL_ENABLE;
        ctrl &= ~GENERIC_TIMER_CTRL_IMASK;
        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_CTRL, ctrl);
        result = HVMM_STATUS_SUCCESS;
    }

    return result;
}

hvmm_status_t generic_timer_disable_int(void)
{
    uint32_t ctrl;
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

    if (_timer_type == GENERIC_TIMER_HYP) {
        ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_HYP_CTRL);
        ctrl &= ~GENERIC_TIMER_CTRL_ENABLE;
        ctrl |= GENERIC_TIMER_CTRL_IMASK;
        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_CTRL, ctrl);
        result = HVMM_STATUS_SUCCESS;
    }

    return result;
}

static void _generic_timer_hyp_irq_handler(int irq, void *regs, void *pdata)
{
    _callback[GENERIC_TIMER_HYP](regs);
}

hvmm_status_t generic_timer_enable_irq(void)
{
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

    if (_timer_type == GENERIC_TIMER_HYP) {
        uint32_t irq = _timer_irqs[_timer_type];
        gic_set_irq_handler(irq, &_generic_timer_hyp_irq_handler);
        gic_configure_irq(irq,
                               GIC_INT_POLARITY_LEVEL,
                               gic_cpumask_current(),
                               GIC_INT_PRIORITY_DEFAULT);
        result = HVMM_STATUS_SUCCESS;
    }

    return result;
}

hvmm_status_t generic_timer_set_callback(timer_callback_t callback,
                void *user)
{
    HVMM_TRACE_ENTER();
    _callback[_timer_type] = callback;
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t generic_timer_dump(void)
{
    HVMM_TRACE_ENTER();
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

struct timer_ops generic_timer_ops = {
    .init = generic_timer_init,
    .request_irq = generic_timer_enable_irq,
    .free_irq = (void *)0,
    .enable = generic_timer_enable_int,
    .disable = generic_timer_disable_int,
    .set_interval = generic_timer_set_tval,
    .set_callbacks = generic_timer_set_callback,
    .dump = generic_timer_dump,
};

struct timer_module _timer_module = {
    .name = "K-Hypervisor Timer Module",
    .author = "Kookmin Univ.",
    .ops = &generic_timer_ops,
};
