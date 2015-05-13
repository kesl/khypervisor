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

.global hyp_init_vectors
/*
 * Hypervisor Vector Table
 */
.align 11
hyp_init_vectors:
    /* EL2t, SP_EL0 */
    entry   hyp_vector_unhandled        /* EL2t Synchronous */
    entry   hyp_vector_unhandled        /* EL2t IRQ */
    entry   hyp_vector_unhandled        /* EL2t FIQ */
    entry   hyp_vector_unhandled        /* EL2t SError */
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
guest_vector_sync_32:
el2h_vector_sync:
    push_registers

    // service other argument values -> _hyp_sync_service(sp)
    msr daifclr, #2
    mov    x0, sp
    bl    _hyp_sync    // r0: HSR

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
    ldr x0, #0
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
guest_vector_irq_32:
el2h_vector_irq:
    push_registers

    // service other argument values -> _hyp_vector_irq(sp)
    mov    x0, sp
    bl    _hyp_irq    // r0: HSR

    pop_registers
    eret

/*
 * todo
 * - Inform the unhandled each routine's name & state
 * - Separate each mode handler to satisfy upper goal
 */
guest_vector_fiq_64:
guest_vector_Serror_64:
guest_vector_fiq_32:
guest_vector_Serror_32:
el2h_vector_fiq:
el2h_vector_Serror:
hyp_vector_unhandled:
    push_registers

    // service other argument values -> _hyp_vector_unhandled(sp)
    mov    x0, sp
    bl    _hyp_unhandled    // r0: HSR

    pop_registers
    eret
