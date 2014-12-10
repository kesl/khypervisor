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
#ifdef LINUX_GUEST
 . = GUEST_START;
 loader_end = .;
 guest_start = .;
 guest_end = . + 0x500000;
#else
 . = GUEST_START;
 loader_end = .;
 guest_start = .;
 guest_end = . + 0x200000;
#endif

 system_map_start = 0x40D00000;
 . = system_map_start;
 system_map_end = .;

 shared_memory_start = 0x4EC00000;
 . = shared_memory_start;
 shared_memory_end = . + 0x400000;

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

 guestloader_end = .;

. = PHYS_STACK;
guestloader_stacktop_svc = .;
. = PHYS_STACK + 0x00400000;
guestloader_stacklimit_svc = .;
guestloader_stacktop_irq = .;
. = PHYS_STACK + 0x00800000;
guestloader_stacklimit_irq = .;

 restore_start = 0x70000000;
 restore_guest_start = restore_start + 0x00500000;
 restore_guest_end = restore_guest_start + (guest_end - guest_start);
 restore_end = restore_start + (guestloader_end - loader_start);

}
