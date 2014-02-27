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
INPUT(GUEST_PATH)
PHYS_STACK  = LDS_STACK;
GUEST_START = LDS_GUEST_START;
LOADER_PHYS_START = LDS_LOADER_PHYS_START;
SECTIONS
{
 . = GUEST_START;
 guest_bin_start = .;
 .guest : {GUEST_PATH}
 guest_bin_end = .;
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
