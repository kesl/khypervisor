#include <hvmm_trace.h>
#include <armv7_p15.h>
#include "trap.h"
#include "context.h"
#include "gic.h"
#include "guest.h"
#include "emulate_trapped_inst.h"
#include "trap.h"
#include <vdev.h>
#define DEBUG
#include <log/print.h>

/*
 * ISS encoding for Data Abort exceptions taken to Hyp mode as beloww
 * ISS[24] : instruction syndrome valid. 0 is invalid information in ISS.
 * 1 is valid information in ISS
 * when ISS[24] is 0, we don't need to extract information
 * from the rest of ISS field
 * when ISS[24] is 1, we need to extract information from ISS[26:13]
 *
 * ISS[26:13] is consist of 10 parts. Details as below
 *
 * - ISS[23:22] is an access size.
 * - e.g. byte, hardword, word
 * - ISS[21] is a sign extend
 * - e.g. 1 is not sign, 1 is sign
 * - ISS[20] is reserved
 * - ISS[19:16] is for register transfer ?
 * - ISS[15:10] is reserved
 * - ISS[9] is an external abort type. It is IMPLEMENTATION_DEFINED
 * - ISS[8] is a cache maintenance. For synchronous fault, it should
 * need a cache maintenance.
 * - ISS[7] is a stage 2 fault for a stage 1 translation table walk
 * - ISS[6] is synchronous abort that was caused by a write or read operation
 * - ISS[5:0] is a data fault status code(DFSC)
 * Additional register we should reference a DFSR
 */

#define ISS_VALID                        0x01000000

#define ISS_FSR_MASK                      0x0000003F
#define ISS_TRANS_FAULT_MASK            0x07
#define TRANS_FAULT_LEVEL1                0x05
#define TRANS_FAULT_LEVEL2                0x06
#define TRANS_FAULT_LEVEL3                0x07
#define ACCESS_FAULT_LEVEL0                0x08
#define ACCESS_FAULT_LEVEL1                0x09
#define ACCESS_FAULT_LEVEL2                0x0A
#define ACCESS_FAULT_LEVEL3                0x0B

#define ISS_WNR_SHIFT                   6
#define ISS_WNR                         (1 << ISS_WNR_SHIFT)

#define ISS_SAS_SHIFT                   22
#define ISS_SAS_MASK                    (0x3 << ISS_SAS_SHIFT)
#define ISS_SAS_BYTE                    0x0
#define ISS_SAS_HWORD                   0x1
#define ISS_SAS_WORD                    0x2
#define ISS_SAS_RESERVED                0x3

#define ISS_SSE_SHIFT                    21
#define ISS_SSE_MASK                    (0x1 << ISS_SSE_SHIFT)

#define ISS_SRT_SHIFT                   16
#define ISS_SRT_MASK                    (0xf << ISS_SRT_SHIFT)

/* HPFAR */
#define HPFAR_INITVAL                   0x00000000
#define HPFAR_FIPA_MASK                 0xFFFFFFF0
#define HPFAR_FIPA_SHIFT                4
#define HPFAR_FIPA_PAGE_MASK                0x00000FFF
#define HPFAR_FIPA_PAGE_SHIFT               12

/* By holding the address to the saved regs struct,
 context or other modules can access to this structure
 through trap_saved_regs() call when it's needed.
 For example, copying register values for context switching can be
 performed this way.
 */
static struct arch_regs *_trap_hyp_saved_regs;
/**
 * @brief Handles data abort exception taken from a mode other than Hyp mode
 * @param regs arm registers
 * <br> which includes 13 general purpose register r0-r12, 1 Stack Pointer (SP),
 * <br> 1 Link Register (LR), 1 Program Counter (PC)
 * <br> this fuction uses current arm registers to dump and save as parameter
 * @return The result of function process, it doesn't reach step of return due to infinte loop
 */
hvmm_status_t _hyp_dabort(struct arch_regs *regs)
{
    _trap_hyp_saved_regs = regs;
    context_dump_regs(regs);
    hyp_abort_infinite();
    return HVMM_STATUS_UNKNOWN_ERROR;
}

/**
 * @brief Handles IRQ exception whenever hardware interrupt break out
 * <br> this fucntion called gic interrupt and switched context
 * @param regs arm registers
 * @return The result of function process, if it reach last step, it will return HVMM_STATUS_SUCCESS
 */
