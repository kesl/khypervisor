/*
 * Copyright (C) 2015 KESL. All rights reserved.
 */
.text

/*
 * x10 - boot_status
 *
 */
#define EL2T 0x08
#define EL2H 0x09
// Guest loader start code
.global guestloader_start
guestloader_start:
    msr daifset, 0x4
    mrs x11, currentEL
    cmp x11, #EL2T
    ccmp x11, #EL2H, #0x4, ne
    b.ne init_stack
go_el1:
    mov x0, 0x05 //EL1H
    ldr x1, =init_stack
    msr spsr_el2, x0
    msr elr_el2, x1
    eret

init_stack:
    /* Stack pointer initialization */
    msr daifset, 0xf
    // sp for el1
    ldr     x0, = loader_stacklimit
    mov     sp, x0

    // exception vector
    ldr     x0, = guestloader_vector
    msr     vbar_el1, x0

    mov     x0, x10
    // Call the C entrypoint
    b   main

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

.macro push_registers spsr_elX elr_elX
    push x29, x30
    push x27, x28
    push x25, x26
    push x23, x24
    push x21, x22
    push x19, x20
    push x17, x18
    push x15, x16
    push x13, x14
    push x11, x12
    push x9, x10
    push x7, x8
    push x5, x6
    push x3, x4
    push x1, x2
    str x0, [sp, #-8]!

    mrs x0, \spsr_elX
    mrs x1, \elr_elX
    push x0, x1
.endm

.macro pop_registers spsr_elX elr_elX
    pop x0, x1
    msr \spsr_elX, x0
    msr \elr_elX, x1

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
    ldr x30, [sp], #8
.endm

.type guestloader_start, %function
.align 11
guestloader_vector:
    /* EL1t, SP_EL0 */
    entry except_svc /* el1t_sync */
    entry except_irq /* el1t_irq */
    entry except_fiq /* el1t_fiq */
    entry except_ser /* el1t_serr */
    /* EL1h, SP_EL0 */
    entry except_svc /* el1h_sync */
    entry except_irq /* el1h_irq */
    entry except_fiq /* el1h_fiq */
    entry except_ser /* el1h_serr */
    /* EL0t, SP_EL0, AArch64 */
    entry except_svc /* el0t_sync */
    entry except_irq /* el0t_irq */
    entry except_fiq /* el0t_fiq */
    entry except_ser /* el0t_serr */
    /* EL0t, SP_EL0, AArch32 */
    entry except_svc /* el0h_sync */
    entry except_irq /* el0h_irq */
    entry except_fiq /* el0h_fiq */
    entry except_ser /* el0h_serr */

except_svc:
    push_registers spsr_el1 elr_el1
    mov x0, sp
    bl      _except_svc
    pop_registers spsr_el1 elr_el1
    eret

except_irq:
    push_registers spsr_el1 elr_el1
    mov x0, sp
    bl      _except_irq
    pop_registers spsr_el1 elr_el1
    eret

except_fiq:
    push_registers spsr_el1 elr_el1
    mov x0, sp
    bl      _except_fiq
    pop_registers spsr_el1 elr_el1
    eret

except_ser:
    push_registers spsr_el1 elr_el1
    mov x0, sp
    bl      _except_ser
    pop_registers spsr_el1 elr_el1
    eret

_unhandled:
    push_registers spsr_el1 elr_el1
    mov x0, sp
    bl      _except_unhandled
    pop_registers  spsr_el1 elr_el1
    eret

