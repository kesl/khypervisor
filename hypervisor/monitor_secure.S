/*
 * monitor_secure.S - Secure mode code for switching to Hyp mode
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */

	.syntax	unified
	.arch_extension sec
	.arch_extension virt
	.text


/* ---[Secure Mode]------------------------------------------------------ */

/*
 * Secure Monitor Vector Table
 */
.align 5
monitor_secure_vectors:
	.word 0	/* reset */
	b	trap_secure_unhandled	/* undef*/
	b	trap_smc                /* smc */
	b	trap_secure_unhandled	/* pabt*/
	b	trap_secure_unhandled	/* dabt */
	b	trap_hyp_entry 	        /* hvc */
	b	trap_secure_unhandled	/* irq */
	b	trap_secure_unhandled	/* fiq*/

    /* Install hvbar, hyp vector, and return in Non-secure state */
trap_smc:
	mrc	p15, 0, r10, c1, c1, 0	@ SCR -> r10
	bic	r10, r10, #0x07f	@ SCR.NS=1, IRQ,FIQ,EA=0, FW,AW=1, nET=0, SCD=1, HCE=1
	ldr	r11, =0x1b1
	orr	r10, r10, r11
	mcr	p15, 0, r11, c1, c1, 0
	isb

    @ Use monitor_secure_vectors as temporary Hyp exception vector for Hyp mode entrance
	ldr	r11, =monitor_secure_vectors
	mcr	p15, 4, r11, c12, c0, 0
	@ return in NS state
	movs	pc, lr

trap_hyp_entry:
    /* Setup stack for Hyp for the first time */
	ldr	sp,=mon_stacklimit

    /* Stay in Hyp mode and branch to where hvc invoked. */
	mrs	lr, elr_hyp
	mov	pc, lr

trap_secure_unhandled:
	@ Push registers
	push	{r0-r12}
	mrs	r0, spsr_mon
	mov	r1, #0
	push	{r0, r1}

	mov	r0, sp
	/* Place holder in case we need to handle secure state exception */
	/* bl	_trap_secure_unhandled */

	@ Pop registers
	pop 	{r0-r1}
	msr	spsr_mon, r0
	pop     {r0-r12}
	eret

/* 
 * Initialize vector and stack pointers for modes: Secure SVC, Non-secure SVC, and Hyp mode
 */
.global mon_init
mon_init:
	push {r0-r2, lr}
	@ Setup Exception Vector
	ldr	r1, =monitor_secure_vectors
	mcr	p15, 0, r1, c12, c0, 1		@ Monitor vector base address

	@ regular return
	pop {r0-r2, pc}
	
.type mon_init, %function

.global __mon_enter_hyp
__mon_enter_hyp:
	smc #0  @ -> trap_smc
	hvc #0  @ -> trap_hyp_entry

    /* Place the right Hyp exception vector for the first time */
	ldr	r0, =monitor_hyp_vectors
	mcr	p15, 4, r0, c12, c0, 0

	bl main

	@ never come back
1:	b 1b

.type __mon_enter_hyp, %function
