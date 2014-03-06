#include <hvmm_trace.h>
#include <armv7_p15.h>
#include "trap.h"
#include "context.h"
#include "gic.h"
#include "sched_policy.h"
#include "trap_dabort.h"
#include "emulate_trapped_inst.h"
#include "trap.h"
#define DEBUG
#include <log/print.h>

/* By holding the address to the saved regs struct,
 context or other modules can access to this structure
 through trap_saved_regs() call when it's needed.
 For example, copying register values for context switching can be
 performed this way.
 */

static struct arch_regs *_trap_hyp_saved_regs;
/**
 * @brief Handler for data aboart exception
 * @param currnt registers' values
 * @return value of hvmm status
 */
hvmm_status_t _hyp_trap_dabort(struct arch_regs *regs)
{
    _trap_hyp_saved_regs = regs;
    context_dump_regs(regs);
    hyp_abort_infinite()
    ;
    return HVMM_STATUS_UNKNOWN_ERROR;
}
/**
 * @brief Handler for irq exception
 * @param currnt registers' values
 * @return value of hvmm status
 */
hvmm_status_t _hyp_trap_irq(struct arch_regs *regs)
{
    _trap_hyp_saved_regs = regs;
    gic_interrupt(0, regs);
    context_perform_switch();
    return HVMM_STATUS_SUCCESS;
}
/**
 * @brief Handler for unhandled exception
 * @param currnt registers' values
 * @return value of hvmm status
 */
hvmm_status_t _hyp_trap_unhandled(struct arch_regs *regs)
{
    _trap_hyp_saved_regs = regs;
    context_dump_regs(regs);
    hyp_abort_infinite();

    return HVMM_STATUS_UNKNOWN_ERROR;
}
/**
 * @brief check architecture registers(spsr, lr, sp) for debugging mode
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
/**
 * @brief Handler for hvc exception
 * @param currnt registers' values
 * @return value of hvmm status
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
