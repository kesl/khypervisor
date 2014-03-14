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
 *   - VMID Virtual Machine IDentifier
 *   - SP Stack Point Register
 *   - PSR Program Status Register
 *   - LR Link Register
 *   - GPR General Purpose Register [R0-R12]
 *   - PC Program Counter
 */
#define ARCH_REGS_NUM_GPR 13

/**
 * \defgroup TTBCR
 * - Translation Table Base Control Register(TTBCR) p15, c2, c0, 2
 *  - It determines which of TTBR (TTBR0 or TTBR1) defines the
 *    base address for a trnaslation table walk required for the
 *    stage-1 translation of a memory access from any mode other than
 *    Hyp mode.
 * - Long-descriptor translation table format
 *  - EAE[31] Extended Address Enable.
 *   - 0 32-bit translation system
 *   - 1 40-bit translation system
 *  - Implementation defined[30]
 *  - SH1[29:28] Shareability attribute for memory associated
 *    with translation table wlaks using TTBR1.
 *  - ORGN1[27:26] Outer cacheability attribute for memory associated
 *    with translation table walks using TTBR1.
 *  - IRGN1[25:24] Inner cacheability attribute for memory associated
 *    with translation table walks using TTBR1.
 *  - EPD1[23] Translation table walk disable for translations using
 *    TTBR1. This bit controls whether a translation table walk is performed
 *    on a TLB miss, for an address that is translated using TTBR1.
 *  - A1[22] Select whether TTBR0 or TTBR1 defines the ASID.
 *   - 0 TTBR0.ASID defines the ASID.
 *   - 1 TTBR1.ASID defines the ASID.
 *  - UNK/SBZP[21:19]
 *  - T1SZ[18:16] The size offset of the memory region addressed by TTBR1.
 *  - UNK/SBZP[15:14]
 *  - SH0[13:12] Shareability attribute for memory associated with
 *    translation table walks using TTBR0.
 *  - ORGN0[11:10] Outer cacheability attribute for memory assciated
 *    with translation table walks using TTBR0.
 *  - IRGN0[9:8] Inner cacheability attribute for memory associated
 *    with translation table walks using TTBR0.
 *  - EPD0[7] Translation table walk disable for translations using
 *    TTBR0. This bit controls whether translation table walk is performed
 *    on a TLB miss, for an address that is translated using TTBR0.
 *  - UNK/SBZP[6:3]
 *  - T0SZ[2:0] The size offset of the memory region addressed by TTBR0.
 *
 * - SHx
 *  - 00 non-shareable
 *  - 01 Unpredictable
 *  - 10 Outer Shareable
 *  - 11 Inner Shareable
 * - ORGNx
 *  - 00 Normal memory, Outer Non-cacheable
 *  - 01 Normal memory, Outer Write-Back Write-Allocate Cacheable
 *  - 10 Normal memory, Outer Write-Through Cacheable
 *  - 11 Normal memory, Outer Write-Back no Write-Allocate Cacheable
 * - IRGNx
 *  - 00 Normal memory, Inner Non-cacheable
 *  - 01 Normal memory, Inner Write-Back Write-Allocate Cacheable
 *  - 10 Normal memory, Inner Write-Through Cacheable
 *  - 11 Normal memory, Inner Write-Back no Write-Allocate Cacheable
 * - EPDx
 *  - 0 Perform translation table walks using TTBRx.
 *  - 1 A TLB miss on an address that is translated using TTBRx
 *    generates a Translation fault. No translation table walk is performed.
 */

