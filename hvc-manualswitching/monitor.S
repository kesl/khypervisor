/*
 * monitor.S - Secure/Non-Secure Switching Monitor
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */

#define NS_BIT	0x1

	.syntax	unified
	.arch_extension sec
	.arch_extension virt
	.text

/*
 * Monitor Vector Table
 */
.align 5
monitor_vectors:
	.word 0	/* reset */
	.word 0	/* undef */
	b	handler_monitor_smc /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	b	handler_monitor_hvc /* hvc */
	.word 0 /* irq */
	.word 0 /* fiq */

handler_monitor_smc:
	mrc	p15, 0, r10, c1, c1, 0	@ SCR -> r10
	bic	r10, r10, #0x07f	@ SCR.NS=1, IRQ,FIQ,EA=0, FW,AW=1, nET=0, SCD=1, HCE=1
	ldr	r11, =0x1b1
	orr	r10, r10, r11
	mcr	p15, 0, r11, c1, c1, 0
	isb
	ldr	r11, =monitor_vectors
	mcr	p15, 4, r11, c12, c0, 0
	@ return in NS state
	mvs	pc, lr

handler_monitor_hvc:
	mrs	lr, elr_hyp
	mov	pc, lr

switch_secure_mode:

	@ World switch
	@ -------------------------------
	mrc	p15, 0, r0, c1, c1, 0
	tst	r0, #NS_BIT
	EOR	r0, r0, #NS_BIT
	MCR	p15, 0, r0, c1, c1, 0
	@ -------------------------------

	@ Load Saving Context Space Address
	@ ------------------------------------
	@ Secure -> Non Secure: Save Secure World	- secure_context_sp
	@ Non Secure -> Secure: Save Non-secure World 	- guest_context_sp
	ldreq	r0, =secure_context_sp
	ldrne	r0, =guest_context_sp
	ldr	r2, [r0]			@ r2 = [saving_context_sp] for saving

	@ Load Restoring Context Space Address
	@ ------------------------------------
	ldreq	r1, =guest_context_sp
	ldrne	r1, =secure_context_sp
	ldr	r3, [r1]			@ r3 = [restoring_context_sp] for loading

	@ Save the current context -> r2
	@ ------------------------------------
	stmfd	r2!, {r4-r12}
	mrs	r4, spsr
	stmfd	r2!, {r4, lr}

	str	r2, [r0]			@ [saving_context_sp] = r2

	@ Load the saved context <- r3
	ldmfd	r3!, {r0, lr}
						@ First time smc called: S.SVC -> NS.SVC, Non Secure saved only lr
	tst	r0, #0				@ if ( saved(CPSR) != 0 ) SPSR_CXSF = r0
	msrne	spsr_cxsf, r0
	ldmfd	r3!, {r4-r12}

	str	r3, [r1]			@ [restoring_context_sp] = r3

	@ Clear Exclusive Access Record since we are switching the context
	@ ------------------------
	clrex

	@pop	{r0-r3}

	@ branch to LR: exception return from Security Monitor Call
	movs	pc, lr


/* Initialize vector and stack pointers for modes: Secure SVC, Non-secure SVC, and Hyp mode
 * - simon
 */

.global mon_init
mon_init:
	push {r0-r2, lr}
	@ Setup Exception Vector
	ldr	r1, =monitor_vectors
	mcr	p15, 0, r1, c12, c0, 1		@ Monitor vector base address

	@ NS.SVC entry
	@ -------------
	@	[guest_context_stack_limit] = 
	ldr	r0, =guest_context_stack_limit
	stmfd	r0!, {r4, r12}

	mov	r1, 0				@ Invalid CPSR, No need to load
	ldr	r2, =guest_start
	stmfd	r0!, {r1, r2}			@ CPSR, LR
	
	ldr 	r1, =guest_context_sp
	str	r0, [r1]			@ [guest_context_sp] = stack pointer for next time load
						@ [guest_context_sp] = guest_context_stack_limit - context_size

	ldr	r0, =secure_context_stack_limit
	ldr	r1, =secure_context_sp
	str	r0, [r1]			@ [secure_context_sp] = secure_context_stack_limit


	@ NS.HYP entry
	@ ------------
	@ TODO: Implement this part

	@ regular return
	pop {r0-r2, pc}
	
.type mon_init, %function

.global mon_enter_hyp
mon_enter_hyp:
	ldr r10, =monitor_vectors
	mcr	p15, 0, r10, c12, c0, 1	
	smc #0
	hvc #0

	ldr	sp,=hyp_stacktop
	bl	hyp_main

	@ never come back
1:	b 1b

.type mon_enter_hyp, %function
	
	@
	@ Data
	@

.data
	@ Space for secure context
secure_context_stack_base:
	.word	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
secure_context_stack_limit:

	@ Space for guest context
guest_context_stack_base:
	.word	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
guest_context_stack_limit:

	@ Space for hyp context
hyp_context_stack:
	.word	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
hyp_context_stack_limit:

	@ Address to last time saved/loaded in the Secure context space
secure_context_sp:
	.word 	0

	@ Address to last time saved/loaded in the Non Secure context space
guest_context_sp:
	.word	0

	@ Address to last time saved/loaded in the Hyp context space
hyp_context_sp:
	.word	0
