/*
 * monitor.S - Secure/Non-Secure Switching Monitor
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */

#define NS_BIT	0x1
#define HSR_EC_OFFSET	26
#define HSR_EC_HYPERCALL	0x12

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
	b	trap_unhandled	/* undef*/
	b	trap_smc 	/* smc */
	b	trap_unhandled	/* pabt*/
	b	trap_dabt	/* dabt */
	b	trap_hvc 	/* hvc */
	b	trap_irq	/* irq */
	b	trap_unhandled	/* fiq*/

trap_smc:
	mrc	p15, 0, r10, c1, c1, 0	@ SCR -> r10
	bic	r10, r10, #0x07f	@ SCR.NS=1, IRQ,FIQ,EA=0, FW,AW=1, nET=0, SCD=1, HCE=1
	ldr	r11, =0x1b1
	orr	r10, r10, r11
	mcr	p15, 0, r11, c1, c1, 0
	isb
	ldr	r11, =monitor_vectors
	mcr	p15, 4, r11, c12, c0, 0
	@ return in NS state
	movs	pc, lr

trap_hvc:
	@ Push registers
	push	{sp, lr}
	push	{r0-r12}
	mrs	r0, spsr_hyp
	mrs	r1, elr_hyp
	push	{r0, r1}

	@ hvc #imm:
	@	0xFFFF: stay in Hyp mode and branch to elr_hyp
	@	0xFFFE: [special] switch to Hyp ping
	@	0xFFFD: [special] switch to a guest
	@ mrc	p15, 4, r0, c5, c2, 0	@ HSR -> r0
	@ LSR	r1, r0, #HSR_EC_OFFSET
	
	@ service other argument values -> _hyp_hvc_service(sp)
	mov	r0, sp
	bl	_hyp_hvc_service	@ r0: HSR

	@ if (return == HYP_RET_STAY) stay in Hyp mode and eturn
	tst	r0, #1			
	bne	1f			

	@ else (HYP_RET_ERET)  Exception Return
	
	@ Pop registers
	pop 	{r0-r1}
	msr	spsr_hyp, r0
	msr	elr_hyp, r1
	ldm	sp, {r0-r14}
	eret
	
1:	
	@ Pop registers
	pop 	{r0-r1}
	msr	spsr_hyp, r0
	msr	elr_hyp, r1
	ldm	sp, {r0-r14}
	@ stay in Hyp mode
	mrs	lr, elr_hyp
	mov	pc, lr

trap_dabt:
	@ Push registers
	push	{sp, lr}
	push	{r0-r12}
	mrs	r0, spsr_hyp
	mrs	r1, elr_hyp
	push	{r0, r1}

	@ service other argument values -> _hyp_trap_dabort(sp)
	mov	r0, sp
	bl	_hyp_trap_dabort	@ r0: HSR

	@ if (return == 0) execute a conditional code
	@ tst	r0, #1			
	@ <conditional code>

	@ Pop registers
	pop 	{r0-r1}
	msr	spsr_hyp, r0
	msr	elr_hyp, r1
	ldm	sp, {r0-r14}
	eret

trap_irq:
	@ Push registers
	push	{sp, lr}
	push	{r0-r12}
	mrs	r0, spsr_hyp
	mrs	r1, elr_hyp
	push	{r0, r1}

	@ service other argument values -> _hyp_trap_irq(sp)
	mov	r0, sp
	bl	_hyp_trap_irq	@ r0: HSR

	@ Pop registers
	pop 	{r0-r1}
	msr	spsr_hyp, r0
	msr	elr_hyp, r1
	ldm	sp, {r0-r14}
	eret

trap_unhandled:
	@ Push registers
	push	{sp, lr}
	push	{r0-r12}
	mrs	r0, spsr_hyp
	mrs	r1, elr_hyp
	push	{r0, r1}

	@ service other argument values -> _hyp_trap_unhandled(sp)
	mov	r0, sp
	bl	_hyp_trap_unhandled	@ r0: HSR

	@ Pop registers
	pop 	{r0-r1}
	msr	spsr_hyp, r0
	msr	elr_hyp, r1
	ldm	sp, {r0-r14}
	eret