/**
 * \defgroup TTBRx
 * - Translation Table Base Register x (TTBRx) p15 c2, c0, x
 * - Purpose
 *   TTBRx holds the base address of translation table x, and information about
 *   the memory it occupies. This is one of the translation tables for the
 *   stage-1 translation of memory accesses from modes other than Hyp mode.
 *   This register is part of the Virtual memory control registers functional
 *   group.
 * - Usage contraints
 *   Only accessible from PL1 or higher.
 *   Used in conjunction with the TTBCR. When the 64-bit TTBRx format is used,
 *   cacheability and shareability information is held in the TTBCR,
 *   not in TTBRx.
 * - Configurations
 *   The Multiprocessing Extensions change the TTBRx 32-bit format.
 *   In LPAE, TTBCR.EAE determines which TTBR0 format is used
 *  - EAE == 0 32-bit format is used.
 *  - EAE == 1 64-bit format is used.
 *
 * - 32-bit TTBR0 format
 *  - [31:x] Translation table base 0 address.
 *  - UNK/SBZP[x-1:6] without Multiprocessing Extensions.
 *  - UNK/SBZP[x-1:7] with Multiprocessing Extensions.
 *  - IRGN[0] [6] see bit 0 descripton.
 *  - NOS[5] Not Outer Shareable bit. Indicates the Outer Shareable
 *    attribute for the memory associated with a translation table walk
 *    that has the Shareable attribute, indicated by TTBR0.S ==1
 *   - 0 Outer Shareable.
 *   - 1 Inner Shareable.
 *  - RGN[4:3] Region bits. Indicates the Outer cacheability attributes for
 *    the memory associated with the translation table walks
 *   - 0b00 Normal memory, Outer Non-cacheable.
 *   - 0b01 Normal memory, Outer Write-Back Write-Allocate Cacheable.
 *   - 0b10 Normal memory, Outer Write-Through Cacheable.
 *   - 0b11 normal memory, Outer Write-Back no Write-Allocate Cacheable.
 *  - IMP[2] The effect of this bit is implementation defined.
 *    Else this bit is UNK/SBZP.
 *  - S[1] Shareable bit. Indicates the Shareable attribute for the memory
 *    associated with the translation table walks
 *   - 0 Non-shareable.
 *   - 1 Shareable.
 *  - C[0] without Multiprocessing Extension. Cacheable bit.
 *    indicates whether the translation table walk is to Inner Cacheable memory
 *   - 0 Inner non-cacheable.
 *   - 1 Inner Cacheable.
 *  - IRGN[6,0] with Multiprocessing Extensions.
 *    Inner region bits. Indicates the Inner Cacheability attributes for
 *    the memory associated with the translation table walks.
 *   - 0b00 Normal memory, Inner Non-cacheable.
 *   - 0b01 Normal memory, Inner Write-Back Write-Allocate Cacheable.
 *   - 0b10 Normal memory, Inner Write-Through Cacheable.
 *   - 0b11 Normal memory, Inner Write-Back no Write-Allocate Cacheable.
 *
 * - 32-bit TTBR1 format
 *  - [31:14] Translation table base 1 address. The translation table must be
 *    aligned on a 16KByte boundary.
 *  - behind is same 32-bit TTBR0 format.
 *
 * - 64-bit TTBR0 and TTBR1 format
 *  - UNK/SBZP [63:56]
 *  - ASID[55:48] An ASID for the translation table base address.
 *    The TTBCR.A1 field selects either TTBR0.ASID or TTBR1.ASID.
 *  - UNK/SBZP[47:40]
 *  - BADDR[39:x] Translation table base address.
 *    - TTBCR.T0SZ determines the x value for TTBR0.
 *    - TTBCR.T1SZ determines the x value for TTBR1.
 *    - TTBCR.TxSZ > 1 x = 14 - TxSZ
 *    - else x = 5 - TxSZ
 *  - UNK/SBZP[x-1:0]
 */

/**
 * @brief Function result of Hypervisor mode.
 */
enum hyp_hvc_result {
    HYP_RESULT_ERET = 0,  /**< Error status */
    HYP_RESULT_STAY = 1   /**< Stay status */
};

/**
 * @brief Architecture registers
 *
 * Used to store the current state registers.
 * - Registers
 *  - R0-R12
 *  - LR
 *  - PC
 *  - CPSR
 */
struct arch_regs {
    uint32_t cpsr; /**< CPSR*/
    uint32_t pc;   /**< PC*/
    uint32_t lr;   /**< LR*/
    uint32_t gpr[ARCH_REGS_NUM_GPR]; /**< R0 - R12 */
} __attribute((packed));

/**
 * @brief Registers which access via the coprocessor.
 * - cp15
 *  - Vector Base Address Register(VBAR) p15, c12, c0, 0
 *   - When high exception vectors are not selected, this holds the
 *     exception base address for exceptions that are not taken to
 *     Monitor mode or to Hyp mode. A 32-bit RW.
 *  - \ref TTBRx "TTBR0"
 *  - \ref TTBRx "TTBR1"
 *  - \ref TTBCR
 *  - System Control Register(SCTLR) p15, c1, c0, 0
 *   - It provides the top level control of the system, including its
 *     memory system. This register is part of Virtual memory control
 *     registers functional group.
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
 * @brief Context storage of hypervisor guest.
 *
 * Thie storage structure is storing and restoring the state(context)
 * of a guest process.
 * - arch_regs R0-R12, LR, PC, and SPSR.
 * - ttbl Address Translation Table.
 * - vmid Virtual machine Identifier.
 * - vgic_status Virtual Generic Interrupt Controller(GIC) status.
 * - arch_regs_cop VBAR, TTBR0, TTBR1, TTBCR, and SCTLR.
 * - arch_regs_banked User, svc, abt, und, irt, fiq mode sp, spsr, and lr.
 */
