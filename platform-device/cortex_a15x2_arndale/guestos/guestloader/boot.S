/*
 * Copyright (C) 2013 KESL. All rights reserved.
 */
    .text
    @ Guest loader start code
.global guestloader_start
guestloader_start:
    /* Stack pointer initialization */
    @ sp for irq
    msr     cpsr_c, #0xd2
    ldr     sp, = guestloader_stacklimit_irq

    @ sp for svc
    msr     cpsr_c, #0xd3
    ldr     sp, =guestloader_stacklimit_svc

    @ exception vector
    ldr     r0, = guestloader_vector
    mcr     p15, 0, r0, c12, c0, 0  @ VBAR

	/* Initilaize bss section */
	ldr r2, =begin_bss
	ldr r3, =end_bss
	mov r0, #0
1: 	str r0, [r2], #4		@ clear bss
	cmp r2, r3
	blo 1b

	/* Initilaize shared memory */
	ldr r2, =shared_memory_start
	ldr r3, =shared_memory_end
	mov r0, #0
2: 	str r0, [r2], #4
	cmp r2, r3
	blo 2b
	mov		r0, r10			@ For Boot status
    @ Call the C entrypoint
    b   main

.type guestloader_start, %function

    .align 5
    guestloader_vector:
    .word 0         /* reset */
    .word 0         /* undefined instruction */
    .word 0         /* svc */
    .word 0         /* pabort */
    .word 0         /* dabort  */
    .word 0         /* unused */
    b   except_irq  /* irq*/
    .word 0         /* fiq*/

    except_irq:
    @ Push registers
    push    {r0 - r12}
    mrs     r0, spsr    /* CPSR */
    push    {r0, lr}

    mov     r0, sp
    bl      _except_irq

    @ Pop registers
    pop     {r0, lr}    /* CPSR, LR */
    msr     spsr, r0
    pop     {r0 - r12}

    @ movs  pc, lr
    subs    pc, lr, #4
