#ifndef __GENERIC_TIMER_REGS_H__
#define __GENERIC_TIMER_REGS_H__
#include "asm-arm_inline.h"
#include "armv7_p15.h"
#include "arch_types.h"

#define GENERIC_TIMER_CTRL_ENABLE       (1 << 0)
#define GENERIC_TIMER_CTRL_IMASK        (1 << 1)
#define GENERIC_TIMER_CTRL_ISTATUS      (1 << 2)
#define generic_timer_pcounter_read()   read_cntpct()
#define generic_timer_vcounter_read()   read_cntvct()


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

/**
*@param : register, unit32_t
*@return void
*@brief Generic timer write register value
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

/**
*@param int reg
*@return uint32_t
*@brief Generic timer read register
*/
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

/**
*@param int reg, uint64_t val
*@return void
*@brief Generic timer write register(64bit)
*/
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

/**
*@param int reg
*@return uint64_t
*@brief Generic timer read register(64bit)
*/
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
#endif
