/*
 * Copyright (C) 2013 KESL. All rights reserved.
 */
    .text
    @ Guest loader start code
.global guestloader_start
guestloader_start:
    /* Stack pointer initialization */
    @ sp
    ldr     sp, =guestloader_stacklimit_svc

    @ sp for irq
    msr     cpsr_c, #0xd2
    ldr     sp, = guestloader_stacklimit_irq

    @ sp for guest(system / user)
    msr     cpsr_c, #0xdf
    ldr     sp, = guestloader_stacklimit

    @ Call the C entrypoint
    b   main

.type guestloader_start, %function

