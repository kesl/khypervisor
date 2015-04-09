/*
 * boot.S - simple register setup code for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */

#include <k-hypervisor-config.h>
    .text 
hyp_init_vectors:
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0

.align 5
    /*
     *
     * x0 - address or system register temp
     * x1 - arg 1
     * x2 - arg 2
     * x3 - arg 3
     * x4 -
     * x5 -
     * x6 - bitmap temp 0
     * x7 - bitmap temp 1
     * x7 -
     * x9 -
     * x10 -
     * x11 -
     * x12 -
     * x13 -
     * x14 -
     * x15 -
     * x16 -
     * x17 -
     * x18 -
     * x19 -
     * x20 - cpu id & cluster id
     * x21 -
     * x22 -
     * x23 -
     * x24 -
     * x25 -
     * x26 -
     * x27 -
     * x28 -
     * x29 -
     * x30 -
     *
     */
    .globl	start
start:
    #
    # Program architected timer frequency
    #

    ldr	x0, = CFG_CNTFRQ
    msr cntfrq_el0, x0      // CNTFRQ
1:
    #
    # Get CPU Number & Cluster id
    #
    mrs x0, mpidr_el1
    mov x6, #~0xffffff
    bic x20, x0, x6

    #
    # EL2 initialization
    #

    cmp	x20, #0				// is primary CPU?
    beq	2f

    mov x0, x1
    msr spsel, x0
    ldr x0, =sec_stacklimit

    # set sp register for secondary cpus
    mov sp, x0
    mov x1, #SEC_STACK_SIZE
    mul  x1, x1, x20
    sub sp, sp, x1
    #
    # Secondary CPUs
    #
    bl go_to_c // hyp_main()

    .globl enter_smp_pen
enter_smp_pen:
    ldr  x0, =secondary_smp_pen
    ldr x1, [x0]
    cmp x1, #0 // pending
    beq enter_smp_pen

    /*
     * Make instruction copy coherent
     */
    dc cvau, x2     /* Clean the data cache by MVA*/
    ic iallu        /* Invalidate the I-cache */
    isb /* Make sure the invalidate ops are complete */
    dsb sy

    bl main
    # never come back
    1:
    b 1b

2:
    # serial device initialize routine was removed
    /* Initialize bss section */
clear_bss:
    ldr  x0, =begin_bss
    ldr  x1, =end_bss

1:  str  xzr, [x0], #8
    cmp  x0, x1
    blo  1b
    # Now we've got rid of the secondary CPUs, set up a stack
    # for CPU 0 so we can write most of this in C.
    mov x0, 0x1
    msr spsel, x0

    ldr x0, =sec_stacklimit
    mov sp, x0

    bl go_to_c // hyp_main()

	# Never reached
1:
    b 1b

    /*
     * Initialize vector and stack pointers for modes:
     * EL2
     */
    .global go_to_c 
go_to_c:
    /* Place the right Hyp exception vector for the first time */
    ldr x0, = hyp_init_vectors
    msr vbar_el2, x0

    cmp	x20, #0				// is primary CPU?
    beq	2f
    #
    # Secondary CPUs
    #
    b enter_smp_pen
2:
    bl main
    # never come back
    1:
    b 1b
.type go_to_c, % function


/* this code must be changed*/
.global __mon_switch_to_guest_context
__mon_switch_to_guest_context:    # struct arch_regs *r0
#ldmfd    x0!, {x1, x3, lr}
    msr     spsr_el2, x1
    msr     elr_el2, x3
#ldm     x0, {x0-x12}
    clrex
    eret
.type __mon_switch_to_guest_context, %function
