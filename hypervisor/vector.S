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

/* ---[Secure Mode]------------------------------------------------------ */
/*
 * Secure Monitor Vector Table
 */
 .global monitor_secure_vectors
.align 5
    monitor_secure_vectors:
.word 0 /* reset */
    b   vector_secure_unhandled   /* undef*/
    b   vector_smc                /* smc */
    b   vector_secure_unhandled   /* pabt*/
    b   vector_secure_unhandled   /* dabt */
    b   vector_hyp_entry          /* hvc */
    b   vector_secure_unhandled   /* irq */
    b   vector_secure_unhandled   /* fiq*/

/* Install hvbar, hyp vector, and return in Non-secure state */
vector_smc:
    mrc p15, 0, r10, c1, c1, 0  @ SCR -> r10
    @ SCR.NS=1, IRQ,FIQ,EA=0, FW,AW=1, nET=0, SCD=1, HCE=1
    bic r10, r10, #0x07f
    ldr r11, = 0x1b1
    orr r10, r10, r11
    mcr p15, 0, r11, c1, c1, 0
    isb
    @ Use monitor_secure_vectors as temporary Hyp exception vector
    @ for Hyp mode entrance
    ldr r11, = monitor_secure_vectors
    mcr p15, 4, r11, c12, c0, 0
    @ return in NS state
    movs    pc, lr

vector_hyp_entry:
    /* Setup stack for Hyp for the first time */
    ldr sp, = mon_stacklimit
    /* Stay in Hyp mode and branch to where hvc invoked. */
    mrs lr, elr_hyp
    mov pc, lr

vector_secure_unhandled:
@ Push registers
    push    {r0 - r12}
    mrs r0, spsr_mon
    mov r1, #0
    push    {r0, r1}
    mov r0, sp
    /* Place holder in case we need to handle secure state exception */
    /* bl   _vector_secure_unhandled */
    @ Pop registers
    pop     {r0 - r1}
    msr spsr_mon, r0
    pop     {r0 - r12}
    eret

/* ---[Hyp Mode]------------------------------------------------------ */
.global monitor_hyp_vectors
/*
 * Monitor Vector Table
 */
.align 5
monitor_hyp_vectors:
    .word 0    /* reset */
    b    vector_unhandled    /* undef*/
    b    vector_unhandled     /* hypervisor */
    b    vector_unhandled    /* pabt*/
    b    vector_dabt    /* dabt */
    b    vector_hvc     /* hvc */
    b    vector_irq    /* irq */
    b    vector_unhandled    /* fiq*/


vector_hvc:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_hvc_service(sp)
    mov    r0, sp
    bl    _hyp_hvc    @ r0: HSR

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

vector_dabt:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_vector_dabort(sp)
    mov    r0, sp
    bl    _hyp_dabort    @ r0: HSR

    @ if (return == 0) execute a conditional code
    @ tst    r0, #1
    @ <conditional code>

    @ Pop registers
    pop     {r0-r1, lr}
    msr    spsr_hyp, r0
    msr    elr_hyp, r1
    pop     {r0-r12}
    eret

vector_irq:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_vector_irq(sp)
    mov    r0, sp
    bl    _hyp_irq    @ r0: HSR

    @ Pop registers
    pop     {r0-r1, lr}
    msr        spsr_hyp, r0
    msr        elr_hyp, r1
    pop     {r0-r12}
    eret

vector_unhandled:
    @ Push registers
    push    {r0-r12}
    mrs     r0, spsr_hyp
    mrs     r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_vector_unhandled(sp)
    mov    r0, sp
    bl    _hyp_unhandled    @ r0: HSR

    @ Pop registers
    pop     {r0-r1, lr}
    msr     spsr_hyp, r0
    msr     elr_hyp, r1
    pop     {r0-r12}
    eret
