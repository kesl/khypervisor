#ifndef _GUEST_HW_H__
#define _GUEST_HW_H__

#include <k-hypervisor-config.h>
#include <test/tests.h>
#include <version.h>
#include <log/print.h>
#include <hvmm_trace.h>
#include <vgic.h>

#define ARCH_REGS_NUM_GPR    31

/* system registers */
struct regs_sys{
    uint64_t vbar_el1;
    uint64_t ttbr0_el1;
    uint64_t ttbr1_el1;
    uint64_t tcr_el1;
    uint32_t sctlr_el1;
    uint64_t sp_el0;
    uint64_t sp_el1;
    uint64_t elr_el1;
};

/* Defines the architecture specific registers */
struct arch_regs {
    uint32_t cpsr; /* CPSR, SPSR_EL1 */
    uint64_t pc; /* Program Counter */
    uint64_t gpr[ARCH_REGS_NUM_GPR]; /* X0 - X29 */
};

/* Defines the architecture specific information, except general regsiters */
struct arch_context {
    struct regs_sys regs_sys;
};

#endif
