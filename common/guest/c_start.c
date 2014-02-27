/*
 * Copyright (c) 2012 Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Linaro Limited nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 */

/* This file just contains a small glue function which fishes the
 * location of kernel etc out of linker script defined symbols, and
 * calls semi_loader functions to do the actual work of loading
 * and booting the kernel.
 */

#include <stdint.h>
#include <asm-arm_inline.h>
#include <log/uart_print.h>
#include <gic.h>
#include <test/tests.h>

/* #define TESTS_ENABLE_VDEV_SAMPLE */

#ifdef __MONITOR_CALL_HVC__
#define hsvc_ping()     asm("hvc #0xFFFE")
#define hsvc_yield()    asm("hvc #0xFFFD")
#else
#define SWITCH_MANUAL() asm("smc #0")
#endif

#ifndef GUEST_LABEL
#define GUEST_LABEL "[guest0] "
#endif

#ifndef NUM_ITERATIONS
#define NUM_ITERATIONS 10
#endif

inline void nrm_delay(void)
{
    volatile int i = 0;
    for (i = 0; i < 0x0000FFFF; i++)
        ;
}

void nrm_loop(void)
{
    int i = 0;
    uart_init();
    uart_print(GUEST_LABEL);
    uart_print("=== Starting commom start up\n\r");
    gic_init();
    /* We are ready to accept irqs with GIC. Enable it now */
    irq_enable();
    /* Test the sample virtual device.
     * - Uncomment the following line of code only if 'vdev_sample' is
     *   registered at the monitor.
     * - Otherwise, the monitor will hang with data abort.
     */
#ifdef TESTS_ENABLE_VDEV_SAMPLE
    test_vdev_sample();
#endif
    for (i = 0; i < NUM_ITERATIONS; i++) {
        uart_print(GUEST_LABEL);
        uart_print("iteration ");
        uart_print_hex32(i);
        uart_print("\n\r");
        nrm_delay();
#ifdef __MONITOR_CALL_HVC__
        /* Hyp monitor guest run in Non-secure supervisor mode.
         Request test hvc ping and yield one after another
         */
        if (i & 0x1) {
            uart_print(GUEST_LABEL);
            uart_print("hsvc_ping()\n\r");
            hsvc_ping();
            uart_print(GUEST_LABEL);
            uart_print("returned from hsvc_ping()\n\r");
        } else {
            uart_print(GUEST_LABEL);
            uart_print("hsvc_yield()\n\r");
            hsvc_yield();
            uart_print(GUEST_LABEL);
            uart_print("returned from hsvc_yield()\n\r");
        }
#else
        /* Secure monitor guest run in Non-secure supervisor mode
         Request for switch to Secure mode (sec_loop() in the monitor)
         */
        SWITCH_MANUAL();
#endif
        nrm_delay();
    }
    uart_print(GUEST_LABEL);
    uart_print("common nrm_loop done\n\r");
    uart_print("\n[K-HYPERVISOR]TEST#INSTALLATION#BMGUEST-BOOT#PASS\n\r");
    /* start platform start up code */
    main();
}
