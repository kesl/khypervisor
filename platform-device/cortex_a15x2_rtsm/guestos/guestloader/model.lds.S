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
#include "memmap.cfg"
#ifdef LINUX_GUEST
INPUT(../../guestimages/zImage)
#else
INPUT(../../guestimages/bmguest.bin)
#endif
PHYS_STACK  = LDS_STACK;
GUEST_START = LDS_GUEST_START;
LOADER_PHYS_START = LDS_LOADER_PHYS_START;
SECTIONS
{
 . = GUEST_START;
 #ifdef LINUX_GUEST
 guest_bin_start = .;
 .guest : { ../../guestimages/zImage}
 guest_bin_end = .;
 #else
 guest_bin_start = .;
 .guest : { ../../guestimages/bmguest.bin}
 guest_bin_end = .;
 #endif
 . = LOADER_PHYS_START;
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
 . = PHYS_STACK;
 guestloader_stacktop = .;
 . = PHYS_STACK - 0x00400000;
 guestloader_stacklimit = .;
}
