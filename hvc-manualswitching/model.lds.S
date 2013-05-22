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
	INPUT(./bmguest2.bin)
#endif

PHYS_OFFSET = 0x80000000;
MON_OFFSET  = 0xf0000000;
STACKTOP = 0xff000000;

/* NS.SVC mode code space 
 * - simon
 */
GUEST_OFFSET  	= 0xB0000000;
GUEST_STACK  	= 0xBF000000;

GUEST2_OFFSET	= 0xC0000000;
GUEST2_STACK	= 0xCF000000;

HYP_OFFSET	= 0xD0000000;
HYP_STACK	= 0xDF000000;

SECTIONS
{
 . = PHYS_OFFSET;

 . = PHYS_OFFSET + 0x8000 - 0x40;

 kernel_start = .;
 kernel_entry = . + 0x40;
#ifndef SEMIHOSTING
 .kernel : { ./uImage }
#endif
 kernel_end = .;


 . = PHYS_OFFSET + 0x00d00000;
 fs_start = .;
#if defined(USE_INITRD) && !defined(SEMIHOSTING)
 .filesystem : { ./filesystem.cpio.gz }
#endif
 fs_end = .;

/* Guest2 image */
 . = GUEST2_OFFSET;
#ifdef BAREMETAL_GUEST
  guest2_bin_start = .; 
  .guest2 : { ./bmguest2.bin }
  guest2_bin_end = .; 
#endif
 . = GUEST2_STACK;
 guest2_stacktop = .;
 . = GUEST2_STACK + 0x01000000; 
 guest2_stacklimit = .;


/* Guest image between Kernel and Mon */
 . = GUEST_OFFSET;
#ifdef BAREMETAL_GUEST
  guest_bin_start = .; 
  .guest : { ./bmguest.bin }
  guest_bin_end = .; 
#endif

 . = GUEST_STACK;
 guest_stacktop = .;
 . = GUEST_STACK + 0x01000000; 
 guest_stacklimit = .;

/* Hyp image follows */
 . = HYP_OFFSET;
 hyp_start = .;
 . = HYP_STACK;
 hyp_stacktop = .;
 . = HYP_STACK + 0x01000000;
 hyp_stacklimit = .;

 . = MON_OFFSET;
 /* Put most of the actual boot loader code up in high memory
  * where it won't get overwritten by kernel, initrd or atags.
  */
 .text : { 
	boot.o(.text) 
	monitor.o(.text) 
	/* guest.o(.text) */
 }
 .data : { 
	boot.o(.data) 
	monitor.o(.data) 
	/* guest.o(.data)  */
 }
 .bss : { 
	boot.o(.bss) 
	monitor.o(.bss) 
	/* guest.o(.bss)  */
 }

 . = STACKTOP;
 stacktop = .;
}