hvmm_status_t _hyp_irq(struct arch_regs *regs)
{
    _trap_hyp_saved_regs = regs;
    gic_interrupt(0, regs);
    context_perform_switch();
    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Handles unhandled exception whenever undefined exception break out
 * @param regs arm registers
 * @return The result of function process, it doesn't reach step of return due to infinte loop
 */
hvmm_status_t _hyp_unhandled(struct arch_regs *regs)
{
    _trap_hyp_saved_regs = regs;
    context_dump_regs(regs);
    hyp_abort_infinite();

    return HVMM_STATUS_UNKNOWN_ERROR;
}

/**
 * @brief Indirecting _hyp_hvc_service function in file
 * @param regs arm registers
 * @return The result of _hyp_hvc_service() function
 */
enum hyp_hvc_result _hyp_hvc(struct arch_regs *regs)
{
    return _hyp_hvc_service(regs);
}

hvmm_status_t trap_hvc_dabort(unsigned int iss, struct arch_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    /* far, fipa, il */
    uint32_t far = read_hdfar();
    uint32_t fipa;
    uint32_t sas, srt, wnr;
    HVMM_TRACE_ENTER();
    printh("trap_hvc_dabort: hdfar:%x hpfar:%x\n", far, read_hpfar());
    fipa = (read_hpfar() & HPFAR_FIPA_MASK) >> HPFAR_FIPA_SHIFT;
    fipa = fipa << HPFAR_FIPA_PAGE_SHIFT;
    fipa = fipa | (far & HPFAR_FIPA_PAGE_MASK);
    sas = (iss & ISS_SAS_MASK) >> ISS_SAS_SHIFT;
    srt = (iss & ISS_SRT_MASK) >> ISS_SRT_SHIFT;
    wnr = (iss & ISS_WNR) ? 1 : 0;
    if ((iss & ISS_VALID) && ((iss & ISS_FSR_MASK) < 8)) {
        /*
           vdev emulates read/write, update pc, update destination register
         */
        result = vdev_emulate(fipa, wnr, (enum vdev_access_size) sas,
                    srt, regs);
        if (result != HVMM_STATUS_SUCCESS) {
            printh("trap_dabort: emulation failed guest pc:%x\n", regs->pc);
            /* Let the guest continue by increasing pc */
            regs->pc += 4;
        }
    } else {
        printh("trap_dboart: fipa=0x%x\n", fipa);
        result = HVMM_STATUS_BAD_ACCESS;
    }
    if (result != HVMM_STATUS_SUCCESS)
        printh("- INSTR: %s[%d] r%d [%x]\n",
                wnr ? "str" : "ldr", (sas + 1) * 8, srt, fipa);
    switch (iss & ISS_FSR_MASK) {
    case TRANS_FAULT_LEVEL1:
    case TRANS_FAULT_LEVEL2:
    case TRANS_FAULT_LEVEL3:
        break;
    case ACCESS_FAULT_LEVEL1:
    case ACCESS_FAULT_LEVEL2:
    case ACCESS_FAULT_LEVEL3:
        printh("ACCESS fault %d\n", iss & ISS_FSR_MASK);
        break;
    default:
        break;
    }
    HVMM_TRACE_EXIT();
    return result;
}

/**
 * @brief Showing arm registers(gpr, spsr, lr, sp) value for debugging mode
 */
static void _trap_dump_bregs(void)
{
    uint32_t spsr, lr, sp;
    printh(" - banked regs\n");
    asm volatile(" mrs     %0, sp_usr\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_usr\n\t" : "=r"(lr) : : "memory", "cc");
    printh(" - usr: sp:%x lr:%x\n", sp, lr);
    asm volatile(" mrs     %0, spsr_svc\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    printh(" - svc: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
    asm volatile(" mrs     %0, spsr_irq\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_irq\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_irq\n\t" : "=r"(lr) : : "memory", "cc");
    printh(" - irq: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
}

/*
 * hvc #imm handler
 *
 * HYP Syndrome Register(HSR)
 * EC[31:26] is an exception class for the exception that is taken to HYP mode
 * IL[25] is an instruction length for the trapped insruction that is
 * 16 bit or 32 bit
 * ISS[24:0] is an instruction-specific syndrome for the instruction
 * information included. It depends on EC field.
 * END OF HSR DESCRIPTION FROM ARM DDI0406_C ARCHITECTURE MANUAL
 */

enum hyp_hvc_result _hyp_hvc_service(struct arch_regs *regs)
{
    unsigned int hsr = read_hsr();
    unsigned int ec = (hsr & HSR_EC_BIT) >> EXTRACT_EC;
    /* unused variable */
    /* unsigned int il = (hsr & HSR_IL_BIT) >> EXTRACT_IL; */
    unsigned int iss = hsr & HSR_ISS_BIT;
    _trap_hyp_saved_regs = regs;
    printh("[hvc] _hyp_hvc_service: enter\n\r");
    switch (ec) {
    case TRAP_EC_ZERO_UNKNOWN:
        printh("Unknown reason: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_WFI_WFE:
        printh("Trapped WFI or WFE instruction: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCR_MRC_CP15:
        printh("Trapped MCR or MRC access to CP15: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCRR_MRRC_CP15:
        printh("Trapped MCRR or MRRC access to CP15: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCR_MRC_CP14:
        printh("Trapped MCR or MRC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_LDC_STC_CP14:
        printh("Trapped LDC or STC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_HCRTR_CP0_CP13:
        printh("HCPTR-trapped access to CP0-CP13: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MRC_VMRS_CP10:
        printh(
            "Trapped MRC or VMRS access to CP10, for ID group traps: 0x%08x\n",
            hsr);
        break;
    case TRAP_EC_ZERO_BXJ:
        printh("Trapped BXJ instruction: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MRRC_CP14:
        printh("Trapped MRRC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_NON_ZERO_SVC:
        printh("Supervisor Call exception routed to Hyp mode: 0x%08x\n", hsr);
        break;
    case TRAP_EC_NON_ZERO_HVC: {
        if ((iss & 0xFFFF) == 0xFFFF) {
            /* Internal request to stay in hyp mode */
            printh("[hvc] enter hyp\n\r");
            context_dump_regs(regs);
            return HYP_RESULT_STAY;
        }
        /* Handle the other cases */
        switch (iss)
        case 0xFFFE:
            /* hyp_ping */
            printh("[hyp] _hyp_hvc_service:ping\n\r");
            context_dump_regs(regs);
            break;
        case 0xFFFD:
            /* hsvc_yield() */
            printh("[hyp] _hyp_hvc_service:yield\n\r");
            context_dump_regs(regs);
            context_switchto(sched_policy_determ_next());
            break;
    }
        break; /* TRAP_EC_NON_ZERO_HVC */
    case TRAP_EC_NON_ZERO_SMC:
        printh("Trapped SMC instruction: 0x%08x\n", hsr);
        break;
    case TRAP_EC_NON_ZERO_PREFETCH_ABORT_FROM_OTHER_MODE:
        printh("Prefetch Abort routed to Hyp mode: 0x%08x\n", hsr);
        break;
    case TRAP_EC_NON_ZERO_PREFETCH_ABORT_FROM_HYP_MODE:
        printh("Prefetch Abort taken from Hyp mode: 0x%08x\n", hsr);
        break;
    case TRAP_EC_NON_ZERO_DATA_ABORT_FROM_OTHER_MODE: {
        /* Handle data abort at the priority */
        printh("[hyp] data abort handler: hsr.iss=%x\n", iss);
        _trap_dump_bregs();
        if (trap_hvc_dabort(iss, regs) != HVMM_STATUS_SUCCESS) {
            printh("[hyp] === Unhandled dabort ===\n");
            printh("[hyp] current guest vmid:%d\n", context_current_vmid());
            context_dump_regs(regs);
            _trap_dump_bregs();
            hyp_abort_infinite();
            }
            _trap_dump_bregs();
        }
        break; /* TRAP_EC_NON_ZERO_DATA_ABORT_FROM_OTHER_MODE. */
    case TRAP_EC_NON_ZERO_DATA_ABORT_FROM_HYP_MODE:
        printh("Data Abort taken from Hyp mode: 0x%08x\n", hsr);
        break;
    default:
        printh("[hyp] _hyp_hvc_service:unknown hsr.iss= %x\n", iss);
        printh("[hyp] hsr.ec= %x\n", ec);
        printh("[hyp] hsr= %x\n", hsr);
        context_dump_regs(regs);
        _trap_dump_bregs();
        hyp_abort_infinite();
        break;
    } /* End of switch(ec) statement. */
    printh("[hyp] _hyp_hvc_service: done\n\r");
    context_perform_switch();
    return HYP_RESULT_ERET;
}

/* API */
struct arch_regs *trap_saved_regs(void)
{
    return _trap_hyp_saved_regs;
}
