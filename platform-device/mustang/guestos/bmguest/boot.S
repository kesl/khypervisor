/*
 * boot.S - Secure/Non-Secure Switching Monitor
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */
.text

// Guest start code
.global guest_start
guest_start:
ldr x0, =0x1c020000
ldr x1, =0x35
str x1, [x0]
b   common_guest_entry
.type guest_start, % function
