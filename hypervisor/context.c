#include <armv7_p15.h>
#include <arch_types.h>
#include <k-hypervisor-config.h>
#include <mm.h>
#include <gic.h>
#include <interrupt.h>
#include <context.h>
#include <hvmm_trace.h>
#include <vdev.h>
#include <vdev/vdev_gicd.h>
#include <gic_regs.h>
#include <virqmap.h>
#include <trap.h>
#include <vmm.h>

#include <string.h>
#include <config/cfg_platform.h>

#include <log/uart_print.h>
#include <log/print.h>
#include <test/tests.h>

#include <log/uart_print.h>
#include <log/print.h>
#include <version.h>

#define NUM_GUEST_CONTEXTS        NUM_GUESTS_STATIC

/** \defgroup CPSR_Operating_modes
 * @brief ARM Operating modes
 *
 * - User mode is the usual ARM program execution state,
 *   and is used for executing most application programs.
 * - Fast interrupt (FIQ) mod is used for handling fast interrupts.
 * - Interrupt (IRQ) mode is used for general-purpose interrupt handling.
 * - Supervisor mode is a protected mode for the operating system.
 * - Abort mode is entered after a data or instruction Prefetch Abort.
 * - System mode is a privileged user mode for the operating system.
 * - Undefined mode is entered when an Undefined instruction exception occurs.
 *
 * <pre>
 * Processor  mode  Encoding Privilege Implemented               Security state
 * User       usr   10000    PL0       Always                        Both
 * FIQ        fiq   10001    PL1       Always                        Both
 * IRQ        irq   10010    PL1       Always                        Both
 * Supervisor svc   10011    PL1       Always                        Both
 * Monitor    mon   10110    PL1       With Security Extension      Secure only
 * Abort      abt   10111    PL1       Always                        Both
 * Hyp        hyp   11010    PL2       With Virtualization Extension Non-secure
 * Undefined  und   11011    PL1       Always                        Both
 * System     sys   11111    PL1       Always                        Both
 * </pre>
 * @{
 */
#define CPSR_MODE_USER  0x10
#define CPSR_MODE_FIQ   0x11
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SVC   0x13
#define CPSR_MODE_MON   0x16
#define CPSR_MODE_ABT   0x17
#define CPSR_MODE_HYP   0x1A
#define CPSR_MODE_UND   0x1B
#define CPSR_MODE_SYS   0x1F
/** @} */

#define __CONTEXT_TRACE_VERBOSE__
/**
 * @brief Checks vmid is whether valid or invalid.
 * 
 * Verify that vmid is in range.
 * - first_vmid <= vmid <= last_vmid
 *
 * @return Result of verification.
 */
#define _valid_vmid(vmid) \
	(vmid >= context_first_vmid() && vmid <= context_last_vmid())

static struct hyp_guest_context guest_contexts[NUM_GUEST_CONTEXTS];
static int _current_guest_vmid = VMID_INVALID;
static int _next_guest_vmid = VMID_INVALID;
/* further switch request will be ignored if set */
static uint8_t _switch_locked;

#ifdef DEBUG
/**
 * @brief Returns the name of operating mode of ARM.
 *
 * Checks the parameter and selects the operating mode name.
 * - \ref CPSR_Operating_modes "Operating mode description".
 *
 * @param mode Operating mode value.
 * @return Mode Operating mode name.
 */
static char *_modename(uint8_t mode)
{
    char *name = "Unknown";
    switch (mode) {
    case CPSR_MODE_USER:
        name = "User";
        break;
    case CPSR_MODE_FIQ:
        name = "FIQ";
        break;
    case CPSR_MODE_IRQ:
        name = "IRQ";
        break;
    case CPSR_MODE_SVC:
        name = "Supervisor";
        break;
    case CPSR_MODE_MON:
        name = "Monitor";
        break;
    case CPSR_MODE_ABT:
        name = "Abort";
        break;
    case CPSR_MODE_HYP:
        name = "Hyp";
        break;
    case CPSR_MODE_UND:
        name = "Undefined";
        break;
    case CPSR_MODE_SYS:
        name = "System";
        break;
    }
    return name;
}
#endif
void context_dump_regs(struct arch_regs *regs)
{
#ifdef DEBUG
    uart_print("cpsr: ");
    uart_print_hex32(regs->cpsr);
    uart_print("\n\r");
    uart_print("  pc: ");
    uart_print_hex32(regs->pc);
    uart_print("\n\r");
    uart_print("  lr: ");
    uart_print_hex32(regs->lr);
    uart_print("\n\r");
#ifdef __CONTEXT_TRACE_VERBOSE__
    {
        int i;
        uart_print(" gpr:\n\r");
        for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
            uart_print("     ");
            uart_print_hex32(regs->gpr[i]);
            uart_print("\n\r");
        }
    }
