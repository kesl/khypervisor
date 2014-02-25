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

#ifdef LINUX_GUEST
INPUT(../../guestimages/zImage)
#else
INPUT(../../guestimages/bmguest.bin)
#endif

#ifdef LDS_PHYS_OFFSET
PHYS_OFFSET = LDS_PHYS_OFFSET;
#else
PHYS_OFFSET = 0x80000000;
#endif

/* NS.SVC mode code space 
 * LDS_GUEST_OFFSET ~ LDS_GUEST_STACK + 0x01000000
 * - simon
 */
#ifdef LDS_GUEST_OFFSET
GUEST_OFFSET  	= LDS_GUEST_OFFSET;
#else
GUEST_OFFSET  	= 0x80000000;
#endif

#ifdef LDS_GUEST_STACK
GUEST_STACK  	= LDS_GUEST_STACK;
#else
GUEST_STACK  	= 0x8F000000;
#endif

SECTIONS
{
 . = PHYS_OFFSET;

/* Guest image between Kernel and Mon */
 . = GUEST_OFFSET;

 loader_start = .;

 /* Put most of the actual boot loader code up in high memory
  * where it won't get overwritten by kernel, initrd or atags.
  */
 .text : { 
    *(.text)
 }
 .= ALIGN(4);
 .rodata : {
    *(.rodata)
 }
 .= ALIGN(4);
 .data : { 
    *(.data)
 }
 .= ALIGN(4);
 begin_bss = .;
 .bss : { 
    *(.bss)
 }
 end_bss = .;

 #ifdef LINUX_GUEST
 guest_bin_start = _binary_______guestimages_zImage_start;
 guest_bin_end = _binary_______guestimages_zImage_end;
 guest_bin_size = _binary_______guestimages_zImage_size;
 #else
 guest_bin_start = _binary_______guestimages_bmguest_bin_start;
 guest_bin_end = _binary_______guestimages_bmguest_bin_end;
 guest_bin_size = _binary_______guestimages_bmguest_bin_size;
 #endif

 . = GUEST_STACK;
 guest_stacktop_svc = .;
 . = GUEST_STACK + 0x00400000; 
 guest_stacklimit_svc = .;
 guest_stacktop = .;
 . = GUEST_STACK + 0x01000000; 
 guest_stacklimit = .;
}
