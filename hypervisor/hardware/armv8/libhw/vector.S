/*
 * monitor.S - Hyp mode portion of Monitor code
 *
 * Copyright (C) 2013 KESL. All rights reserved.
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

    mrs x0, spsr_el2
    mrs x1, elr_el2
    push x0, x1
    str x30, [sp, #-8]! // x30 = lr
.endm

.macro pop_registers
    ldr x30, [sp], #8 // x30 = lr
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

guest_vector_sync_64:
guest_vector_sync_32:
el2h_vector_sync:
    // Push registers
    push_registers

    // service other argument values -> _hyp_hvc_service(sp)
    msr daifclr, #2
    mov    x0, sp
    bl    _hyp_hvc    // r0: HSR

    // r0 = return
    tst    x0, #1
    // if return == HYP_RET_STAY -> stay in Hyp mode
    bne    1f

    // Pop registers
    pop_registers

    // else if return == HYP_RET_ERET -> Exception Return
    eret

1:
    // Pop registers
    ldr x30, [sp], #8   // x30 = lr
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
    // stay in Hyp mode
    mrs    x30, elr_el2 // x30 = lr
    eret
    //ldr    pc, x30 ?

hyp_vector_dabt:
    // Push registers
    push_registers

    // service other argument values -> _hyp_vector_dabort(sp)
    mov    x0, sp
    bl    _hyp_dabort    // r0: HSR

    // if (return == 0) execute a conditional code
    // tst    x0, #1
    // <conditional code>

    // Pop registers
    pop_registers
    eret

guest_vector_irq_64:
guest_vector_irq_32:
el2h_vector_irq:
    // Push registers
    push_registers

    // service other argument values -> _hyp_vector_irq(sp)
    mov    x0, sp
    bl    _hyp_irq    // r0: HSR

    // Pop registers
    pop_registers
    eret

guest_vector_fiq_64:
guest_vector_Serror_64:
guest_vector_fiq_32:
guest_vector_Serror_32:
el2h_vector_fiq:
el2h_vector_Serror:
hyp_vector_unhandled:
    // Push registers
    push_registers

    // service other argument values -> _hyp_vector_unhandled(sp)
    mov    x0, sp
    bl    _hyp_unhandled    // r0: HSR

    // Pop registers
    pop_registers
    eret
