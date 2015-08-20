#include <hvmm_trace.h>
#include <armv8_processor.h>
#include <gic.h>
#include <trap.h>
#include <guest.h>
#include <vdev.h>
#include <smp.h>

#define DEBUG
#include <log/print.h>
#include <interrupt.h>

static const char *ext_level[] = {
    "GUEST_64",
    "GUEST_32",
    "EL2T",
    "EL2H"
};

static const char *mode[] = {
   "synconous",
   "IRQ",
   "FIQ",
   "Serror"
};

/**\defgroup ARM
 * <pre> ARM registers.
 * ARM registers include 13 general purpose registers r0-r12, 1 Stack Pointer,
 * 1 Link Register (LR), 1 Program Counter (PC).
 */

/**@brief Handles IRQ exception when interrupt is occured by a device.
 * This funcntion calls gic interrupt, context switch.
 * @param regs ARM registers for current virtual machine.
 * \ref ARM
 * @return Returns HVMM_STATUS_SUCCESS only.
 */
hvmm_status_t _irq(struct arch_regs *regs, uint32_t el)
{
    uint32_t irq;
#if 0
    // for debug
    uint32_t cpsr = read_sr32(spsr_el1);
    uint32_t currentel = read_sr32(currentel);
    uint32_t daif = read_sr32(daif);

    printh("[%s]IRQ\n\r", ext_level[el]);
    printh("cspr: %x\n", cpsr);
    printh("currentEL:%x\n", currentel);
    printh("daif:%x\n", daif);
#endif
    irq = gic_get_irq_number();
    interrupt_service_routine(irq, (void *)regs, 0);
    guest_perform_switch(regs);
    return HVMM_STATUS_SUCCESS;
}

/**@brief Handles undefined, hypervisor, prefetch abort as unhandled exception.
 * Also this function dumps state of the virtual machine registers, We don't
 * support these exceptions now.
 * @param regs ARM registers for current virtual machine.
 * \ref ARM
 * @return Returns HVMM_STATUS_UNKNOWN_ERROR only.
 */
hvmm_status_t _unhandled(struct arch_regs *regs, uint32_t el, uint32_t md)
{
    printH("[%s] %s is unhandled!\n\r", ext_level[el], mode[md]);
    guest_dump_regs(regs);
    hyp_abort_infinite();
    return HVMM_STATUS_UNKNOWN_ERROR;
}

/**@brief Indirects _hyp_sync_service function in file.
 * @param regs ARM registers for current virtual machine.
 * \ref ARM
 * @return Returns the result is the same as _hyp_sync_service().
 * @todo Within the near future, this function will be deleted.
 */
enum hyp_hvc_result _sync(struct arch_regs *regs, uint32_t el)
{
    printh("[%s]sync!\n\r", ext_level[el]);
    return _hyp_sync_service(regs);
}

/**@brief Shows temporary banked registers for debugging.
 */
