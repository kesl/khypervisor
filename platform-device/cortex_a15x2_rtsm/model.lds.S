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

#include <k-hypervisor-config.h>

INPUT(./guestimages/guest0.bin)
INPUT(./guestimages/guest1.bin)


MON_STACK   = CFG_MEMMAP_MON_OFFSET + MON_SIZE;
SEC_STACKTOP = MON_STACK + MON_STACK_SIZE;

/* NS.SVC mode code space */
GUEST_STACK     = CFG_MEMMAP_GUEST_OFFSET + GUEST_SIZE_MAX;
GUEST2_STACK    = CFG_MEMMAP_GUEST2_OFFSET + GUEST_SIZE_MAX;

SECTIONS
{
 . = CFG_MEMMAP_PHYS_START;
 . = CFG_MEMMAP_PHYS_START + 0x8000 - 0x40;
 . = CFG_MEMMAP_PHYS_START + 0x00d00000;

 fs_start = .;
 fs_end = .;

/* Guest 1 */
 . = CFG_MEMMAP_GUEST_OFFSET;
  _guest_bin_start = .;
  .guest : { ./guestimages/guest0.bin}
  _guest_bin_end = .;

 . = GUEST_STACK;
 guest_stacktop = .;
 . = GUEST_STACK + 0x01000000;
 guest_stacklimit = .;

/* Guest 2 */
 . = CFG_MEMMAP_GUEST2_OFFSET;
  _guest2_bin_start = .;
 .guest2 : { ./guestimages/guest1.bin }
  _guest2_bin_end = .;

 . = GUEST2_STACK;
 guest2_stacktop = .;
 . = GUEST2_STACK + 0x01000000;
 guest2_stacklimit = .;

 . = CFG_MEMMAP_MON_OFFSET;
 /* Put most of the actual boot loader code up in high memory
  * where it won't get overwritten by kernel, initrd or atags.
  */
 .text : {
    *(.text);
    __vdev_module_high_start = .;
    *(.vdev_module0.init);
    __vdev_module_high_end = .;
    *(.vdev_module1.init);
    __vdev_module_middle_end = .;
    *(.vdev_module2.init);
    __vdev_module_low_end = .;
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

 . = MON_STACK;
 mon_stacktop = .;
 . = MON_STACK + MON_STACK_SIZE;
 mon_stacklimit = .;

 . = SEC_STACKTOP;
 sec_stacktop = .;
 . = SEC_STACKTOP + SEC_STACK_SIZE;
 sec_stacklimit = .;
}
