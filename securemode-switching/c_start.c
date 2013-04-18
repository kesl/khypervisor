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
#include "semihosting.h"
#include "semi_loader.h"

/* Linker script defined symbols for any preloaded kernel/initrd */
extern uint8_t fs_start, fs_end, kernel_entry, kernel_start, kernel_end;
/* Symbols defined by boot.S */
extern uint8_t kernel_cmd, kernel_cmd_end;

static struct loader_info loader;

#ifdef MACH_MPS
#define PLAT_ID 10000 /* MPS (temporary) */
#elif defined (VEXPRESS)
#define PLAT_ID 2272 /* Versatile Express */
#else
#define PLAT_ID 827 /* RealView/EB */
#endif

extern void monitor_init(void);
extern void monitor_smc(void);

void sec_loop(void);
void nrm_loop(void) ;

void c_start(void)
{
	semi_write0("[bootwrapper] Starting...\n");

	/* Initialize Monitor by installing Secure Monitor Call Execption handler */
	monitor_init();

	/* Begin with Secure world loop */
	sec_loop();

}

void sec_loop(void)
{
	int i = 0;
	for( i = 0; i < 10; i++ ) {
		semi_write0("[sec] hello\n");
		/* World Switch to Non-Secure through Secure Monitor Call Exception */
		asm ("smc #0");		/* -> guest_start: or nrm_loop() */
	}
	semi_write0("[sec] done\n");

	/* Give the last turn to nrm_loop() to execute it's the last line of the code */
	asm ("smc #0");
}

void nrm_loop(void) 
{
	int i = 0;
	for( i = 0; i < 10; i++ ) {
		semi_write0("[nrm] hello\n");
		/* World Switch to Secure through Secure Monitor Call Exception */
		asm ("smc #0");		/* -> sec_loop() */
	}
	semi_write0("[nrm] done\n");
}
