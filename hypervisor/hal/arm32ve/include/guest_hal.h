#ifndef _GUEST_HAL_H__
#define _GUEST_HAL_H__

#include <k-hypervisor-config.h>
#include <test/tests.h>
#include <version.h>
#include <log/print.h>
#include <hvmm_trace.h>
#include <virq.h>
#include <vgic.h>
#include <lpae.h>

#define ARCH_REGS_NUM_GPR    13

/* co-processor registers: cp15, cp2 */
struct regs_cop {
    uint32_t vbar;
    uint32_t ttbr0;
    uint32_t ttbr1;
    uint32_t ttbcr;
    uint32_t sctlr;
};

/* banked registers */
struct regs_banked {
    uint32_t sp_usr;
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
    /* Cortex-A15 processor does not support sp_fiq */
    /* uint32_t sp_fiq; */
    uint32_t lr_fiq;
    uint32_t r8_fiq;
    uint32_t r9_fiq;
    uint32_t r10_fiq;
    uint32_t r11_fiq;
    uint32_t r12_fiq;
};

/* Defines the architecture specific registers */
struct arch_regs {
    uint32_t cpsr; /* CPSR */
    uint32_t pc; /* Program Counter */
    uint32_t lr;
    uint32_t gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
} __attribute((packed));

/* Defines the architecture specific information, except general regsiters */
struct arch_context {
    struct vgic_status vgic_status;
    struct regs_cop regs_cop;
    struct regs_banked regs_banked;
    union lpaed *ttbl;
};

#endif
