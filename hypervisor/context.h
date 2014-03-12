#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <k-hypervisor-config.h>
#include "arch_types.h"
#include "vgic.h"
#include "lpae.h"

/**
 * @file context.h
 *
 * - Glossary
 *   - VMIDi - Virtual Machine ID
 *   - SP - Stack Point Register
 *   - PSR - Program Status Register
 *   - LR - Link Register
 *   - GPR - General Perpuose Register[R0-R12]
 *   - PC - Program Counter
 */
#define ARCH_REGS_NUM_GPR    13

/**
 * @brief Hypervisor mode function's result
 */
enum hyp_hvc_result {
    HYP_RESULT_ERET = 0,  /**< Error statun*/
    HYP_RESULT_STAY = 1   /**< Stay status*/
};

/**
 * @brief Architecture registers
 *
 * Used to save the current state registers
 * - Registers
 *   - CPSR
 *   - PC
 *   - R0-R12
 */
struct arch_regs {
    uint32_t cpsr; /**< CPSR*/
    uint32_t pc;   /**< PC*/
    uint32_t lr;   /**< LR*/
    uint32_t gpr[ARCH_REGS_NUM_GPR]; /**< R0 - R12 */
} __attribute((packed));

/**
 * @brief Coprocessor registers: cp15, c2
 */
struct arch_regs_cop {
    uint32_t vbar;   /**< Vector Base Address Register */
    uint32_t ttbr0;  /**< Translation Table Base Register 0 */
    uint32_t ttbr1;  /**< Translation Table Base Register 1 */
    uint32_t ttbcr;  /**< Translation Table Base Control Register */
    uint32_t sctlr;  /**< System Control Register */
};

/**
 * @brief Banked registers
 */
struct arch_regs_banked {
    uint32_t sp_usr;    /**< User mode SP */
    uint32_t spsr_svc;  /**< Svc mode SPSR */
    uint32_t sp_svc;    /**< SvC mode SP */
    uint32_t lr_svc;    /**< SvC mode LR */
    uint32_t spsr_abt;  /**< Abort mode SPSR */
    uint32_t sp_abt;    /**< Abort mode SP */
    uint32_t lr_abt;    /**< Abort mode LR */
    uint32_t spsr_und;  /**< Undefined mode SPSR */
    uint32_t sp_und;    /**< Undefined mode SP */
    uint32_t lr_und;    /**< Undefined mode LR */
    uint32_t spsr_irq;  /**< Interrupt mode SPSR */
    uint32_t sp_irq;    /**< Interrupt mode SP */
    uint32_t lr_irq;    /**< Interrupt mode LR */

    uint32_t spsr_fiq;  /**< FIQ mode SPSR */
    /* Cortex-A15 processor does not support sp_fiq */
    /* uint32_t sp_fiq; */
    uint32_t lr_fiq;    /**< FIQ mode LR */
    uint32_t r8_fiq;    /**< FIQ mode R8 */
    uint32_t r9_fiq;    /**< FIQ mode R9 */
    uint32_t r10_fiq;   /**< FIQ mode R10 */
    uint32_t r11_fiq;   /**< FIQ mode R11 */
    uint32_t r12_fiq;   /**< FIQ mode R12 */
};

/**
 * @brief Hypervisor guest's context structure
 *
 * - arch_regs - SPSR, PC, LR, and R0-R12
 * - ttbl - Translation Table Pointer
 * - vmid - Vmid
 * - vgic_status - Virtual GIC status
 * - arch_regs_cop - Vbar, ttbr0, ttbr1, ttbcr, and sctlr
 * - arch_regs_banked - usr, svc, abt, und, irt, fiq mode sp, spsr, lr
 */
struct hyp_guest_context {
    struct arch_regs regs;  /**< Guest state registers */
    union lpaed *ttbl;    /**< Guest translation table pointer */
    vmid_t vmid;    /**< Guest vmid */
    struct vgic_status vgic_status;  /**< Guest virtual GIC status */
    struct arch_regs_cop regs_cop;  /**< Guest coprocessor registers */
    struct arch_regs_banked regs_banked;  /**< Guest banked register */
};

/**
 * @brief Show state register's status
 *
 * - Show register's status
 *  - CPSR
 *  - PC
 *  - LR
 *  - R0-R12
 * @param regs Target Registers
 * @return void
 */
void context_dump_regs(struct arch_regs *regs);
void context_switch_to_initial_guest(void);
void context_init_guests(void);
/*
 * Example:
 *      // determine the next guest
 *
 *      next = context_next_vmid(context_current_vmid())
 *      if (next == VMID_INVALID ) next = context_first_vmid();
 *
 *      // request switching
 *      context_switchto(next);
 *      ...
 *      // only once before leaving hyp mode
 *      context_perform_switch();
 */
hvmm_status_t context_perform_switch(void);

/**
 * @brief Current guest's vmid
 * @param void
 * @return _current_guest_vmid
 */
vmid_t context_current_vmid(void);

/**
 * @brief VMID of the next guest to be switched to
 * @param void
 * @return _next_guest_vmid
 */
vmid_t context_waiting_vmid(void);

/* Requests switch to the guest 'vmid' */
hvmm_status_t context_switchto(vmid_t vmid);
hvmm_status_t context_switchto_lock(vmid_t vmid, uint8_t locked);

/* available guest vmid query */
/**
 * @brief First guest's vmid return
 * @param void
 * @return First guest's vmid - 0
 * @todo change the source code
 */
vmid_t context_first_vmid(void);
/**
 * @brief Last guest's vmid return
 * @param void
 * @return Last guest's vmid - 1
 * @todo change the source code
 */
vmid_t context_last_vmid(void);
/**
 * @brief Return the next guest's vmid from parameter(vmid)
 * @param ofvmid Target vmid
 * @return Next vmid
 */
vmid_t context_next_vmid(vmid_t ofvmid);
/**
 * @brief Return the guest's context which has delivered vmid
 * @param vmid Wished guest's context vmid
 * @return Guest's context pointer
 */
struct hyp_guest_context *context_atvmid(vmid_t vmid);

extern void __mon_switch_to_guest_context(struct arch_regs *regs);

extern uint32_t guest_bin_start;
extern uint32_t guest_bin_end;
extern uint32_t guest2_bin_start;

#endif    /* __CONTEXT_H__ */
