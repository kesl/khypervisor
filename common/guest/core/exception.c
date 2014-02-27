
#define ARCH_REGS_NUM_GPR   13
struct arch_regs {
    unsigned int cpsr;
    unsigned int pc;
    unsigned int gpr[ARCH_REGS_NUM_GPR];
} __attribute((packed));


void _except_unhandled(struct arch_regs *regs)
{
}

void _except_svc(struct arch_regs *regs)
{
}

void _except_irq(struct arch_regs *regs)
{
    gic_interrupt(0, regs);
}

