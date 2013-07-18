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
ENTRY(start)

#ifndef SEMIHOSTING
INPUT(./uImage)
#ifdef USE_INITRD
 INPUT(./filesystem.cpio.gz)
#endif
#endif

#ifdef BAREMETAL_GUEST
	INPUT(./bmguest.bin)
/* INPUT(./bmguest2.bin) */
#endif

PHYS_OFFSET = 0x80000000;       /* Customize */
MON_OFFSET  = 0xF0000000;       /* Customize */

MON_SIZE    = 0x0F000000;
MON_STACK	= MON_OFFSET + MON_SIZE;
MON_STACK_SIZE  = 0x00c00000;

SEC_STACKTOP = MON_STACK + MON_STACK_SIZE;
SEC_STACK_SIZE  = 0x00400000;

/* NS.SVC mode code space 
 * - simon
 */
GUEST_OFFSET  	= 0xA0000000;   /* Customize */
GUEST_SIZE_MAX	= 0x0F000000;
GUEST_STACK  	= GUEST_OFFSET + GUEST_SIZE_MAX;

GUEST2_OFFSET	= 0xB0000000;   /* Customize */
GUEST2_STACK	= GUEST2_OFFSET + GUEST_SIZE_MAX;


SECTIONS
{
 . = PHYS_OFFSET;
 . = PHYS_OFFSET + 0x8000 - 0x40;
 . = PHYS_OFFSET + 0x00d00000;

 fs_start = .;
#if defined(USE_INITRD) && !defined(SEMIHOSTING)
 .filesystem : { ./filesystem.cpio.gz }
#endif
 fs_end = .;

/* Guest 1 */
 . = GUEST_OFFSET;
  guest_bin_start = .; 
#ifdef BAREMETAL_GUEST
  .guest : { ./bmguest.bin }
#else
	/* dummy space for guest */
 . = GUEST_OFFSET + GUEST_SIZE_MAX;
#endif
  guest_bin_end = .; 

 . = GUEST_STACK;
 guest_stacktop = .;
 . = GUEST_STACK + 0x01000000; 
 guest_stacklimit = .;

/* Guest 2 */
 . = GUEST2_OFFSET;
  guest2_bin_start = .; 
#ifdef BAREMETAL_GUEST
  .guest2 : { ./bmguest.bin }
#else
	/* dummy space for guest */
 . = GUEST2_OFFSET + GUEST_SIZE_MAX; 
#endif
  guest2_bin_end = .; 


 . = GUEST2_STACK;
 guest2_stacktop = .;
 . = GUEST2_STACK + 0x01000000; 
 guest2_stacklimit = .;

 . = MON_OFFSET;
 /* Put most of the actual boot loader code up in high memory
  * where it won't get overwritten by kernel, initrd or atags.
  */
 .text : { 
	boot.o(.text) 
	monitor.o(.text) 
 }
 .data : { 
	boot.o(.data) 
	monitor.o(.data) 
 }
 .bss : { 
	boot.o(.bss) 
	monitor.o(.bss) 
 }
 . = MON_STACK;
 mon_stacktop = .;
 . = MON_STACK + MON_STACK_SIZE;
 mon_stacklimit = .;

 . = SEC_STACKTOP;
 sec_stacktop = .;
 . = SEC_STACKTOP + SEC_STACK_SIZE;
 sec_stacklimit = .;
}
