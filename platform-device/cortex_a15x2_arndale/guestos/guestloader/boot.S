/*
 * Copyright (C) 2013 KESL. All rights reserved.
 */
    .text
    @ Guest loader start code
.global guestloader_start
guestloader_start:
    /* Stack pointer initialization */
    @ sp for irq
    msr     cpsr_c, #0xd2
    ldr     sp, = guestloader_stacklimit_irq

    @ sp for svc
    msr     cpsr_c, #0xd3
    ldr     sp, =guestloader_stacklimit_svc


    @ Call the C entrypoint
    b   main

.type guestloader_start, %function

