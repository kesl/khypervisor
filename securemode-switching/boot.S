/*
 * boot.S - simple register setup code for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */

#define NS_BIT	0x1

	.syntax	unified
	.arch_extension sec
	.arch_extension virt
	.text

.macro enter_hyp
	@ We assume we're entered in Secure Supervisor mode. To
	@ get to Hyp mode we have to pass through Monitor mode
	@ and NS-Supervisor mode. Note that there is no way to
	@ return to the Secure world once we've done this.
	@
	@ This will trash r10 and r11.
	ldr	r10, =vectors
	mcr	p15, 0, r10, c12, c0, 1		@ Monitor vector base address
	@ Switch to monitor mode, which will set up the HVBAR and
	@ then return to us in NS-SVC
	smc	#0
	@ Now we're in NS-SVC, make a Hyp call to get into Hyp mode
	hvc	#0
	@ We will end up here in NS-Hyp.
.endm


.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
vectors:
	.word 0	/* reset */
	.word 0	/* undef */
	b	2f /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	b	1f
	.word 0 /* irq */
	.word 0 /* fiq */

/* Return directly back to the caller without leaving Hyp mode: */
1:	mrs	lr, elr_hyp
	mov	pc, lr

/* In monitor mode, set up HVBAR and SCR then return to caller in NS-SVC. */
2:
	@ Set up HVBAR
	mrc	p15, 0, r10, c1, c1, 0		@ SCR
	@ Set SCR.NS=1 (needed for setting HVBAR and also returning to NS state)
	@        .IRQ,FIQ,EA=0 (don't take aborts/exceptions to Monitor mode)
	@        .FW,AW=1 (CPSR.A,F modifiable in NS state)
	@        .nET=0 (early termination OK)
	@        .SCD=1 (SMC in NS mode is UNDEF, so accidental SMCs don't
	@                cause us to leap back into this code confusingly)
	@        .HCE=1 (HVC does Hyp call)
	bic	r10, r10, #0x07f
	ldr	r11, =0x1b1
	orr	r10, r10, r11
	mcr	p15, 0, r11, c1, c1, 0
	isb
	ldr	r11, =vectors
	mcr	p15, 4, r11, c12, c0, 0		@ set HVBAR
	@ ...and return to calling code in NS state
	movs	pc, lr


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
	.word 0 /* hvc */
	.word 0 /* irq */
	.word 0 /* fiq */

	.globl	start
start:
#ifdef SMP
#ifdef VEXPRESS
	@
	@ Program architected timer frequency
	@
	mrc	p15, 0, r0, c0, c1, 1		@ CPUID_EXT_PFR1
	lsr	r0, r0, #16
	and	r0, r0, #1			@ Check generic timer support
	beq	1f
	ldr	r0, =24000000			@ 24MHz timer frequency
	mcr	p15, 0, r0, c14, c0, 0		@ CNTFRQ
1:
#endif
	@
	@ CPU initialisation
	@
	mrc	p15, 0, r4, c0, c0, 5		@ MPIDR (ARMv7 only)
	and	r4, r4, #15			@ CPU number

	@
	@ Hypervisor / TrustZone initialization
	@

	@ Set all interrupts to be non-secure
	ldr	r0, =0x2c001000			@ Dist GIC base
	ldr	r1, [r0, #0x04]			@ Type Register
	cmp	r4, #0
	andeq	r1, r1, #0x1f
	movne	r1, #0
	add	r2, r0, #0x080			@ Security Register 0
	mvn	r3, #0
2:	str	r3, [r2]
	sub	r1, r1, #1
	add	r2, r2, #4			@ Next security register
	cmp	r1, #-1
	bne	2b

	@ Set GIC priority mask bit [7] = 1
	ldr	r0, =0x2c002000			@ CPU GIC base
	mov	r1, #0x80
	str	r1, [r0, #0x4]			@ GIC ICCPMR

	@ Set NSACR to allow coprocessor access from non-secure
	mrc	p15, 0, r0, c1, c1, 2
	ldr	r1, =0x43fff
	orr	r0, r0, r1
	mcr	p15, 0, r0, c1, c1, 2

	@ Check CPU nr again
	mrc	p15, 0, r0, c0, c0, 5		@ MPIDR (ARMv7 only)
	bfc	r0, #24, #8			@ CPU number, taking multicluster into account
	cmp	r0, #0				@ primary CPU?
	beq	2f

	@
	@ Secondary CPUs (following the RealView SMP booting protocol)
	@
	enter_hyp

	ldr	r1, =fs_start - 0x100
	adr	r2, 1f
	ldmia	r2, {r3 - r7}			@ move the code to a location
	stmia	r1, {r3 - r7}			@ less likely to be overridden
#ifdef VEXPRESS
	ldr	r0, =0x1c010030			@ VE SYS_FLAGS register
#else
	ldr	r0, =0x10000030			@ RealView SYS_FLAGS register
#endif
	mov	pc, r1				@ branch to the relocated code
1:
#ifdef VEXPRESS
	wfe
#endif
	ldr	r1, [r0]
	cmp	r1, #0
	beq	1b
	mov	pc, r1				@ branch to the given address
#endif

2:
	@
	@ UART initialisation (38400 8N1)
	@
#ifdef MACH_MPS
	ldr	r0, =0x1f005000			@ UART3 base (MPS)
#elif defined (VEXPRESS)
	ldr	r0, =0x1c090000			@ UART base (Versatile Express)
#else
	ldr	r0, =0x10009000			@ UART base (RealView/EB)
#endif
	mov	r1, #0x10			@ ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			@ cr
	str	r1, [r0, #0x30]

	@ Now we've got rid of the secondary CPUs, set up a stack
	@ for CPU 0 so we can write most of this in C.
	ldr     sp, =stacktop

	@ And call the C entrypoint
	bl      c_start
	@ Never reached
1:	b 1b

	@
	@ Function for C code to make semihosting calls:
	@
	.globl __semi_call
__semi_call:
#if defined(MACH_MPS)
	@ M profile semihosting is via bpkt
	bkpt    0xab
#elif defined(__thumb__)
	@ Otherwise, different SVC numbers for ARM or Thumb mode
	svc    0xab
#else
	svc     0x123456
#endif
	mov pc, lr

@ removed __boot_kernel() - simon 

handler_monitor_smc:

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

.global monitor_init
monitor_init:
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
	
.type monitor_init, %function

	
	@ Guest start code
guest_start:
	@ stack for guest
	ldr     sp, =guest_stacktop

	@ And call the C entrypoint
	bl      nrm_loop

	@ Hyp mode start code
hyp_start:


	@
	@ Data
	@
	/* The kernel boot command line for builtin kernels is defined in the Make system */
	.globl kernel_cmd
	.globl kernel_cmd_end
kernel_cmd:
#ifdef KCMD
	.asciz KCMD
#endif
kernel_cmd_end:


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
