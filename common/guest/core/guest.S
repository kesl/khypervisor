/*
 * guest.S - Secure/Non-Secure Switching Monitor
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */

.syntax	unified
.arch_extension sec
.arch_extension virt
.text
.global common_guest_entry
common_guest_entry:
/* Stack pointer initialization for svc, irq, and system/user modes */
    @ sp for guest svc mode
    ldr     sp, = guest_stacklimit_svc

    @ sp_irq
    msr     cpsr_c, #0xd2
    ldr     sp, = guest_stacklimit_irq

    @ sp for guest(system / user)
    msr     cpsr_c, #0xdf
    ldr     sp, = guest_stacklimit

    @ exception vector
    ldr     r0, = nonsecure_vector
    mcr     p15, 0, r0, c12, c0, 0      @ VBAR

    @ And call the C entrypoint
    bl      nrm_loop

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
