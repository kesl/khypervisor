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

.global guest2_start
guest2_start:
	@ stack for guest
	ldr     sp, =guest2_stacktop

	@ And call the C entrypoint
#ifdef BAREMETAL_GUEST
	b	guest2_bin_start
#else
	b      nrm_loop
#endif
.type guest2_start, %function


