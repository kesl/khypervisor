/*
 * monitor.S - Hyp mode portion of Monitor code
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */

    .syntax unified
    .arch_extension sec
    .arch_extension virt
    .text


.global monitor_hyp_vectors
/*
 * Monitor Vector Table
 */
.align 5
monitor_hyp_vectors:
    .word 0    /* reset */
    b    trap_unhandled    /* undef*/
    b    trap_unhandled     /* hypervisor */
    b    trap_unhandled    /* pabt*/
    b    trap_dabt    /* dabt */
    b    trap_hvc     /* hvc */
    b    trap_irq    /* irq */
    b    trap_unhandled    /* fiq*/


trap_hvc:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_hvc_service(sp)
    mov    r0, sp
    bl    _hyp_hvc_service    @ r0: HSR

    @ r0 = return
    tst    r0, #1
    @ if return == HYP_RET_STAY -> stay in Hyp mode
    bne    1f

    @ Pop registers
    pop     {r0-r1, lr}
    msr        spsr_hyp, r0
    msr        elr_hyp, r1
    pop     {r0-r12}

    @ else if return == HYP_RET_ERET -> Exception Return
    eret

1:
    @ Pop registers
    pop     {r0-r1, lr}
    tst     r0, #0x1f
    msrne    spsr_hyp, r0
    msr    elr_hyp, r1
    pop     {r0-r12}
    @ stay in Hyp mode
    mrs    lr, elr_hyp
    mov    pc, lr

trap_dabt:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_trap_dabort(sp)
    mov    r0, sp
    bl    _hyp_trap_dabort    @ r0: HSR

    @ if (return == 0) execute a conditional code
    @ tst    r0, #1
    @ <conditional code>

    @ Pop registers
    pop     {r0-r1, lr}
    msr    spsr_hyp, r0
    msr    elr_hyp, r1
    pop     {r0-r12}
    eret

trap_irq:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_trap_irq(sp)
    mov    r0, sp
    bl    _hyp_trap_irq    @ r0: HSR

    @ Pop registers
    pop     {r0-r1, lr}
    msr        spsr_hyp, r0
    msr        elr_hyp, r1
    pop     {r0-r12}
    eret

trap_unhandled:
    @ Push registers
    push    {r0-r12}
    mrs     r0, spsr_hyp
    mrs     r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_trap_unhandled(sp)
    mov    r0, sp
    bl    _hyp_trap_unhandled    @ r0: HSR

    @ Pop registers
    pop     {r0-r1, lr}
    msr     spsr_hyp, r0
    msr     elr_hyp, r1
    pop     {r0-r12}
    eret

.global __mon_switch_to_guest_context
__mon_switch_to_guest_context:    @ struct arch_regs *r0
    ldmfd    r0!, {r1, r3, lr}
    msr     spsr_hyp, r1
    msr     elr_hyp, r3
    ldm     r0, {r0-r12}
    clrex
    eret
.type __mon_switch_to_guest_context, %function



