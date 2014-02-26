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
PHYS_OFFSET = LDS_PHYS_OFFSET;
PHYS_STACK  = LDS_STACK;

SECTIONS
{
 . = PHYS_OFFSET;

 loader_start = .;

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

 . = PHYS_STACK;
 guest_stacktop = .;
 . = PHYS_STACK - 0x00400000; 
 guest_stacklimit = .;
}
