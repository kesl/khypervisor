#include <hvmm_trace.h>
#include <armv7_p15.h>
#include "trap.h"
#include "gic.h"
#include <trap.h>
#include <guest.h>
#include <vdev.h>
#define DEBUG
#include <log/print.h>
/**\defgroup ARM
 * <pre> ARM registers.
 * ARM registers include 13 general purpose registers r0-r12, 1 Stack Pointer,
 * 1 Link Register (LR), 1 Program Counter (PC).
 */

/**@brief Handles data abort exception taken in Hyp mode.
 * This function uses current ARM registers to dump.
 * @param regs ARM registers for current virtual machine.
 * \ref ARM
 * @return Returns HVMM_STATUS_UNKNOWN_ERROR only.
 */
hvmm_status_t _hyp_dabort(struct arch_regs *regs)
{
    guest_dump_regs(regs);
    hyp_abort_infinite();
    return HVMM_STATUS_UNKNOWN_ERROR;
}
/**@brief Handles IRQ exception when interrupt is occured by a device.
 * This funcntion calls gic interrupt, context switch.
 * @param regs ARM registers for current virtual machine.
 * \ref ARM
 * @return Returns HVMM_STATUS_SUCCESS only.
 */
hvmm_status_t _hyp_irq(struct arch_regs *regs)
{
    gic_interrupt(0, regs);
    guest_perform_switch(regs);
    return HVMM_STATUS_SUCCESS;
}
/**@brief Handles undefined, hypervisor, prefetch abort as unhandled exception.
 * Also this function dumps state of the virtual machine registers.
 * We don't support these exceptions now.
 * @param regs ARM registers for current virtual machine.
 * \ref ARM
 * @return Returns HVMM_STATUS_UNKNOWN_ERROR only.
 */
hvmm_status_t _hyp_unhandled(struct arch_regs *regs)
{
    guest_dump_regs(regs);
    hyp_abort_infinite();
    return HVMM_STATUS_UNKNOWN_ERROR;
}
/**@brief Indirects _hyp_hvc_service function in file.
 * @param regs ARM registers for current virtual machine.
 * \ref ARM
 * @return Returns the same result of _hyp_hvc_service() function.
 */
enum hyp_hvc_result _hyp_hvc(struct arch_regs *regs)
{
    return _hyp_hvc_service(regs);
}
/**@brief Shows temporary banked registers for debugging.
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

/**@brief Handles every exceptions taken from a mode other than Hyp mode.
 * <pre> Exception class
 *     Unknown reason
 *     Trapped WFI or WFE instruction
 *     Trapped MCR or MRC access to CP15
 *     Trapped MCRR or MRRC access to CP15
 *     Trapped MCR or MRC access to CP14
 *     Trapped LDC or STC access to CP14
 *     HCPTR-trapped access to CP0-CP13
 *     Trapped MRC or VMRS access to CP10, for ID group traps
 *     Trapped BXJ instruction
 *     Trapped MRRC access to CP14
 *     Supervisor Call exception routed to Hyp mode
 *     Hypervisor Call
 *     Trapped SMC instruction
 *     Prefetch Abort routed to Hyp mode
 *     Prefetch Abort taken from Hyp mode
 *     Data Abort routed to Hyp mode
 *     Data Abort taken from Hyp mode
 *</pre>
 * @param regs ARM registers for current virtual machine.
 * \ref ARM
 * @return Returns the result of exception using HSR.
 * When result is HYP_RESULT_STAY, Current mode will be stay in hypervisor mode,
 * Otherwise it will revert to previous routine.
 */
enum hyp_hvc_result _hyp_hvc_service(struct arch_regs *regs)
{
    int32_t vdev_num = -1;
    uint32_t hsr = read_hsr();
    uint32_t ec = (hsr & HSR_EC_BIT) >> EXTRACT_EC;
    uint32_t iss = hsr & HSR_ISS_BIT;
    uint32_t far = read_hdfar();
    uint32_t fipa;
    uint32_t srt;
    struct arch_vdev_trigger_info info;
    int level = VDEV_LEVEL_LOW;

    printh("[hvc] _hyp_hvc_service: enter\n\r");
    fipa = (read_hpfar() & HPFAR_FIPA_MASK) >> HPFAR_FIPA_SHIFT;
    fipa = fipa << HPFAR_FIPA_PAGE_SHIFT;
    fipa = fipa | (far & HPFAR_FIPA_PAGE_MASK);
    info.ec = ec;
    info.iss = iss;
    info.fipa = fipa;
    info.sas = (iss & ISS_SAS_MASK) >> ISS_SAS_SHIFT;
    srt = (iss & ISS_SRT_MASK) >> ISS_SRT_SHIFT;
    info.value = &(regs->gpr[srt]);

    switch (ec) {
    case TRAP_EC_ZERO_UNKNOWN:
    case TRAP_EC_ZERO_WFI_WFE:
    case TRAP_EC_ZERO_MCR_MRC_CP15:
    case TRAP_EC_ZERO_MCRR_MRRC_CP15:
    case TRAP_EC_ZERO_MCR_MRC_CP14:
    case TRAP_EC_ZERO_LDC_STC_CP14:
    case TRAP_EC_ZERO_HCRTR_CP0_CP13:
    case TRAP_EC_ZERO_MRC_VMRS_CP10:
    case TRAP_EC_ZERO_BXJ:
    case TRAP_EC_ZERO_MRRC_CP14:
    case TRAP_EC_NON_ZERO_SVC:
    case TRAP_EC_NON_ZERO_SMC:
    case TRAP_EC_NON_ZERO_PREFETCH_ABORT_FROM_OTHER_MODE:
    case TRAP_EC_NON_ZERO_PREFETCH_ABORT_FROM_HYP_MODE:
    case TRAP_EC_NON_ZERO_DATA_ABORT_FROM_HYP_MODE:
        level = VDEV_LEVEL_HIGH;
        break;
    case TRAP_EC_NON_ZERO_HVC:
        level = VDEV_LEVEL_MIDDLE;
        break;
    case TRAP_EC_NON_ZERO_DATA_ABORT_FROM_OTHER_MODE:
        level = VDEV_LEVEL_LOW;
        break;
    default:
        printh("[hyp] _hyp_hvc_service:unknown hsr.iss= %x\n", iss);
        printh("[hyp] hsr.ec= %x\n", ec);
        printh("[hyp] hsr= %x\n", hsr);
        guest_dump_regs(regs);
        goto trap_error;
    }

    vdev_num = vdev_find(level, &info, regs);
    if (vdev_num < 0) {
        printh("[hvc] cann't search vdev number\n\r");
        goto trap_error;
    }

    if (iss & ISS_WNR) {
        if (vdev_write(level, vdev_num, &info, regs) < 0)
            goto trap_error;
    } else {
        if (vdev_read(level, vdev_num, &info, regs) < 0)
            goto trap_error;
    }
    vdev_post(level, vdev_num, &info, regs);

    printh("[hyp] _hyp_hvc_service: done\n\r");
    guest_perform_switch(regs);
    return HYP_RESULT_ERET;
trap_error:
    _trap_dump_bregs();
    hyp_abort_infinite();
}
