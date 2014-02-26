/*
 * Copyright (C) 2013 KESL. All rights reserved.
 */
    .syntax unified
    .arch_extension sec
    .arch_extension virt
    .text
    @ Guest start code
.global guest_start
guest_start:
    /* Stack pointer initialization */
    @ sp  
    ldr     sp, =guest_stacktop
    b   main
.type guest_start, %function

