/* have to move */
/* Access system register */
#define read_sr64(name)       ({ unsigned long rval; asm volatile (\
                              " mrs %0, "#name \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define read_sr32(name)       ({ unsigned int rval; asm volatile (\
                              " mrs %0, "#name \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_sr64(val, name)      asm volatile (\
                              " mrs "#name", %0\n\t" \
                                : : "=r" (val) : "memory", "cc")

#define write_sr32(val, name) asm volatile(\
                              "msr "#name", %0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define ARCH_REGS_NUM_GPR   31
struct arch_regs {
    unsigned int cpsr;
    unsigned long pc;
    unsigned long gpr[ARCH_REGS_NUM_GPR];
};

/* reboot the board */
static void _reboot()
{ unsigned int *reboot = (unsigned int *) 0x17000014;
    *reboot = 0x1;
}

void _except_fiq(struct arch_regs *regs)
{
    unsigned long far;
    unsigned int esr;
    far = read_sr64(far_el1);
    esr = read_sr32(esr_el1);

    uart_print("fiq\n");
    uart_print("far_el1:");
    uart_print_hex64(far);
    uart_print("\n");

    uart_print("esr_el1:");
    uart_print_hex32(esr);
    uart_print("\n");

    uart_print("pc :");
    uart_print_hex64(regs->pc);
    uart_print("\n");

    _reboot();
}

void _except_ser(struct arch_regs *regs)
{
    unsigned long far;
    unsigned int esr;
    far = read_sr64(far_el1);
    esr = read_sr32(esr_el1);

    uart_print("serror\n");
    uart_print("far_el1:");
    uart_print_hex64(far);
    uart_print("\n");

    uart_print("esr_el1:");
    uart_print_hex32(esr);
    uart_print("\n");

    uart_print("pc :");
    uart_print_hex64(regs->pc);
    uart_print("\n");

    _reboot();
}

void _except_unhandled(struct arch_regs *regs)
{
    unsigned long far;
    unsigned int esr;
    unsigned int currentel;
    far = read_sr64(far_el1);
    esr = read_sr32(esr_el1);
    currentel = read_sr32(currentel);

    uart_print("currentEl:");
    uart_print_hex32(currentel);
    uart_print("\n");

    uart_print("far_el1:");
    uart_print_hex64(far);
    uart_print("\n");

    uart_print("esr_el1:");
    uart_print_hex32(esr);
    uart_print("\n");

    uart_print("pc :");
    uart_print_hex64(regs->pc);
    uart_print("\n");
    uart_print("unhandled\n");

    _reboot();
}

void _except_svc(struct arch_regs *regs)
{
    unsigned long far;
    unsigned int esr;
    far = read_sr64(far_el1);
    esr = read_sr32(esr_el1);

    uart_print("svc\n");
    uart_print("far_el1:");
    uart_print_hex64(far);
    uart_print("\n");

    uart_print("esr_el1:");
    uart_print_hex32(esr);
    uart_print("\n");

    uart_print("pc :");
    uart_print_hex64(regs->pc);
    uart_print("\n");

    _reboot();
}

void _except_irq(struct arch_regs *regs)
{
    gic_interrupt(0, regs);
}

