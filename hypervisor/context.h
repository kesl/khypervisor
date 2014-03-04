#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <k-hypervisor-config.h>
#include "arch_types.h"
#include "vgic.h"
#include "lpae.h"

#define ARCH_REGS_NUM_GPR    13
/**
 * @brief Hypervisor hvc result
 */
enum hyp_hvc_result {
    HYP_RESULT_ERET = 0, /**< error status return */
    HYP_RESULT_STAY = 1  /**< stay status return */
};

/**
 * @brief Architecture Registers
 *
 * - Used to save the Current State Registers<br>
 * - Registers<br>
 *   - CPSR<br>
 *   - PC<br>
 *   - GPR[0-12]
 */
struct arch_regs {
    uint32_t cpsr;  /**< CPSR */
    uint32_t pc;    /**< Program Counter */
    uint32_t lr;    /**< Link Register */
    uint32_t gpr[ARCH_REGS_NUM_GPR];  /**< R0 - R12 */
} __attribute((packed));

/**
 *  @brief Co-Processor Registers: cp15, cp2
 */
struct arch_regs_cop {
    uint32_t vbar;   /**< Vector Base Address Register */
    uint32_t ttbr0;  /**< Translation Table Base Address Register 0 */
    uint32_t ttbr1;  /**< Translation Table Base Address Register 1 */
    uint32_t ttbcr;  /**< Translation Table Base Control Register */
    uint32_t sctlr;  /**< System Control Register */
};

/**
 * @brief Banked registers
 */
struct arch_regs_banked {
    uint32_t sp_usr;    /**< User Mode Stack Point Register */
    uint32_t spsr_svc;  /**< SVc Mode Saved Program Status Register */
    uint32_t sp_svc;    /**< SVC Mode Stack point Register */
    uint32_t lr_svc;    /**< SVC Mode Link Register */
    uint32_t spsr_abt;  /**< Abort Mode Saved Program Status Register */
    uint32_t sp_abt;    /**< Abort Mode Stack Point Register */
    uint32_t lr_abt;    /**< Abort Mode Link Register */
    uint32_t spsr_und;  /**< Undefined Mode Saved Program Status Register */
    uint32_t sp_und;    /**< Undefined Mode Stack point Register */
    uint32_t lr_und;    /**< Undefined Mode Link Register */
    uint32_t spsr_irq;  /**< Interrupt Mode Saved Program Status Register */
    uint32_t sp_irq;    /**< Interrupt Mode Stack Point Register */
    uint32_t lr_irq;    /**< Interrupt Mode Link Register */

    uint32_t spsr_fiq;  /**< FIQ Mode Saved Program Status Register */
    /* Cortex-A15 processor does not support sp_fiq */
    /* uint32_t sp_fiq; */
    uint32_t lr_fiq;    /**< FIQ Mode Link Register */
    uint32_t r8_fiq;    /**< FIQ Mode Register 8 */
    uint32_t r9_fiq;    /**< FIQ Mode Register 9 */
    uint32_t r10_fiq;   /**< FIQ Mode Register 10 */
    uint32_t r11_fiq;   /**< FIQ Mode Register 11 */
    uint32_t r12_fiq;   /**< FIQ Mode Register 12 */
};

/**
 * @brief Hypervisor guest's context structure
 *
 * arch_regs, ttbl, vmid, vgic_status, arch_regs_cop, arch_regs_banked
 */
struct hyp_guest_context {
    struct arch_regs regs;  /**< Guest's architecture registers */
    union lpaed *ttbl;    /**< Guest's translation table */
    vmid_t vmid;    /**< Guest's Virtual machine ID */
    struct vgic_status vgic_status;  /**< Guest's Virtual GIC Status */
    struct arch_regs_cop regs_cop;  /**< Guest's Co-Processor Registers */
    struct arch_regs_banked regs_banked;  /**< Guet's Banked Register */
};

/**
 * @fn void context_dump_regs( struct arch_regs *regs)
 * @brief Show delivered register's status
 *
 * CPSR, PC, LR, GPR[0-ARCH_REGS_NUM_GPR]
 * @param regs
 * @return void
 */
void context_dump_regs(struct arch_regs *regs);
/**
 * @fn context_switch_to_initial_guest
 * @brief Switch the context to initial guest
 *
 * Select the First Guest context to switch to.<br>
 * Dump the Initial Register values of the guest for debugging purpose.<br>
 * Context Switch with Current context == none.
 *
 * @param void
 * @return void
 */
void context_switch_to_initial_guest(void);

/**
 * @brief Initialize guests 0, 1
 *
 * initialize guet 0, 1<br>
 * cpsr -supervisor, interrupt disable<br>
 * init co-processor register, banked registers, vgic<br>
 * load guest image and if linux, setup tags
 *
 * @param void
 * @return void
 */
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
/**
 * @brief Perform the context switching
 *
 * - First switch to the initial guest<br>
 * - else, _current_guest_vmid is not same to _next_guest_vmid,<br>
 * - then switch to next guest<br>
 * - else, _current_guest_vmid is _next_guest_vmid, then,<br>
 *   Staying at the currently active guest.<br>
 *   Flush out queued virqs since we didn't have a chance<br>
 *   to switch the context,<br>
 * where virq flush takes place, this time
 * @param void
 * @return result of swithcing
 */
hvmm_status_t context_perform_switch(void);

/**
 * @brief VMID of the current guest
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

/**
 * @brief Requests switch to the guest with'vmid'
 * @param vmid_t vmid
 * @return result of switch.<br>
 * success - HVMM_STATUS_SUCCESS<br>
 * vmid vmid is not valied - HVMM_STATUS_BAD_ACCESS<br>
 * switching is locked - HVMM_STATUS_IGNORED<br>
 */
hvmm_status_t context_switchto(vmid_t vmid);
/**
 * @brief Switch the context with delivered vmid
 * @param vmid target vmid
 * @param locked set context's switching lock<br>
 * @return result of switch.<br>
 * success - HVMM_STATUS_SUCCESS<br>
 * vmid is not valied - HVMM_STATUS_BAD_ACCESS<br>
 * switching is locked - HVMM_STATUS_IGNORED<br>
 */
hvmm_status_t context_switchto_lock(vmid_t vmid, uint8_t locked);

/* available guest vmid query */
/**
 * @brief Get first context's virtual machine id
 * @param void
 * @return return first guest's virtual machine id - 0
 * @todo change the source code
 */
vmid_t context_first_vmid(void);
/**
 * @brief Get last context's virtual machine id
 * @param void
 * @return last guest's virtual machine id - 1
 * @todo change the source code
 */
vmid_t context_last_vmid(void);
/**
 * @brief Get the next context's virtual machine id from ofvmid
 * @param target virtual machine id
 * @return next virtual machine id
 */
vmid_t context_next_vmid(vmid_t ofvmid);
/**
 * @brief Get guest's context with vmid
 * @param vmid wished guest's context vmid
 * @return guest's context
 */
struct hyp_guest_context *context_atvmid(vmid_t vmid);

/**
 * @brief Initialize the hyp mode configuration & start guest
 *
 * Initialize hyp mode mm, interrupt, guest, vdev, vgic<br>
 * Mapping PIRQ to VIRQ<br>
 * Start scheduler<br>
 * Start Initial Guest
 *
 * @param void
 * @return void
 */
void start_guest_os(void);

extern void __mon_switch_to_guest_context(struct arch_regs *regs);

extern uint32_t guest_bin_start;
extern uint32_t guest_bin_end;
extern uint32_t guest2_bin_start;

#endif    /* __CONTEXT_H__ */
