#include <k-hypervisor-config.h>
#include <log/print.h>
#include <hvmm_trace.h>
#include <guest.h>
#include <guest_hw.h>

/* AArch32 */
#define CPSR_MODE_USER  0x10
#define CPSR_MODE_FIQ   0x11
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SVC   0x13
#define CPSR_MODE_MON   0x16
#define CPSR_MODE_ABT   0x17
#define CPSR_MODE_HYP   0x1A
#define CPSR_MODE_UND   0x1B
#define CPSR_MODE_SYS   0x1F

/* AArch64 */
#define CPSR_MODE_EL0t  0x00
#define CPSR_MODE_EL1t  0x04
#define CPSR_MODE_EL1h  0x05
#define CPSR_MODE_EL2t  0x08
#define CPSR_MODE_EL2h  0x09
#define CPSR_MODE_EL3t  0x0C
#define CPSR_MODE_EL3h  0x0D


static void context_copy_regs(struct arch_regs *regs_dst,
                struct arch_regs *regs_src)
{
    int i;
    regs_dst->cpsr = regs_src->cpsr;
    regs_dst->pc = regs_src->pc;
    for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
        regs_dst->gpr[i] = regs_src->gpr[i];
}

/* System register state management: init/save/restore */
static void context_init_sys(struct regs_sys *regs_sys)
{
    regs_sys->vbar_el1 = 0;
    regs_sys->ttbr0_el1 = 0;
    regs_sys->ttbr1_el1 = 0;
    regs_sys->tcr_el1 = 0;
    regs_sys->sctlr_el1 = 0;
    regs_sys->sp_el0 = 0;
    regs_sys->sp_el1 = 0;
    regs_sys->elr_el1 = 0;
    regs_sys->spsel = 0;
}

static void context_save_sys(struct regs_sys *regs_sys)
{
    regs_sys->vbar_el1 = read_vbar();
    regs_sys->ttbr0_el1 = read_ttbr0();
    regs_sys->ttbr1_el1 = read_ttbr1();
    regs_sys->tcr_el1 = read_ttbcr();
    regs_sys->sctlr_el1 = read_sctlr();
    regs_sys->sp_el0 = read_sr64(sp_el0);
    regs_sys->sp_el1 = read_sr64(sp_el1);
    regs_sys->elr_el1 = read_sr64(elr_el1);
    regs_sys->spsel = read_sr32(spsel);
}

static void context_restore_sys(struct regs_sys *regs_sys)
{
    write_vbar(regs_sys->vbar_el1);
    write_ttbr0(regs_sys->ttbr0_el1);
    write_ttbr1(regs_sys->ttbr1_el1);
    write_ttbcr(regs_sys->tcr_el1);
    write_sctlr(regs_sys->sctlr_el1);
    write_sr64(regs_sys->sp_el0, sp_el0);
    write_sr64(regs_sys->sp_el1, sp_el1);
    write_sr64(regs_sys->elr_el1, elr_el1);
    write_sr32(regs_sys->spsel, spsel);
}

#ifndef DEBUG
static char *_modename(uint8_t mode)
{
    char *name = "Unknown";
    switch (mode) {
    /* AArch32 */
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
    /* AArch64 */
    case CPSR_MODE_EL0t:
        name = "EL0t";
        break;
    case CPSR_MODE_EL1t:
        name = "EL1t";
        break;
    case CPSR_MODE_EL1h:
        name = "EL1h";
        break;
    case CPSR_MODE_EL2t:
        name = "EL2t";
        break;
    case CPSR_MODE_EL2h:
        name = "EL2h";
        break;
    case CPSR_MODE_EL3t:
        name = "EL3t";
        break;
    case CPSR_MODE_EL3h:
        name = "EL3h";
        break;
  }
    return name;
}
#endif

static hvmm_status_t guest_hw_save(struct guest_struct *guest,
                struct arch_regs *current_regs)
{
    struct arch_regs *regs = &guest->regs;
    struct arch_context *context = &guest->context;

