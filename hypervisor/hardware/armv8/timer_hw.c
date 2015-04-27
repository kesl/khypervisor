#include <k-hypervisor-config.h>
#include <armv8_processor.h>
#include <timer.h>
#include <log/uart_print.h>
#include <asm-arm_inline.h>
#include <hvmm_trace.h>
#include <interrupt.h>

enum generic_timer_type {
    GENERIC_TIMER_HYP,      /* IRQ 15 */
    GENERIC_TIMER_VIR,      /* IRQ 14 */
    GENERIC_TIMER_NSP,      /* IRQ 13 */
    GENERIC_TIMER_NUM_TYPES
};

enum {
    GENERIC_TIMER_REG_FREQ,
    GENERIC_TIMER_REG_HCTL,
    GENERIC_TIMER_REG_KCTL,
    GENERIC_TIMER_REG_HYP_CTRL,
    GENERIC_TIMER_REG_HYP_TVAL,
    GENERIC_TIMER_REG_HYP_CVAL,
    GENERIC_TIMER_REG_PHYS_CTRL,
    GENERIC_TIMER_REG_PHYS_TVAL,
    GENERIC_TIMER_REG_PHYS_CVAL,
    GENERIC_TIMER_REG_VIRT_CTRL,
    GENERIC_TIMER_REG_VIRT_TVAL,
    GENERIC_TIMER_REG_VIRT_CVAL,
    GENERIC_TIMER_REG_VIRT_OFF,
};

#define GENERIC_TIMER_CTRL_ENABLE       (1 << 0)
#define GENERIC_TIMER_CTRL_IMASK        (1 << 1)
#define GENERIC_TIMER_CTRL_ISTATUS      (1 << 2)
#define generic_timer_pcounter_read()   read_cntpct()
#define generic_timer_vcounter_read()   read_cntvct()

/*
 * read Generic timer register, register size is 32bit
 */
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
        val = read_cntp_cval();
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

/** @brief Configures time interval by PL2 physical timerValue register.
 *  Read CNTHP_TVAL into R0.
 *
 *  "CNTHP_TVAL" characteristics
 *  -Holds the timer values for the Hyp mode physical timer.
 *  -Only accessible from Hyp mode, or from
 *   Monitor mode when SCR.NS is  set to 1.
 */
static hvmm_status_t generic_timer_set_tval(enum generic_timer_type timer_type,
        uint32_t tval)
{
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

    if (timer_type == GENERIC_TIMER_HYP) {
        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_TVAL, tval);
        result = HVMM_STATUS_SUCCESS;
    } else if (timer_type == GENERIC_TIMER_VIR) {
        generic_timer_reg_write(GENERIC_TIMER_REG_VIRT_TVAL, tval);
        result = HVMM_STATUS_SUCCESS;
    }

    return result;
}

/** @brief Enables the timer interrupt such as hypervisor timer event
 *  by PL2 Physical Timer Control register(VMSA : CNTHP_CTL)
 *  The Timer output signal is not masked.
 *
 *  The Cortex-A15 processor implements a 5-bit version of the interrupt
 *  priority field for 32 interrupt priority levels.
 */
static hvmm_status_t generic_timer_enable(enum generic_timer_type timer_type)
{
    uint32_t ctrl;
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

    if (timer_type == GENERIC_TIMER_HYP) {
        ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_HYP_CTRL);
        ctrl |= GENERIC_TIMER_CTRL_ENABLE;
        ctrl &= ~GENERIC_TIMER_CTRL_IMASK;

        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_CTRL, ctrl);
        result = HVMM_STATUS_SUCCESS;
    } else if (timer_type == GENERIC_TIMER_VIR) {
        ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_VIRT_CTRL);
        ctrl |= GENERIC_TIMER_CTRL_ENABLE;
        ctrl &= ~GENERIC_TIMER_CTRL_IMASK;
        generic_timer_reg_write(GENERIC_TIMER_REG_VIRT_CTRL, ctrl);
        result = HVMM_STATUS_SUCCESS;
    }

    return result;
}

/** @brief Disable the timer interrupt such as hypervisor timer event
 *  by PL2 physical timer control register.The Timer output signal is not masked.
 */
static hvmm_status_t generic_timer_disable(enum generic_timer_type timer_type)
{
    uint32_t ctrl;
    hvmm_status_t result = HVMM_STATUS_UNSUPPORTED_FEATURE;

    if (timer_type == GENERIC_TIMER_HYP) {
        ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_HYP_CTRL);
        ctrl &= ~GENERIC_TIMER_CTRL_ENABLE;
        ctrl |= GENERIC_TIMER_CTRL_IMASK;
        generic_timer_reg_write(GENERIC_TIMER_REG_HYP_CTRL, ctrl);
        result = HVMM_STATUS_SUCCESS;
    } else if (timer_type == GENERIC_TIMER_VIR) {
        ctrl = generic_timer_reg_read(GENERIC_TIMER_REG_VIRT_CTRL);
        ctrl &= ~GENERIC_TIMER_CTRL_ENABLE;
        ctrl |= GENERIC_TIMER_CTRL_IMASK;
        generic_timer_reg_write(GENERIC_TIMER_REG_VIRT_CTRL, ctrl);
        result = HVMM_STATUS_SUCCESS;
    }

    return result;
}

static hvmm_status_t timer_disable()
{
    return generic_timer_disable(GENERIC_TIMER_HYP);
}

static hvmm_status_t timer_enable()
{
    return generic_timer_enable(GENERIC_TIMER_HYP);
}

static hvmm_status_t timer_set_tval(uint64_t tval)
{
    return generic_timer_set_tval(GENERIC_TIMER_HYP, tval);
}


/** @brief dump at time.
 *  @todo have to write dump with meaningful printing.
 */
static hvmm_status_t timer_dump(void)
{
    HVMM_TRACE_ENTER();
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

struct timer_ops _timer_ops = {
    .enable = timer_enable,
    .disable = timer_disable,
    .set_interval = timer_set_tval,
    .dump = timer_dump,
};

struct timer_module _timer_module = {
    .name = "K-Hypervisor Timer Module",
    .author = "Kookmin Univ.",
    .ops = &_timer_ops,
};
