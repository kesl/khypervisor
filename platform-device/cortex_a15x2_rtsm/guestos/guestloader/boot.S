/*
 * Copyright (C) 2013 KESL. All rights reserved.
 */
    .text
    @ Guest loader start code
.global guestloader_start
guestloader_start:
    /* Stack pointer initialization */
    @ sp  
    ldr     sp, =guestloader_stacktop
    b   main
.type guestloader_start, %function