    if (!current_regs)
        return HVMM_STATUS_SUCCESS;
    guest->vmpidr_el2 = read_sr64(vmpidr_el2);

    context_copy_regs(regs, current_regs);
    context_save_sys(&context->regs_sys);
    printh("guest_hw_save  context: saving vmid[%d] mode(%x):%s pc:0x%x\n",
            _current_guest[0]->vmid,
           regs->cpsr & 0x1F,
           _modename(regs->cpsr & 0x1F),
           regs->pc);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_hw_restore(struct guest_struct *guest,
                struct arch_regs *current_regs)
{
    struct arch_context *context = &guest->context;
    write_sr64(guest->vmpidr_el2, vmpidr_el2);

    if (!current_regs) {
        /* init -> hyp mode -> guest */
        /*
         * The actual context switching (Hyp to Normal mode)
         * handled in the asm code
         */
        __mon_switch_to_guest_context(&guest->regs);
        return HVMM_STATUS_SUCCESS;
    }

    /* guest -> hyp -> guest */
    context_copy_regs(current_regs, &guest->regs);
    context_restore_sys(&context->regs_sys);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_hw_init(struct guest_struct *guest,
                struct arch_regs *regs)
{
    /*
     * todo:
     * - support multicore, configure vmpidr
     *    -> now, just suggest uniprocessor guest system.
     *
     */
    struct arch_context *context = &guest->context;
    /* vmpidr_el2 -> mpidr_el1 */
    guest->vmpidr_el2 = (0x1 << 30);

    regs->pc = CFG_GUEST_START_ADDRESS;
    /* Initialize loader status for reboot */
    regs->gpr[10] = 0;
    /* supervisor mode */
    regs->cpsr = 0x1C0|CPSR_MODE_EL1t;
    /* regs->gpr[] = whatever */
    context_init_sys(&context->regs_sys);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_hw_dump(uint8_t verbose, struct arch_regs *regs)
{
    if (verbose & GUEST_VERBOSE_LEVEL_0) {
        uart_print("cpsr: ");
        uart_print_hex32(regs->cpsr);
        uart_print("\n\r");
        uart_print("  pc: ");
        uart_print_hex64(regs->pc);
        uart_print("\n\r");
        {
            int i;
            uart_print(" gpr:\n\r");
            for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
                uart_print("     ");
                uart_print_hex64(regs->gpr[i]);
                uart_print("\n\r");
            }
        }
    }
    if (verbose & GUEST_VERBOSE_LEVEL_1) {
        uint64_t elr = 0;
        elr =read_sr64(elr_el2);
        printH("context: restoring vmid[%d] mode(%x):%s pc:0x%x elr:0x%x\n",
                _current_guest[0]->vmid,
                _current_guest[0]->regs.cpsr & 0x1F,
                _modename(_current_guest[0]->regs.cpsr & 0x1F),
                _current_guest[0]->regs.pc, elr);

    }
    if (verbose & GUEST_VERBOSE_LEVEL_2) {
        uint64_t pct = read_cntpct();
        uint32_t tval = read_cnthp_tval();
        uart_print("cntpct_el0:");
        uart_print_hex64(pct);
        uart_print("\n\r");
        uart_print("cnthp_tval_el2:");
        uart_print_hex32(tval);
        uart_print("\n\r");
    }
    return HVMM_STATUS_SUCCESS;
}

//hvmm_status_t guest_hw_move(struct arch_regs *dst, struct arch_regs *src)
hvmm_status_t guest_hw_move(struct guest_struct *dst, struct guest_struct *src)
{
    context_copy_regs(&(dst->regs), &(src->regs));
}

struct guest_ops _guest_ops = {
    .init = guest_hw_init,
    .save = guest_hw_save,
    .restore = guest_hw_restore,
    .dump = guest_hw_dump,
    .move = guest_hw_move
};

struct guest_module _guest_module = {
    .name = "K-Hypervisor Guest Module",
    .author = "Kookmin Univ.",
    .ops = &_guest_ops,
};