/*
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
*/

/*
.global __mon_switch_to_guest
__mon_switch_to_guest:

	@ Load Restoring Context Space Address
	@ ------------------------------------
	ldr	r1, =guest_context_sp
	ldr	r3, [r1]			@ r3 = [restoring_context_sp] for loading

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

	@ branch to LR: exception return from HVC
	# movs	pc, lr
	msr	elr_hyp, lr
	eret
.type __mon_switch_to_guest, %function
*/

.global __mon_switch_to_guest_context
__mon_switch_to_guest_context:	@ struct arch_regs *r0
	ldmfd	r0!, {r3, lr}
	tst 	r3, #0
	msrne	spsr_hyp, r3
	msr	elr_hyp, lr
	ldm	r0, {r0-r14}
	clrex
	eret


.type __mon_switch_to_guest_context, %function

/*
.global __mon_install_guest
__mon_install_guest:
	push {r0-r2, lr}
	ldr	r0, =guest_context_stack_limit
	stmfd	r0!, {r4, r12}

	mov	r1, 0				@ Invalid CPSR, No need to load
	ldr	r2, =guest_bin_start
	stmfd	r0!, {r1, r2}			@ CPSR, LR
	
	ldr 	r1, =guest_context_sp
	str	r0, [r1]			@ [guest_context_sp] = stack pointer for next time load
						@ [guest_context_sp] = guest_context_stack_limit - context_size

	@ regular return
	pop {r0-r2, pc}
.type __mon_install_guest, %function
*/

__mon_install_hyp:
	push {r0-r2, lr}
	ldr	r0, =hyp_context_stack_limit
	stmfd	r0!, {r4, r12}

	mov	r1, 0				@ Invalid CPSR, No need to load
	ldr	r2, =hyp_main
	stmfd	r0!, {r1, r2}			@ CPSR, LR
	
	ldr 	r1, =hyp_context_sp
	str	r0, [r1]			@ [hyp_context_sp] = stack pointer for next time load
						@ [hyp_context_sp] = hyp_context_stack_limit - context_size

	@ regular return
	pop {r0-r2, pc}

/* Initialize vector and stack pointers for modes: Secure SVC, Non-secure SVC, and Hyp mode
 * - simon
 */
.global mon_init
mon_init:
	push {r0-r2, lr}
	@ Setup Exception Vector
	ldr	r1, =monitor_vectors
	mcr	p15, 0, r1, c12, c0, 1		@ Monitor vector base address
/*
	@ NS.SVC entry
	@ -------------
	@	[guest_context_stack_limit] = 
	ldr	r0, =guest_context_stack_limit
	stmfd	r0!, {r4, r12}

	mov	r1, 0				@ Invalid CPSR, No need to load
	ldr	r2, =guest_bin_start
	stmfd	r0!, {r1, r2}			@ CPSR, LR
	
	ldr 	r1, =guest_context_sp
	str	r0, [r1]			@ [guest_context_sp] = stack pointer for next time load
						@ [guest_context_sp] = guest_context_stack_limit - context_size

	ldr	r0, =secure_context_stack_limit
	ldr	r1, =secure_context_sp
	str	r0, [r1]			@ [secure_context_sp] = secure_context_stack_limit
*/

	@ NS.HYP entry
	@ ------------
	@ TODO: Implement this part

	@ regular return
	pop {r0-r2, pc}
	
.type mon_init, %function

.global __mon_enter_hyp
__mon_enter_hyp:
	ldr r10, =monitor_vectors
	mcr	p15, 0, r10, c12, c0, 1	
	smc #0
	hvc #0xFFFF

	ldr	sp,=hyp_stacktop
	bl	__mon_install_hyp

	@TODO: load hyp context and brach instead of directly calling hyp_main
	bl	hyp_main

	@ never come back
1:	b 1b

.type __mon_enter_hyp, %function
	
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