static void _trap_dump_bregs(void)
{
    uint32_t spsr, lr, sp;

    printh(" - banked regs\n");
    asm volatile(" mrs     %0, sp_el0\n\t" : "=r"(sp) : : "memory", "cc");
    printh(" - el0: sp:%x\n", sp);
    asm volatile(" mrs     %0, spsr_el1\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_el1\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, elr_el1\n\t" : "=r"(lr) : : "memory", "cc");
    printh(" - el1: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
    asm volatile(" mrs     %0, spsr_irq\n\t" : "=r"(spsr) : : "memory", "cc");
    printh(" - irq: spsr:%x\n", spsr);
}

/*
 * sync #imm handler
 *
 * HYP Syndrome Register(HSR)
 * EC[31:26] is an exception class for the exception that is taken to HYP mode
 * IL[25] is an instruction length for the trapped insruction that is
 * 16 bit or 32 bit
 * ISS[24:0] is an instruction-specific syndrome for the instruction
 * information included. It depends on EC field.
 * END OF HSR DESCRIPTION FROM ARM DDI0406_C ARCHITECTURE MANUAL
 */

/**@brief Handles every exceptions taken from a mode other than sync mode.
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
 * @return Returns the result of exceptions.
 * If hypervisor can b handled the exception then it returns HYP_RESULT_ERET.
 * If not, hypervisor should be stopped into trap_error in handler.
 */
enum hyp_hvc_result _hyp_sync_service(struct arch_regs *regs)
{
    /*
     * todo
     * - fit in armv8 - separate hvc, abort and other exception
     */
    int32_t vdev_num = -1;
    uint32_t esr = read_esr();
    uint32_t ec = (esr & ESR_EC_BIT) >> EXTRACT_EC;
    uint32_t iss = esr & ESR_ISS_BIT;
    uint64_t far = read_hdfar();
    uint64_t fipa;
    uint32_t srt;
    struct arch_vdev_trigger_info info;
    int level = VDEV_LEVEL_LOW;

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
    case TRAP_EC_UNKNOWN:
    case TRAP_EC_WFI_WFE:
    case TRAP_EC_MCR_MRC_CP15:
    case TRAP_EC_MCRR_MRRC_CP15:
    case TRAP_EC_MCR_MRC_CP14:
    case TRAP_EC_LDC_STC_CP14:
    case TRAP_EC_SIMD_FP:
    case TRAP_EC_MCR_MRC_CP10:
    case TRAP_EC_MRRC_CP14:

    case TRAP_EC_SVC:
    case TRAP_EC_SMC:
    case TRAP_EC_PREFETCH_ABORT_FROM_OTHER_MODE:
    case TRAP_EC_PREFETCH_ABORT_FROM_HYP_MODE:
    case TRAP_EC_DATA_ABORT_FROM_HYP_MODE:
        level = VDEV_LEVEL_HIGH;
        break;
    case TRAP_EC_HVC_FROM_AARCH32:
    case TRAP_EC_HVC_FROM_AARCH64:
        level = VDEV_LEVEL_MIDDLE;
        break;
    case TRAP_EC_DATA_ABORT_FROM_OTHER_MODE:
        level = VDEV_LEVEL_LOW;
        break;
    default:
        printH("[hyp] _hyp_sync_service:unknown esr.iss= %x\n", iss);
        printH("[hyp] esr.ec= %x\n", ec);
        printH("[hyp] esr= %x\n", esr);
        guest_dump_regs(regs);
        goto trap_error;
    }

    vdev_num = vdev_find(level, &info, regs);
    if (vdev_num < 0) {
        printH("[hvc] cann't search vdev number\n\r");
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

    guest_perform_switch(regs);

    return HYP_RESULT_ERET;
trap_error:
    switch (ec) {
        case TRAP_EC_PREFETCH_ABORT_FROM_OTHER_MODE:
        case TRAP_EC_DATA_ABORT_FROM_OTHER_MODE:
            {
                uint32_t esr_el1 = read_sr32(esr_el1);
                uint64_t vttbr = read_sr64(vttbr_el2);
                uint64_t far = read_sr64(far_el1);
                uint64_t hpfar = read_sr64(hpfar_el2);
                printH("esr_el1: %x\n", esr_el1);
                printH("vttbr_el2 :");
                uart_print_hex64(vttbr);
                printH("\n");
                printH("far_el1:");
                uart_print_hex64(far);
                printH("\n");
                printH("hpfar_el2:");
                uart_print_hex64(hpfar);
                printH("\n");
            }
            break;
        case TRAP_EC_PREFETCH_ABORT_FROM_HYP_MODE:
        case TRAP_EC_DATA_ABORT_FROM_HYP_MODE:
            {
            uint64_t ttbr = read_sr64(ttbr0_el2);
            printH("ttbr_el2 :");
            uart_print_hex64(ttbr);
            printH("\n");
            }
            break;
    }
    _trap_dump_bregs();
    printH("ESR_EL2:%x\n", esr);
    printH("FAR_EL2:");
    uart_print_hex64(far);
    printH("\n");

    printH("fipa is ");
    uart_print_hex64(fipa);
    printH("\n");
    printH("guest pc is ");
    uart_print_hex64(regs->pc);
    printH("\n");
    hyp_abort_infinite();

    return HYP_RESULT_STAY;
}
