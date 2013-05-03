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

	
	@ Guest start code
.global guest_start
guest_start:

	b 	guest_entry
.type guest_start, %function

guest_entry:
	@ stack for guest
	ldr     sp, =guest_stacktop

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
