
#define ARCH_REGS_NUM_GPR   31
struct arch_regs {
    unsigned int cpsr;
    unsigned long pc;
    unsigned long gpr[ARCH_REGS_NUM_GPR];
};

static void _reboot()
{ unsigned int *reboot = (unsigned int *) 0x17000014;
    *reboot = 0x1;
}

void _except_unhandled(struct arch_regs *regs)
{
    _reboot();
}

void _except_svc(struct arch_regs *regs)
{
    _reboot();
}

void _except_irq(struct arch_regs *regs)
{
    gic_interrupt(0, regs);
}

