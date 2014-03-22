#include <k-hypervisor-config.h>
#include <test/tests.h>
#include <version.h>
#include <log/print.h>
#include <hvmm_trace.h>
#include <timer.h>
#include <mm.h>
#include <interrupt.h>
#include <vdev.h>
#include <virq.h>
#include <guest.h>
#include <guest_hw.h>
#include <vmm.h>

#define CPSR_MODE_USER  0x10
#define CPSR_MODE_FIQ   0x11
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SVC   0x13
#define CPSR_MODE_MON   0x16
#define CPSR_MODE_ABT   0x17
#define CPSR_MODE_HYP   0x1A
#define CPSR_MODE_UND   0x1B
#define CPSR_MODE_SYS   0x1F

static void context_copy_regs(struct arch_regs *regs_dst,
                struct arch_regs *regs_src)
{
    int i;
    regs_dst->cpsr = regs_src->cpsr;
    regs_dst->pc = regs_src->pc;
    regs_dst->lr = regs_src->lr;
    for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
        regs_dst->gpr[i] = regs_src->gpr[i];
}

/* banked registers */
static void context_init_banked(struct regs_banked *regs_banked)
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

static void context_save_banked(struct regs_banked *regs_banked)
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

static void context_restore_banked(struct regs_banked *regs_banked)
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

/* Co-processor state management: init/save/restore */
static void context_init_cops(struct regs_cop *regs_cop)
{
    regs_cop->vbar = 0;
    regs_cop->ttbr0 = 0;
    regs_cop->ttbr1 = 0;
    regs_cop->ttbcr = 0;
    regs_cop->sctlr = 0;
}

static void context_save_cops(struct regs_cop *regs_cop)
{
    regs_cop->vbar = read_vbar();
    regs_cop->ttbr0 = read_ttbr0();
    regs_cop->ttbr1 = read_ttbr1();
    regs_cop->ttbcr = read_ttbcr();
    regs_cop->sctlr = read_sctlr();
}

static void context_restore_cops(struct regs_cop *regs_cop)
{
    write_vbar(regs_cop->vbar);
    write_ttbr0(regs_cop->ttbr0);
    write_ttbr1(regs_cop->ttbr1);
    write_ttbcr(regs_cop->ttbcr);
    write_sctlr(regs_cop->sctlr);
}

#ifdef DEBUG
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

static hvmm_status_t guest_save(struct guest_struct *guest,
                struct arch_regs *regs_current)
{
    struct arch_regs *regs = &guest->regs;
    struct arch_context *context = &guest->context;

    context_copy_regs(regs, regs_current);
    context_save_cops(&context->regs_cop);
    context_save_banked(&context->regs_banked);
    vgic_save_status(&context->vgic_status, guest->vmid);
    printh("context: saving vmid[%d] mode(%x):%s pc:0x%x\n",
            _current_guest->vmid,
           regs->cpsr & 0x1F,
           _modename(regs->cpsr & 0x1F),
           regs->pc);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_restore(struct guest_struct *guest,
                struct arch_regs *regs_current)
{
    struct arch_context *context = &guest->context;

    vgic_restore_status(&context->vgic_status, guest->vmid);
    context_copy_regs(regs_current, &guest->regs);
    context_restore_cops(&context->regs_cop);
    context_restore_banked(&context->regs_banked);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_init(struct guest_struct *guest,
                struct arch_regs *regs)
{
    struct arch_context *context = &guest->context;

    regs->pc = 0x80000000;
    /* supervisor mode */
    regs->cpsr = 0x1d3;
    /* regs->gpr[] = whatever */
    context->ttbl = vmm_vmid_ttbl(guest->vmid);
    context_init_cops(&context->regs_cop);
    context_init_banked(&context->regs_banked);
    vgic_init_status(&context->vgic_status, guest->vmid);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t guest_dump(uint8_t verbose, struct arch_regs *regs)
{
    if (verbose & GUEST_VERBOSE_LEVEL_0) {
        uart_print("cpsr: ");
        uart_print_hex32(regs->cpsr);
        uart_print("\n\r");
        uart_print("  pc: ");
        uart_print_hex32(regs->pc);
        uart_print("\n\r");
        uart_print("  lr: ");
        uart_print_hex32(regs->lr);
        uart_print("\n\r");
        {
            int i;
            uart_print(" gpr:\n\r");
            for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
                uart_print("     ");
                uart_print_hex32(regs->gpr[i]);
                uart_print("\n\r");
            }
        }
    }
    if (verbose & GUEST_VERBOSE_LEVEL_1) {
        uint32_t lr = 0;
        asm volatile("mov  %0, lr" : "=r"(lr) : : "memory", "cc");
        printh("context: restoring vmid[%d] mode(%x):%s pc:0x%x lr:0x%x\n",
                _current_guest->vmid,
                _current_guest->regs.cpsr & 0x1F,
                _modename(_current_guest->regs.cpsr & 0x1F),
                _current_guest->regs.pc, lr);

    }
    if (verbose & GUEST_VERBOSE_LEVEL_2) {
        uint64_t pct = read_cntpct();
        uint32_t tval = read_cnthp_tval();
        uart_print("cntpct:");
        uart_print_hex64(pct);
        uart_print("\n\r");
        uart_print("cnth_tval:");
        uart_print_hex32(tval);
        uart_print("\n\r");
    }
    return HVMM_STATUS_SUCCESS;
}

struct guest_ops _guest_ops = {
    .init = guest_init,
    .save = guest_save,
    .restore = guest_restore,
    .dump = guest_dump,
};

struct guest_module _guest_module = {
    .name = "K-Hypervisor Guest Module",
    .author = "Kookmin Univ.",
    .ops = &_guest_ops,
};

