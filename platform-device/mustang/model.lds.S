/*
 * model.lds.S - simple linker script for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */

OUTPUT_FORMAT("elf64-littleaarch64")
/*OUTPUT_ARCH(armv8-a)*/
TARGET(binary)
ENTRY(start)

#include <k-hypervisor-config.h>

/*INPUT(./guestimages/guest0.bin)
INPUT(./guestimages/guest1.bin)*/
#ifdef _SMP_
/*INPUT(./guestimages/guest2.bin)
INPUT(./guestimages/guest3.bin)*/
#endif

MON_STACK   = CFG_MEMMAP_MON_OFFSET + MON_SIZE;
SEC_STACKTOP = MON_STACK + MON_STACK_SIZE;

/* NS.SVC mode code space */
GUEST0_STACK     = CFG_MEMMAP_GUEST0_OFFSET + GUEST_SIZE_MAX;
GUEST1_STACK    = CFG_MEMMAP_GUEST1_OFFSET + GUEST_SIZE_MAX;
#ifdef _SMP_
GUEST2_STACK    = CFG_MEMMAP_GUEST2_OFFSET + GUEST_SIZE_MAX;
GUEST3_STACK    = CFG_MEMMAP_GUEST3_OFFSET + GUEST_SIZE_MAX;
#endif
SECTIONS
{
 . = CFG_MEMMAP_PHYS_START;
 . = CFG_MEMMAP_PHYS_START + 0x8000 - 0x40;
 . = CFG_MEMMAP_PHYS_START + 0x00d00000;

 fs_start = .;
 fs_end = .;

/* Guest 0 */
 . = CFG_MEMMAP_GUEST0_OFFSET;
  _guest0_bin_start = .;
/*.guest0 : { ./guestimages/guest0.bin}*/
  _guest0_bin_end = .;

 . = GUEST0_STACK;
 guest0_stacktop = .;
 . = GUEST0_STACK + 0x01000000;
 guest0_stacklimit = .;

/* Guest 1 */
 . = CFG_MEMMAP_GUEST1_OFFSET;
  _guest1_bin_start = .;
/*.guest1 : { ./guestimages/guest1.bin } */
  _guest1_bin_end = .;

 . = GUEST1_STACK;
 guest1_stacktop = .;
 . = GUEST1_STACK + 0x01000000;
 guest1_stacklimit = .;

#ifdef _SMP_
/* Guest 2 */
 . = CFG_MEMMAP_GUEST2_OFFSET;
  _guest2_bin_start = .;
 /*.guest2 : { ./guestimages/guest2.bin }*/
  _guest2_bin_end = .;

 . = GUEST2_STACK;
 guest2_stacktop = .;
 . = GUEST2_STACK + 0x01000000;
 guest2_stacklimit = .;

/* Guest 3 */
 . = CFG_MEMMAP_GUEST3_OFFSET;
  _guest3_bin_start = .;
 .guest3 : { ./guestimages/guest3.bin }
  _guest3_bin_end = .;

 . = GUEST3_STACK;
 guest3_stacktop = .;
 . = GUEST3_STACK + 0x01000000;
 guest3_stacklimit = .;
#endif

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
