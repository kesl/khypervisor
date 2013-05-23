/*
 * model.lds.S - simple linker script for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)

#ifdef LDS_PHYS_OFFSET
PHYS_OFFSET = LDS_PHYS_OFFSET;
#else
PHYS_OFFSET = 0x00000000;
#endif

/* NS.SVC mode code space 
 * LDS_GUEST_OFFSET ~ LDS_GUEST_STACK + 0x01000000
 * - simon
 */
#ifdef LDS_GUEST_OFFSET
GUEST_OFFSET  	= LDS_GUEST_OFFSET;
#else
GUEST_OFFSET  	= 0x00000000;
#endif

#ifdef LDS_GUEST_STACK
GUEST_STACK  	= LDS_GUEST_STACK;
#else
GUEST_STACK  	= 0x0F000000;
#endif

SECTIONS
{
 . = PHYS_OFFSET;

/* Guest image between Kernel and Mon */
 . = GUEST_OFFSET;
#if (LDS_GUEST_SECMON==1)
 __secmon_guest_start = .;
#elif (LDS_GUEST_HYPMON==1)
 __hypmon_guest_start = .;
#else
 __monitor_type_unknown_guest_start = .;
#endif

 /* Put most of the actual boot loader code up in high memory
  * where it won't get overwritten by kernel, initrd or atags.
  */
 .text : { 
	guest.o(.text)
 }
 .data : { 
	guest.o(.data) 
 }
 .bss : { 
	guest.o(.bss) 
 }

 . = GUEST_STACK;
 guest_stacktop = .;
 . = GUEST_STACK + 0x01000000; 
 guest_stacklimit = .;
}