#endif
#endif
}
/**
 * @brief Copys the source register to destination register.
 *
 * Copy Architecture registers
 * - R0-R12
 * - LR
 * - PC
 * - CPSR
 *
 * @param regs_dst Destination register.
 * @param regs_src Source register.
 * @return void
 */
static void context_copy_regs(struct arch_regs *regs_dst,
                struct arch_regs *regs_src)
{
    int i;
    for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
        regs_dst->gpr[i] = regs_src->gpr[i];
    regs_dst->lr = regs_src->lr;
    regs_dst->pc = regs_src->pc;
    regs_dst->cpsr = regs_src->cpsr;
}

/**
 * @brief Initialize the banked registers.
 *
 * Initialize the banked registers of all mode
 *
 * @param regs_banked Target banked registers
 * @return void
 */
void context_init_banked(struct arch_regs_banked *regs_banked)
{
    regs_banked->sp_usr = 0;
    regs_banked->spsr_svc = 0;
    regs_banked->sp_svc = 0;
    regs_banked->lr_svc = 0;
    regs_banked->spsr_abt = 0;
    regs_banked->sp_abt = 0;
    regs_banked->lr_abt = 0;
    regs_banked->spsr_und = 0;
    regs_banked->sp_und = 0;
    regs_banked->lr_und = 0;
    regs_banked->spsr_irq = 0;
    regs_banked->sp_irq = 0;
    regs_banked->lr_irq = 0;
    regs_banked->spsr_fiq = 0;
    regs_banked->lr_fiq = 0;
    regs_banked->r8_fiq = 0;
    regs_banked->r9_fiq = 0;
    regs_banked->r10_fiq = 0;
    regs_banked->r11_fiq = 0;
    regs_banked->r12_fiq = 0;
    /* Cortex-A15 processor does not support sp_fiq */
}

/**
 * @brief Copy currently running banked registers to storage.
 *
 * Copy the registers to the storage variable.
 * @param regs_banked Target structure
 * @return void
 */
