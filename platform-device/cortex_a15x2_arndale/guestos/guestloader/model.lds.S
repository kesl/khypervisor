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
/*
 . = GUEST_START;
 guest_start = .;
 .guest : {GUEST_PATH}
 .= ALIGN(4);
 guest_end = .;
*/
#ifdef LINUX_GUEST
 . = GUEST_START;
 guest_start = .;
 guest_end = . + 0x500000;
#else
 . = GUEST_START;
 guest_start = .;
 guest_end = . + 0x200000;
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

 loader_end = .;

. = PHYS_STACK;
guestloader_stacktop_svc = .;
. = PHYS_STACK + 0x00400000;
guestloader_stacklimit_svc = .;
guestloader_stacktop_irq = .;
. = PHYS_STACK + 0x00800000;
guestloader_stacklimit_irq = .;
}
