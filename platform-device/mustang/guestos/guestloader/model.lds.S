/*
 * model.lds.S - simple linker script for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */

OUTPUT_FORMAT("elf64-littleaarch64")
/*OUTPUT_ARCH(arm)*/
TARGET(binary)
#include "memmap.cfg"

#ifdef USE_ANDROID_INITRD
INPUT(INITRD_PATH)
#endif
#ifdef _MON_
/*INPUT(../../guestimages/System.map)*/
#endif

PHYS_STACK  = LDS_STACK;
#ifdef LINUX_GUEST
GUEST_START = LDS_ZIMAGE_START;
#else
GUEST_START = LDS_GUEST_START;
#endif
LOADER_PHYS_START = LDS_LOADER_PHYS_START;

SECTIONS
{
#ifdef LINUX_GUEST
 . = GUEST_START;
 guest_start = .;
 guest_end = . + 0xA00000;
#else
 . = GUEST_START;
 guest_start = .;
 guest_end = . + 0x200000;
#endif

 #ifdef USE_ANDROID_INITRD
 . = 0x400D00000;
 initrd_start = .;
 .androidrfs : { INITRD_PATH }
 initrd_end = .;
 #endif

 #ifdef _MON_
 system_map_start = 0x4002000000;
 . = system_map_start;
 /*.systemmap : {../../guestimages/System.map}*/
 system_map_end = .;
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

 . = ALIGN(4);

 shared_memory_start = 0x400EC00000;
 . = shared_memory_start;
 shared_memory_end = . + 0x400000;

 loader_end = .;
 guestloader_end = .;

 . = PHYS_STACK;
 loader_stacktop = .;
 . = PHYS_STACK + 0x00800000;
 loader_stacklimit = .;

 restore_start = 0x4040000000;
 restore_guest_start = restore_start + 0x00500000;
 restore_guest_end = restore_guest_start + (guest_end - guest_start);
 restore_end = restore_start + (guestloader_end - loader_start);
}
