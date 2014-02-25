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
    /* Stack pointer initialization for svc, irq, and system/user modes */
    @ sp for guest svc mode
    ldr     sp, =guest_stacklimit_svc

    b   main
.type guest_start, %function