void context_save_banked(struct arch_regs_banked *regs_banked)
{
    /* USR banked register */
    asm volatile(" mrs     %0, sp_usr\n\t"
                 : "=r"(regs_banked->sp_usr) : : "memory", "cc");
    /* SVC banked register */
    asm volatile(" mrs     %0, spsr_svc\n\t"
                 : "=r"(regs_banked->spsr_svc) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t"
                 : "=r"(regs_banked->sp_svc) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_svc\n\t"
                 : "=r"(regs_banked->lr_svc) : : "memory", "cc");
    /* ABT banked register */
    asm volatile(" mrs     %0, spsr_abt\n\t"
                 : "=r"(regs_banked->spsr_abt) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_abt\n\t"
                 : "=r"(regs_banked->sp_abt) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_abt\n\t"
                 : "=r"(regs_banked->lr_abt) : : "memory", "cc");
    /* UND banked register */
    asm volatile(" mrs     %0, spsr_und\n\t"
                 : "=r"(regs_banked->spsr_und) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_und\n\t"
                 : "=r"(regs_banked->sp_und) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_und\n\t"
                 : "=r"(regs_banked->lr_und) : : "memory", "cc");
    /* IRQ banked register */
    asm volatile(" mrs     %0, spsr_irq\n\t"
                 : "=r"(regs_banked->spsr_irq) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_irq\n\t"
                 : "=r"(regs_banked->sp_irq) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_irq\n\t"
                 : "=r"(regs_banked->lr_irq) : : "memory", "cc");
    /* FIQ banked register  R8_fiq ~ R12_fiq, LR and SPSR */
    asm volatile(" mrs     %0, spsr_fiq\n\t"
                 : "=r"(regs_banked->spsr_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_fiq\n\t"
                 : "=r"(regs_banked->lr_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r8_fiq\n\t"
                 : "=r"(regs_banked->r8_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r9_fiq\n\t"
                 : "=r"(regs_banked->r9_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r10_fiq\n\t"
                 : "=r"(regs_banked->r10_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r11_fiq\n\t"
                 : "=r"(regs_banked->r11_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r12_fiq\n\t"
                 : "=r"(regs_banked->r12_fiq) : : "memory", "cc");
}

/**
 * @brief Restore the banked registers from storage varible.
 * to the currently running banked registers.
 *
 * @param regs_banked Source storage variable.
 * @return void
 */
void context_restore_banked(struct arch_regs_banked *regs_banked)
{
    /* USR banked register */
    asm volatile(" msr    sp_usr, %0\n\t"
                 : : "r"(regs_banked->sp_usr) : "memory", "cc");
    /* SVC banked register */
    asm volatile(" msr    spsr_svc, %0\n\t"
                 : : "r"(regs_banked->spsr_svc) : "memory", "cc");
    asm volatile(" msr    sp_svc, %0\n\t"
                 : : "r"(regs_banked->sp_svc) : "memory", "cc");
    asm volatile(" msr    lr_svc, %0\n\t"
                 : : "r"(regs_banked->lr_svc) : "memory", "cc");
    /* ABT banked register */
    asm volatile(" msr    spsr_abt, %0\n\t"
                 : : "r"(regs_banked->spsr_abt) : "memory", "cc");
    asm volatile(" msr    sp_abt, %0\n\t"
                 : : "r"(regs_banked->sp_abt) : "memory", "cc");
    asm volatile(" msr    lr_abt, %0\n\t"
                 : : "r"(regs_banked->lr_abt) : "memory", "cc");
    /* UND banked register */
    asm volatile(" msr    spsr_und, %0\n\t"
                 : : "r"(regs_banked->spsr_und) : "memory", "cc");
    asm volatile(" msr    sp_und, %0\n\t"
                 : : "r"(regs_banked->sp_und) : "memory", "cc");
    asm volatile(" msr    lr_und, %0\n\t"
                 : : "r"(regs_banked->lr_und) : "memory", "cc");
    /* IRQ banked register */
    asm volatile(" msr     spsr_irq, %0\n\t"
                 : : "r"(regs_banked->spsr_irq) : "memory", "cc");
    asm volatile(" msr     sp_irq, %0\n\t"
                 : : "r"(regs_banked->sp_irq) : "memory", "cc");
    asm volatile(" msr     lr_irq, %0\n\t"
                 : : "r"(regs_banked->lr_irq) : "memory", "cc");
    /* FIQ banked register */
    asm volatile(" msr     spsr_fiq, %0\n\t"
                 : : "r"(regs_banked->spsr_fiq) : "memory", "cc");
    asm volatile(" msr     lr_fiq, %0\n\t"
                 : : "r"(regs_banked->lr_fiq) : "memory", "cc");
    asm volatile(" msr    r8_fiq, %0\n\t"
                 : : "r"(regs_banked->r8_fiq) : "memory", "cc");
    asm volatile(" msr    r9_fiq, %0\n\t"
                 : : "r"(regs_banked->r9_fiq) : "memory", "cc");
    asm volatile(" msr    r10_fiq, %0\n\t"
                 : : "r"(regs_banked->r10_fiq) : "memory", "cc");
    asm volatile(" msr    r11_fiq, %0\n\t"
                 : : "r"(regs_banked->r11_fiq) : "memory", "cc");
    asm volatile(" msr    r12_fiq, %0\n\t"
                 : : "r"(regs_banked->r12_fiq) : "memory", "cc");
}

/**
 * @brief Initialize registers which access via the coprocessor.
 *
 * Initialize context registers which access via the coprocessor to zero.
 * - VBAR
 * - TTBR0
 * - TTBR1
 * - TTBCR
 * - SCTLR
 *
 * @param regs_cop Initialize target registers.
 * @return void
 */
void context_init_cops(struct arch_regs_cop *regs_cop)
{
    regs_cop->vbar = 0;
    regs_cop->ttbr0 = 0;
    regs_cop->ttbr1 = 0;
    regs_cop->ttbcr = 0;
    regs_cop->sctlr = 0;
}

/**
 * @brief Stores the currently running registers which access via coprocessor.
 *
 * Stores the currently running context registers which access via coprocessor.
 *
 * @param regs_cop Target storage variable.
 * @return void
 */
void context_save_cops(struct arch_regs_cop *regs_cop)
{
    regs_cop->vbar = read_vbar();
    regs_cop->ttbr0 = read_ttbr0();
    regs_cop->ttbr1 = read_ttbr1();
    regs_cop->ttbcr = read_ttbcr();
    regs_cop->sctlr = read_sctlr();
}

/**
 * @brief Restoring the context of the registers which access via coprocessor.
 *
 * Restores the guest context of the registers which access via coprocessor.
 *
 * @param regs_cop Source storage variable
 * @return void
 */
void context_restore_cops(struct arch_regs_cop *regs_cop)
{
    write_vbar(regs_cop->vbar);
    write_ttbr0(regs_cop->ttbr0);
    write_ttbr1(regs_cop->ttbr1);
    write_ttbcr(regs_cop->ttbcr);
    write_sctlr(regs_cop->sctlr);
}


/* DEPRECATED: use context_switchto(vmid) and context_perform_switch()
   void context_switch_to_next_guest(struct arch_regs *regs_current)
 */

/**
 * @brief Switchs the current context to the next guest context.
 *
 * Saves the current running context of guest before restore the next context.
 * And appling context of the next guest.
 * - Detailed sequence
 *  - Disable stage-2 translation. HCR.VM = 0
 *  - Save the context of current current.
 *  - Restores the translation table for the next guest.
 *  - Enable Stage 2 Translation.
 *  - The next guest becomes the current
 *
 * @param regs_current The current register.
 * @param next_vmid The vmid of next guest.
 * @return Switching result.<br>
 *         If context switch is succseed, return HVMM_STATUS_SUCCESS.<br>
 *         Else context switch is failed, return HVMM_STATUS_UNKNOWN_ERROR.
 */
static hvmm_status_t context_perform_switch_to_guest_regs(
                        struct arch_regs *regs_current, vmid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    struct hyp_guest_context *context = 0;
    struct arch_regs *regs = 0;
    HVMM_TRACE_ENTER();
    if (_current_guest_vmid == next_vmid) {
        /* the same guest? WTF? */
        return HVMM_STATUS_IGNORED;
    }
    /*
     * We assume VTCR has been configured and initialized
     * in the memory management module
     */
    /* Disable Stage 2 Translation: HCR.VM = 0 */
    vmm_stage2_enable(0);
    if (regs_current != 0) {
        /* save the current guest's context */
        context = &guest_contexts[_current_guest_vmid];
        regs = &context->regs;
        context_copy_regs(regs, regs_current);
        context_save_cops(&context->regs_cop);
        context_save_banked(&context->regs_banked);
        vgic_save_status(&context->vgic_status, context->vmid);
        printh("context: saving vmid[%d] mode(%x):%s pc:0x%x\n",
               _current_guest_vmid,
               regs->cpsr & 0x1F,
               _modename(regs->cpsr & 0x1F),
               regs->pc);
    }
    /* The context of the next guest */
    context = &guest_contexts[next_vmid];
    /*
     * Restore Translation Table for the next guest and
     * Enable Stage 2 Translation
     */
    vmm_set_vmid_ttbl(context->vmid, context->ttbl);
    vmm_stage2_enable(1);
    vgic_restore_status(&context->vgic_status, context->vmid);
    {
        uint32_t lr = 0;
        asm volatile("mov  %0, lr" : "=r"(lr) : : "memory", "cc");
        printh("context: restoring vmid[%d] mode(%x):%s pc:0x%x lr:0x%x\n",
               next_vmid,
               context->regs.cpsr & 0x1F,
               _modename(context->regs.cpsr & 0x1F),
               context->regs.pc, lr);
    }
    /* The next becomes the current */
    _current_guest_vmid = next_vmid;
    if (regs_current == 0) {
        /* init -> hyp mode -> guest */
        /*
         * The actual context switch (Hyp to Normal mode)
         * handled in the asm code
         */
        __mon_switch_to_guest_context(&context->regs);
    } else {
        /* guest -> hyp -> guest */
        context_copy_regs(regs_current, &context->regs);
        context_restore_cops(&context->regs_cop);
        context_restore_banked(&context->regs_banked);
    }
    result = HVMM_STATUS_SUCCESS;
    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t context_perform_switch(void)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    if (_current_guest_vmid == VMID_INVALID) {
        printh("context: launching the first guest\n");
        /* very first time, to the default first guest */
        result = context_perform_switch_to_guest_regs(0, _next_guest_vmid);
        /* DOES NOT COME BACK HERE */
    } else if (_next_guest_vmid != VMID_INVALID &&
                _current_guest_vmid != _next_guest_vmid) {
        struct arch_regs *regs = trap_saved_regs();
        if ((regs->cpsr & 0x1F) != 0x1A) {
            printh("curr: %x\n", _current_guest_vmid);
            printh("next: %x\n", _next_guest_vmid);
            /* Only if not from Hyp */
            result = context_perform_switch_to_guest_regs(regs,
                            _next_guest_vmid);
            _next_guest_vmid = VMID_INVALID;
        }
    } else {
        /*
         * Staying at the currently active guest.
         * Flush out queued virqs since we didn't have a chance
         * to switch the context, where virq flush takes place,
         * this time
         */
        vgic_flush_virqs(_current_guest_vmid);
    }
    _switch_locked = 0;
    return result;
}

void context_switch_to_initial_guest(void)
{
    struct hyp_guest_context *context = 0;
    struct arch_regs *regs = 0;
    uart_print("[hyp] switch_to_initial_guest:\n\r");
    /* Select the first guest context to switch to. */
    _current_guest_vmid = VMID_INVALID;
    context = &guest_contexts[0];
    /* Dump the initial register values of the guest for debugging purpose */
    regs = &context->regs;
    context_dump_regs(regs);
    /* Context Switch with current context == none */
    context_switchto(0);
    context_perform_switch();
}

void context_init_guests(void)
{
    struct hyp_guest_context *context;
    struct arch_regs *regs = 0;
    int i;
    uart_print("[hyp] init_guests: enter\n\r");
    /* Initializes 2 guests */
    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        /* Guest i @guest_bin_start */
        context = &guest_contexts[i];
        regs = &context->regs;
        regs->pc = 0x80000000;
        /* supervisor mode */
        regs->cpsr = 0x1d3;
        /* regs->gpr[] = whatever */
        context->vmid = i;
        context->ttbl = vmm_vmid_ttbl(context->vmid);
        context_init_cops(&context->regs_cop);
        context_init_banked(&context->regs_banked);
        vgic_init_status(&context->vgic_status, context->vmid);
    }
    uart_print("[hyp] init_guests: return\n\r");
}

struct hyp_guest_context *context_atvmid(vmid_t vmid)
{
    struct hyp_guest_context *result = 0;

    if (vmid < NUM_GUEST_CONTEXTS)
        result = &guest_contexts[vmid];

    return result;
}


vmid_t context_first_vmid(void)
{
    /* FIXME:Hardcoded for now */
    return 0;
}

vmid_t context_last_vmid(void)
{
    /* FIXME:Hardcoded for now */
    return 1;
}

vmid_t context_next_vmid(vmid_t ofvmid)
{
    vmid_t next = VMID_INVALID;
    if (ofvmid == VMID_INVALID)
        next = context_first_vmid();
    else if (ofvmid < context_last_vmid()) {
        /* FIXME:Hardcoded */
        next = ofvmid + 1;
    }
    return next;
}

vmid_t context_current_vmid(void)
{
    return _current_guest_vmid;
}

vmid_t context_waiting_vmid(void)
{
    return _next_guest_vmid;
}

hvmm_status_t context_switchto(vmid_t vmid)
{
    return context_switchto_lock(vmid, 0);
}

hvmm_status_t context_switchto_lock(vmid_t vmid, uint8_t locked)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    HVMM_TRACE_ENTER();
    /* valid and not current vmid, switch */
    if (_switch_locked == 0) {
        if (!_valid_vmid(vmid))
            result = HVMM_STATUS_BAD_ACCESS;
        else {
            _next_guest_vmid = vmid;
            result = HVMM_STATUS_SUCCESS;
            printh("switching to vmid: %x\n", (uint32_t)vmid);
        }
    } else
        printh("context: next vmid locked to %d\n", _next_guest_vmid);

    if (locked)
        _switch_locked = locked;

    HVMM_TRACE_EXIT();
    return result;
}

