#include <generic_timer.h>
#include <hvmm_trace.h>
#include "generic_timer_regs.h"

#include <config/cfg_platform.h>

static void _generic_timer_hyp_irq_handler(int irq, void *regs, void *pdata);

static uint32_t _timer_irqs[GENERIC_TIMER_NUM_TYPES];
static uint32_t _tvals[GENERIC_TIMER_NUM_TYPES];
static generic_timer_callback_t _callback[GENERIC_TIMER_NUM_TYPES];

/**
*@return HVMM_STATUS_SUCCESS (It takes "0")
*@brief Initial generic timer each level : HYP, NSP, VIR
*/
hvmm_status_t generic_timer_init()
{
    _timer_irqs[GENERIC_TIMER_HYP] = 26;
    _timer_irqs[GENERIC_TIMER_NSP] = 27;
    _timer_irqs[GENERIC_TIMER_VIR] = 30;
    return HVMM_STATUS_SUCCESS;
}

/**
*@param enum generic_timer_type type, uint32_t tval
*@return HVMM_STATUS_SUCCESS or HVMM_STATUS_UNSUPPORTED_FEATURE
*@brief Setting generic timer value
*/
hvmm_status_t generic_timer_set_tval(enum generic_timer_type type,
                uint32_t tval)
{
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    if (type == GENERIC_TIMER_HYP) {
        _tvals[type] = tval;
        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_TVAL, tval);
        result = HVMM_STATUS_SUCCESS;
    } else {
    }
    return result;
}

/**
*@param enum generic_timer_type type
*@return HVMM_STATUS_UNSUPPORTED_FEATURE or HVMM_STATUS_SUCCESS
*@brief Enable generic timer
*/
hvmm_status_t generic_timer_enable_int(enum generic_timer_type type)
{
    uint32_t ctrl;
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    if (type == GENERIC_TIMER_HYP) {
        ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_HYP_CTRL);
        ctrl |= GENERIC_TIMER_CTRL_ENABLE;
        ctrl &= ~GENERIC_TIMER_CTRL_IMASK;
        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_CTRL, ctrl);
        result = HVMM_STATUS_SUCCESS;
    }
    return result;
}

/**
*@param enum generic_timer_type type
*@return HVMM_STATUS_UNSUPPORTED_FEATURE or HVMM_STATUS_SUCCESS
*@brief Disable generic timer
*/
hvmm_status_t generic_timer_disable_int(enum generic_timer_type type)
{
    uint32_t ctrl;
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    if (type == GENERIC_TIMER_HYP) {
        ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_HYP_CTRL);
        ctrl &= ~GENERIC_TIMER_CTRL_ENABLE;
        ctrl |= GENERIC_TIMER_CTRL_IMASK;
        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_CTRL, ctrl);
        result = HVMM_STATUS_SUCCESS;
    }
    return result;
}

/**
*@param int irq, void *regs, void *pdata
*@return void
*@brief Callback generic timer(hypervisor mode)
*/
static void _generic_timer_hyp_irq_handler(int irq, void *regs, void *pdata)
{
    _callback[GENERIC_TIMER_HYP](regs);
}

/**
*@param enum generic_timer_type type
*@return HVMM_STATUS_UNSUPPORTED_FEATURE or HVMM_STATUS_SUCCESS
*@brief Enable generic timer irq
*/
hvmm_status_t generic_timer_enable_irq(enum generic_timer_type type)
{
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    if (type == GENERIC_TIMER_HYP) {
        uint32_t irq = _timer_irqs[type];
        gic_test_set_irq_handler(irq, &_generic_timer_hyp_irq_handler, 0);
        gic_test_configure_irq(irq,
                               GIC_INT_POLARITY_LEVEL,
                               gic_cpumask_current(),
                               GIC_INT_PRIORITY_DEFAULT);
        result = HVMM_STATUS_SUCCESS;
    }
    return result;
}

/**
*@param enum generic_timer_type type, generic_timer_callback_t callback
*@return HVMM_STATUS_SUCCESS
*@brief Callback for setting
*/
hvmm_status_t generic_timer_set_callback(enum generic_timer_type type,
                generic_timer_callback_t callback)
{
    HVMM_TRACE_ENTER();
    _callback[type] = callback;
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

