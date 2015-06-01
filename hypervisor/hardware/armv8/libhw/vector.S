/*
 * monitor.S - Hyp mode portion of Monitor code
 *
 * Copyright (C) 2015 KESL. All rights reserved.
 *
 */

    .text
.macro push, val1, val2
    stp \val1, \val2, [sp, #-16]!
.endm
.macro pop, val1, val2
    ldp \val1, \val2, [sp], #16
.endm

.macro push_registers
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

    mrs x0, spsr_el2
    mrs x1, elr_el2
    push x0, x1
.endm

.macro pop_registers
    pop x0, x1
    msr spsr_el2, x0
    msr elr_el2, x1

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

#include <k-hypervisor-config.h>
/* ---[Hyp Mode]------------------------------------------------------ */
.macro entry   entry
    .align 7
    b   \entry
.endm

/* Exception Level */
#define GUEST_64    0x0
#define GUEST_32    0x1
#define EL2T        0x2
#define EL2H        0x3

/* Mode */
#define SYNC    0
#define IRQ     1
#define FIQ     2
#define SERROR  3
.macro run_service, service, el
    push_registers

    mov x0, sp
    mov x1, \el
    bl \service     // r0: HSR

    pop_registers
.endm
.macro run_unhandled, el, mode
    push_registers

    mov x0, sp
    mov x1, \el
    mov x2, \mode
    bl _unhandled     // r0: HSR


    pop_registers
.endm

/*
 * Hypervisor Vector Table
 */
    .align 11
    .global hyp_init_vectors
    .align 2
hyp_init_vectors:
    /* EL2t, SP_EL0 */
    entry   el2t_vector_sync            /* EL2t Synchronous */
    entry   el2t_vector_irq             /* EL2t IRQ */
    entry   el2t_vector_fiq             /* EL2t FIQ */
    entry   el2t_vector_Serror          /* EL2t SError */
    /* EL2h, SP_EL2 */
    entry   el2h_vector_sync            /* EL2h Synchronous */
    entry   el2h_vector_irq             /* EL2h IRQ */
    entry   el2h_vector_fiq             /* EL2h FIQ */
    entry   el2h_vector_Serror          /* EL2h SError */
    /* EL0/EL1, Guest, AArch64 */
    entry   guest_vector_sync_64        /* EL0/1 Synchronous */
    entry   guest_vector_irq_64         /* EL0/1 IRQ */
    entry   guest_vector_fiq_64         /* EL0/1 FIQ */
    entry   guest_vector_Serror_64      /* EL0/1 SError */
    /* EL0/EL1, Guest, AArch32 */
    entry   guest_vector_sync_32        /* EL0/1 Synchronous */
    entry   guest_vector_irq_32         /* EL0/1 IRQ */
    entry   guest_vector_fiq_32         /* EL0/1 FIQ */
    entry   guest_vector_Serror_32      /* EL0/1 SError */

    /* synchronous = hvc + dabort + etc*/
guest_vector_sync_64:
    run_service _sync GUEST_64
    eret
guest_vector_sync_32:
    run_service _sync GUEST_32
    eret
el2h_vector_sync:
    push_registers

    // service other argument values -> _sync_service(sp)
    //msr daifclr, #2
    mov x0, sp
    mov x1, EL2H
    bl  _sync    // r0: HSR

    // r0 = return
    tst    x0, #1
    // if return == HYP_RET_STAY -> stay in Hyp mode
    bne    1f

    pop_registers

    // else if return == HYP_RET_ERET -> Exception Return
    eret

1:
    // Pop registers
    pop x0, x1
    tst x0, #0x1f
    beq 2f
    ldr x0, =0
    msr spsr_el2, x0
2:
    msr elr_el2, x1

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
    // stay in Hyp mode
    eret
    //ldr    pc, x30 ?

guest_vector_irq_64:
    run_service _irq GUEST_64
    eret
guest_vector_irq_32:
    run_service _irq GUEST_32
    eret
el2h_vector_irq:
    run_service _irq EL2H
    eret

/* unhandled Exception modes */
    /* guest_64 */
guest_vector_fiq_64:
    run_unhandled   GUEST_64, FIQ
guest_vector_Serror_64:
    run_unhandled   GUEST_64, SERROR

    /* guest_32 */
guest_vector_fiq_32:
    run_unhandled   GUEST_32, FIQ
guest_vector_Serror_32:
    run_unhandled   GUEST_32, SERROR

    /* el2 handler */
el2h_vector_fiq:
    run_unhandled   EL2H, FIQ
el2h_vector_Serror:
    run_unhandled   EL2H, SERROR

    /* el2 thread */
el2t_vector_sync:
    run_unhandled   EL2T, SYNC
el2t_vector_irq:
    run_unhandled   EL2T, IRQ
el2t_vector_fiq:
    run_unhandled   EL2T, FIQ
el2t_vector_Serror:
    run_unhandled   EL2T, SERROR
