/*
 * boot.S - EL2/EL1 Switchinr mode
 *
 * Copyright (C) 2015 KESL. All rights reserved.
 *
 */
.text

#define EL2T 0x08
#define EL2H 0x09
// Guest start code
.global guest_start
guest_start:
    msr daifset, 0x4
    mrs x10, currentEL
    cmp x10, #EL2T
    ccmp x10, #EL2H, #0x4, ne
    b.ne common_guest_entry

go_el1:
    mov x0, 0x05 //EL1H
    ldr x1, =common_guest_entry
    msr spsr_el2, x0
    msr elr_el2, x1
    eret

b   common_guest_entry

.type guest_start, % function