struct hyp_guest_context {
    struct arch_regs regs;  /**< Guest state registers */
    union lpaed *ttbl;    /**< Guest address translation table pointer */
    vmid_t vmid;    /**< Guest vmid */
    struct vgic_status vgic_status;  /**< Guest virtual GIC status */
    struct arch_regs_cop regs_cop;  /**< Guest registers through coprocessor */
    struct arch_regs_banked regs_banked;  /**< Guest banked register */
};

/**
 * @brief Show the status of registers.
 *
 * Show the status of registers when dumping context status.
 * - R0-R12
 * - LR
 * - PC
 * - CPSR
 *
 * @param regs Target Registers
 * @return void
 */
void context_dump_regs(struct arch_regs *regs);
/**
 * @brief Switching the context to the initial guest.
 *
 * Verrify the first context of the guest.
 * And performing the context switch to the initial guest.
 *
 * @return void.
 */
void context_switch_to_initial_guest(void);
/**
 * @brief Initialize all the guests
 *
 * Initialize all the guests.
 * Save the context of initialized guest after initialization.
 * - Initialize state
 *  - PC - 0x80000000 (guest Initial address)
 *  - CPSR - supervisor mode, interrupt disable
 *  - Initialize register which access via coprocessor, banked registers
 *  - Initialize virtual gic status.
 *
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
 * @brief Performs the context switch.
 *
 * Performing the context swithcing.
 * If currently running guest isn't exist, context switch to first guest.
 * When currnt guest is running, performs switching to the next guest.
 * 
 * @return Result of context swithcing the guests.
 */
hvmm_status_t context_perform_switch(void);

/**
 * @brief VMID of the current guest.
 *
 * Return the vmid of the current guest.
 * @return _current_guest_vmid VMID of current guest.
 */
vmid_t context_current_vmid(void);

/**
 * @brief VMID of the next guest to be switched.
 *
 * Return the vmid of the next guest.
 * @return _next_guest_vmid VMID of the next guset.
 */
vmid_t context_waiting_vmid(void);

/* Requests switch to the guest 'vmid' */
/**
 * @brief Verifying the vmid of the context which can be switchable context.
 *
 * Verifying the vmid which can be valid switchable context
 * and return the result.
 *
 * @param vmid Verification object.
 * @return Result of verification.
 */
hvmm_status_t context_switchto(vmid_t vmid);
/**
 * @brief Verifying the vmid which is valid context to the context switch.
 *
 * Verifying the vmid which is valid to switchable.
 * When switching sequence is locked, verification will be denied.
 *
 * @param vmid Verification target.
 * @param locked Configure switching lock.
 * @return Result of verification.<br>
 * When switchable, result value is HVMM_STATUS_SUCCESS.<br>
 * Else, result value is HVMM_STATUS_BAD_ACCESS or HVMM_STATUS_IGNORED.
 */
hvmm_status_t context_switchto_lock(vmid_t vmid, uint8_t locked);

/* available guest vmid query */
/**
 * @brief Returns the vmid of the first guest.
 *
 * @return VMID of the first guest. - 0
 * @todo Change the source code.
 */
vmid_t context_first_vmid(void);
/**
 * @brief Returns the vmid of the last guest.
 *
 * @return VMID of the last guest. - 1
 * @todo Change the source code.
 */
vmid_t context_last_vmid(void);
/**
 * @brief Returns the next vmid of the base vmid.
 *
 * Returns the next vmid of the vmid delivered.
 * If delivered vmid is invalid, return the first vmid.
 * @param ofvmid Base vmid of the next vmid
 * @return Next vmid
 */
vmid_t context_next_vmid(vmid_t ofvmid);
/**
 * @brief Returns the context which has delivered vmid.
 *
 * @param vmid The vmid which the target context has.
 * @return Context of the next vmid.
 */
struct hyp_guest_context *context_atvmid(vmid_t vmid);

extern void __mon_switch_to_guest_context(struct arch_regs *regs);

extern uint32_t guest_bin_start;
extern uint32_t guest_bin_end;
extern uint32_t guest2_bin_start;

#endif    /* __CONTEXT_H__ */
