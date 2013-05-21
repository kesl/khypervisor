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

PHYS_OFFSET = 0x80000000;

/* NS.SVC mode code space 
 * - simon
 */
GUEST_OFFSET  	= 0xC0000000;
GUEST_STACK  	= 0xCF000000;

SECTIONS
{
 . = PHYS_OFFSET;

/* Guest image between Kernel and Mon */
 . = GUEST_OFFSET;

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
