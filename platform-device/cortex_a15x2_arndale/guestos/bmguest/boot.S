/*
 * boot.S - Secure/Non-Secure Switching Monitor
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */
.syntax unified
.arch_extension sec
.arch_extension virt
.text

@ Guest start code
.global guest_start
guest_start:

b   common_guest_entry
.type guest_start, % function
