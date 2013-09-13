#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "hyp_config.h"
#include "arch_types.h"
#include "vgic.h"
#include "lpae.h"

#define NUM_GUEST_CONTEXTS		NUM_GUESTS_STATIC
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

/* co-processor registers: cp15 */
struct arch_regs_cop {
    uint32_t vbar;
};

struct hyp_guest_context {
        struct arch_regs regs;
        lpaed_t *ttbl;
        vmid_t vmid;
        struct vgic_status vgic_status;
        struct arch_regs_cop regs_cop;
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

/* available guest vmid query */
vmid_t context_first_vmid(void);
vmid_t context_last_vmid(void);
vmid_t context_next_vmid(vmid_t ofvmid);

#endif	// __CONTEXT_H__
