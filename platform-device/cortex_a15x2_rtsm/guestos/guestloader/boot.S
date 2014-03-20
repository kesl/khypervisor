/*
 * Copyright (C) 2013 KESL. All rights reserved.
 */
    .text
    @ Guest loader start code
.global guestloader_start
guestloader_start:
    /* Stack pointer initialization */
    @ sp
    ldr     sp, =guestloader_stacklimit_svc

    @ sp for irq
    msr     cpsr_c, #0xd2
    ldr     sp, = guestloader_stacklimit_irq

    @ sp for guest(system / user)
    msr     cpsr_c, #0xdf
    ldr     sp, = guestloader_stacklimit

    @ exception vector
    ldr     r0, = nonsecure_vector
    mcr     p15, 0, r0, c12, c0, 0  @ VBAR

    @ Call the C entrypoint
    b   main
.type guestloader_start, %function

    .align 5
    nonsecure_vector:
    .word 0	                    /* reset */
    b	except_unhandled        /* undefined instruction */
    b	except_svc          	/* svc */
    b	except_pabort       	/* pabort */
    b	except_dabort 	        /* dabort  */
    b	except_unhandled    	/* unused */
    b	except_irq          	/* irq*/
    b	except_fiq	            /* fiq*/

    except_unhandled:
    @ Push registers
    push	{r0 - r12}
    mrs	    r0, spsr            /* CPSR */
    push	{r0, lr}

    mov	    r0, sp
    bl	    _except_unhandled

    @ Pop registers
    pop 	{r0, lr}            /* CPSR, LR */
    msr	    spsr, r0
    pop 	{r0 - r12}
    movs	pc, lr

    except_svc:
    @ Push registers
    push	{r0 - r12}
    mrs	    r0, spsr            /* CPSR */
    push	{r0, lr}

    mov	    r0, sp
    bl	    _except_svc

    @ Pop registers
    pop 	{r0, lr}            /* CPSR, LR */
    msr	    spsr, r0
    pop	    {r0 - r12}
    movs	pc, lr

    except_irq:
    @ Push registers
    push	{r0 - r12}
    mrs	    r0, spsr            /* CPSR */
    push	{r0, lr}

    mov	    r0, sp
    bl	    _except_irq

    @ Pop registers
    pop 	{r0, lr}            /* CPSR, LR */
    msr	    spsr, r0
    pop	    {r0 - r12}

    @ movs	pc, lr
    subs    pc, lr, #4

except_pabort:
except_dabort:
except_fiq:
b   except_unhandled
