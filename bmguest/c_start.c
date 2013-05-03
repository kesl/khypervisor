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

void nrm_loop(void) 
{
	semi_write0("[nrm] starting...\n");
	int i = 0;
	for( i = 0; i < 10; i++ ) {
		semi_write0("[nrm] hello\n");
		/* World Switch to Secure through Secure Monitor Call Exception */
		asm ("smc #0");		/* -> sec_loop() */
	}
	semi_write0("[nrm] done\n");
}
