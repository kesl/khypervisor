#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "hyp_config.h"
#include "arch_types.h"
#include "mm.h"

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

struct hyp_guest_context {
        struct arch_regs regs;
        lpaed_t *ttbl;
        vmid_t vmid;
};

void context_dump_regs( struct arch_regs *regs );
void context_switch_to_next_guest(struct arch_regs *regs_current);  
void context_switch_guest(void);
void context_init_guests(void);

#endif	// __CONTEXT_H__
