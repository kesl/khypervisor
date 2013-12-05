
.extern do_IRQ

.text
.global start_ram
start_ram:
	b 	guest_entry
.type start_ram, %function
guest_entry:
	cpsid	if
/* Initialize bss section */
    ldr  r2, =begin_bss
    ldr  r3, =end_bss
    mov  r0, #0
1:  str  r0, [r2], #4    @ clear bss
    cmp  r2, r3
    blo  1b

    @ sp for guest svc mode
    ldr     sp, =guest_stacklimit_svc

    @ sp_irq
    msr     cpsr_c, #0xd2
    ldr     sp, =guest_stacklimit_irq

    @ sp for guest (system/user)
    msr     cpsr_c, #0xdf
    ldr     sp, =guest_stacklimit

  	@ exception vector
    ldr     r0, =nonsecure_vector
    mcr     p15, 0, r0, c12, c0, 0      @ VBAR

    b	start_up

.align 5
nonsecure_vector:
	.word 0	                    /* reset */
	b	arm_undef_exception
	b	arm_swi_exception
	b	arm_prefetch_abort_exception
	b	arm_data_abort_exception
        nop
	b	arm_irq_exception
	b	arm_fiq_exception

arm_undef_exception:
	movs    pc, lr

arm_swi_exception:
	ldmfd	sp!, {r0}
	stmfd	sp!, {r0, lr}
	mrs		r0, SPSR
	stmfd	sp!, {r0}
	ldr		r0, [lr, #-4]
	bic 		r0, r0, #0xff000000
	cmp 	r0, #0x00
	bne		return_swi_handler
return_swi_handler:
	ldmfd	sp!, {r0}
	msr		SPSR_cxsf, r0
	ldmfd	sp!, {r0,lr}
	movs	pc, lr

arm_prefetch_abort_exception:
	b	arm_prefetch_abort_exception

arm_data_abort_exception:
	b	arm_data_abort_exception

arm_fiq_exception:
	subs	pc, lr, #4

arm_irq_exception:
	mrs	r8, spsr
	stmfd	sp!, {r0-r3}
	mov		r2, sp
	add		sp, sp,#16
	sub		r3,lr,#4
	mov		r1, r8
	orr		r1, r1, #0x80
	msr		SPSR, r1
	ldr		r0,=irq_svc_handler
	movs	pc,r0

irq_svc_handler:
	stmfd	sp!,{r3}
	stmfd	sp!,{r4-r12,lr}
	mov		r4,r2
	ldmfd	r4!,{r0-r3}
	stmfd	sp!,{r0-r3}
	mrs		r5,CPSR
	bic		r5, r5, #0x80		// I bit clear
	stmfd	sp!,{r5}
	ldr		r0,=OSIntNesting
	ldrb		r1,[r0]
	add		r1,r1,#1
	strb		r1,[r0]

	mov		r0, sp
	bl		do_IRQ
	bl		OSIntExit
	ldmfd	sp!,{r0}
	msr		SPSR,r0
	ldmfd	sp!,{r0 - r12, lr , pc}^

