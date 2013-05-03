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
	@ stack for guest
	ldr     sp, =guest_stacktop

	@ And call the C entrypoint
#ifdef BAREMETAL_GUEST
	b	guest_bin_start
#else
	b      nrm_loop
#endif
.type guest_start, %function
