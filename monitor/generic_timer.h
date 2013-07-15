#ifndef __GENERIC_TIMER_H__
#define __GENERIC_TIMER_H__

#include "hvmm_types.h"
#include "arch_types.h"
#include "timer.h"
#include "armv7_p15.h"
#include "uart_print.h"
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
#define isb()           asm volatile ("" : : : "memory")
#define GENERIC_TIMER_CTRL_ENABLE		(1 << 0)
#define GENERIC_TIMER_CTRL_IMASK		(1 << 1)
#define GENERIC_TIMER_CTRL_ISTATUS		(1 << 2)

#define generic_timer_pcounter_read()	read_cntpct()
#define generic_timer_vcounter_read()   read_cntvct()

typedef enum {
	GENERIC_TIMER_HYP,		/* IRQ 26 */
	GENERIC_TIMER_VIR,		/* IRQ 27 */
	GENERIC_TIMER_NSP,		/* IRQ 30 */
	GENERIC_TIMER_NUM_TYPES
} generic_timer_type_t;

typedef void (*generic_timer_callback_t)(void *pdata);

/* Input timer information & register funtion pointer */
hvmm_status_t generic_timer_init(struct timer_source *ts);
/* Enable the timer. Timer output signal masking. */ 
hvmm_status_t generic_timer_enable_int(generic_timer_type_t type);
/* Disable the timer. */
hvmm_status_t generic_timer_disable_int(generic_timer_type_t typevoid);
/* */
hvmm_status_t generic_timer_set_tval(generic_timer_type_t type, uint32_t interval);
/* */
hvmm_status_t generic_timer_enable_irq(generic_timer_type_t type);
/* TODO Add callback funtion. Called when occur timer interrupt */
hvmm_status_t generic_timer_set_callback(generic_timer_type_t type, generic_timer_callback_t callback);
/* TODO Get System time. Not clear */
void generic_timer_get_time(struct timeval* timeval);
/* generic timer interrutp handler */
//static void generic_timer_irq_handler(int irq, void *regs, void *pata);

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

#endif

