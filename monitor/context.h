#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "hyp_config.h"
#include "arch_types.h"
#include "vgic.h"
#include "lpae.h"

#define ARCH_REGS_NUM_GPR	13

typedef enum {
        HYP_RESULT_ERET = 0,
        HYP_RESULT_STAY = 1
} hyp_hvc_result_t;

struct arch_regs {
        unsigned int cpsr; /* CPSR */
        unsigned int pc; /* Program Counter */
        unsigned int gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
} __attribute((packed));

/* co-processor registers: cp15, cp2 */
struct arch_regs_cop {
    uint32_t vbar;
	uint32_t ttbr0;
	uint32_t ttbr1;
	uint32_t ttbcr;
};

/* banked registers */
struct arch_regs_banked {
    uint32_t spsr_usr;
    uint32_t sp_usr;
    uint32_t lr_usr;
    uint32_t spsr_svc;
    uint32_t sp_svc;
    uint32_t lr_svc;
    uint32_t spsr_abt;
    uint32_t sp_abt;
    uint32_t lr_abt;
    uint32_t spsr_und;
    uint32_t sp_und;
    uint32_t lr_und;
    uint32_t spsr_irq;
    uint32_t sp_irq;
    uint32_t lr_irq;

    uint32_t spsr_fiq;
	//Cortex-A15 processor does not support sp_fiq
    //uint32_t sp_fiq;
    uint32_t lr_fiq;
    uint32_t r8_fiq;
    uint32_t r9_fiq;
    uint32_t r10_fiq;
    uint32_t r11_fiq;
    uint32_t r12_fiq;
};

struct hyp_guest_context {
        struct arch_regs regs;
        lpaed_t *ttbl;
        vmid_t vmid;
        struct vgic_status vgic_status;
        struct arch_regs_cop regs_cop;
        struct arch_regs_banked regs_banked;
};

void context_dump_regs( struct arch_regs *regs );
void context_switch_to_initial_guest(void);
void context_init_guests(void);
/*
 * Example:
 *      // determine the next guest
 *
 *      next = context_next_vmid(context_current_vmid())
 *      if ( next == VMID_INVALID ) next = context_first_vmid();
 *
 *      // request switching
 *      context_switchto(next);
 *      ...
 *      // only once before leaving hyp mode
 *      context_perform_switch();
 */
hvmm_status_t context_perform_switch(void);

/* VMID of the current guest */
vmid_t context_current_vmid(void);

/* VMID of the next guest to be switched to */
vmid_t context_waiting_vmid(void);

/* Requests switch to the guest 'vmid' */
hvmm_status_t context_switchto(vmid_t vmid);
hvmm_status_t context_switchto_lock(vmid_t vmid, uint8_t locked);

/* available guest vmid query */
vmid_t context_first_vmid(void);
vmid_t context_last_vmid(void);
vmid_t context_next_vmid(vmid_t ofvmid);
struct hyp_guest_context *context_atvmid(vmid_t vmid);

#endif	// __CONTEXT_H__
