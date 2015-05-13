/*
 * guest.S - Secure/Non-Secure Switching Monitor
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */

.text
.global common_guest_entry
common_guest_entry:
/* Stack pointer initialization for svc, irq, and system/user modes */
/*
 * test print code
 * x10 - uart address
 * x11 - character
 */
    ldr x10, =0x1c020000
    ldr x11, =0x30
    str x11, [x10]
    // sp for el1, spsr_el1
    ldr     x0, = guest_stacklimit_svc
    mov     sp, x0
    mov     x0, #0x1C5
    msr     spsr_el1, x0

    ldr x11, =0x31
    str x11, [x10]

    // sp for el0
    ldr     x0, = guest_stacklimit
    msr     sp_el0, x0

    ldr x11, =0x32
    str x11, [x10]

    // exception vector
    ldr     x0, = nonsecure_vector
    msr     vbar_el1, x0    //vbar_el1

    ldr x11, =0x33
    str x11, [x10]

    // And call the C entrypoint
    bl      nrm_loop

    //
    // Function for C code to make semihosting calls:
    //
    .globl __semi_call
    __semi_call:
#if defined(MACH_MPS)
    // M profile semihosting is via bpkt
    bkpt    0xab
#elif defined(__thumb__)
    // Otherwise, different SVC numbers for ARM or Thumb mode
    svc    0xab
#else
    //svc     0x123456
#endif
    eret//mov pc, x30
/*********************************************************************************/
/* MACRO */
.macro  entry   entry
    .align 7
    b   \entry
    .endm

.macro push, val1, val2
    stp \val1, \val2, [sp, #-16]!
.endm

.macro pop, val1, val2
    ldp \val1, \val2, [sp], #16
.endm

.macro push_registers spsr_elX
    push x28, x29
    push x26, x27
    push x24, x25
    push x22, x23
    push x20, x21
    push x18, x19
    push x16, x17
    push x14, x15
    push x12, x13
    push x10, x11
    push x8, x9
    push x6, x7
    push x4, x5
    push x2, x3
    push x0, x1

    mrs x0, \spsr_elX
    push x0, x30
.endm

.macro pop_registers spsr_elX
    pop x0, x30
    msr \spsr_elX, x0

    pop x0, x1
    pop x2, x3
    pop x4, x5
    pop x6, x7
    pop x8, x9
    pop x10, x11
    pop x12, x13
    pop x14, x15
    pop x16, x17
    pop x18, x19
    pop x20, x21
    pop x22, x23
    pop x24, x25
    pop x26, x27
    pop x28, x29
.endm


/*********************************************************************************/
.align 11
nonsecure_vector:
    /* EL1t, SP_EL0 */
    entry   el1t_sync
    entry   el1t_irq
    entry   el1t_fiq
    entry   el1t_serr
    /* EL1h, SP_EL1 */
    entry   el1h_sync
    entry   el1h_irq
    entry   el1h_fiq
    entry   el1h_serr
    /* EL0t, SP_EL0, AArch64 */
    entry   el0t_a64_sync
    entry   el0t_a64_irq
    entry   el0t_a64_fiq
    entry   el0t_a64_serr
    /* EL0t, SP_EL0, AArch32 */
    entry   el0t_a32_sync
    entry   el0t_a32_irq
    entry   el0t_a32_fiq
    entry   el0t_a32_serr

el1t_sync:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_svc         //svc & dabort
    pop_registers spsr_el1
    eret
    //msr pc, x30
el1t_irq:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_irq
    pop_registers spsr_el1
    eret
    //msr pc, x30
el1t_fiq:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_unhandled
    pop_registers spsr_el1
    eret
    //msr pc, x30
el1t_serr:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_unhandled
    pop_registers spsr_el1
    eret//msr pc, x30

el1h_sync:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_svc         //svc & dabort
    pop_registers spsr_el1
    eret//msr pc, x30
el1h_irq:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_irq
    pop_registers spsr_el1
    eret//msr pc, x30
el1h_fiq:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_unhandled
    pop_registers spsr_el1
    eret//msr pc, x30
el1h_serr:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_unhandled
    pop_registers spsr_el1
    eret//msr pc, x30

el0t_a64_sync:
el0t_a64_irq:
el0t_a64_fiq:
el0t_a64_serr:
el0t_a32_sync:
el0t_a32_irq:
el0t_a32_fiq:
el0t_a32_serr:
    push_registers spsr_el1
    mov x0, sp
    bl  _except_unhandled
    pop_registers spsr_el1
    eret// msr pc, x30
