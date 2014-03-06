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
.align 5
    monitor_secure_vectors:
.word 0 /* reset */
    b   trap_secure_unhandled   /* undef*/
    b   trap_smc                /* smc */
    b   trap_secure_unhandled   /* pabt*/
    b   trap_secure_unhandled   /* dabt */
    b   trap_hyp_entry          /* hvc */
    b   trap_secure_unhandled   /* irq */
    b   trap_secure_unhandled   /* fiq*/

/* Install hvbar, hyp vector, and return in Non-secure state */
trap_smc:
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

trap_hyp_entry:
    /* Setup stack for Hyp for the first time */
    ldr sp, = mon_stacklimit
    /* Stay in Hyp mode and branch to where hvc invoked. */
    mrs lr, elr_hyp
    mov pc, lr

trap_secure_unhandled:
@ Push registers
    push    {r0 - r12}
    mrs r0, spsr_mon
    mov r1, #0
    push    {r0, r1}
    mov r0, sp
    /* Place holder in case we need to handle secure state exception */
    /* bl   _trap_secure_unhandled */
    @ Pop registers
    pop     {r0 - r1}
    msr spsr_mon, r0
    pop     {r0 - r12}
    eret
/*
 * Initialize vector and stack pointers for modes:
 * Secure SVC, Non-secure SVC, and Hyp mode
 */
.global mon_init
    mon_init:
    push {r0 - r2, lr}
    @ Setup Exception Vector
    ldr r1, = monitor_secure_vectors
    mcr p15, 0, r1, c12, c0, 1      @ Monitor vector base address
    @ regular return
    pop {r0 - r2, pc}

.type mon_init, % function

.global mon_enter_hyp
    mon_enter_hyp:
    smc #0  @ -> trap_smc
    hvc #0  @ -> trap_hyp_entry
    /* Place the right Hyp exception vector for the first time */
    ldr r0, = monitor_hyp_vectors
    mcr p15, 4, r0, c12, c0, 0
    bl main
    @ never come back
    1:
    b 1b

.type mon_enter_hyp, % function


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



