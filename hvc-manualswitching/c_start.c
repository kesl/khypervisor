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

#include <stdint.h>
#include "semihosting.h"
#include "monitor.h"



void c_start(void)
{
	semi_write0("[secure] Starting...\n");

	/* Initialize Monitor by installing Secure Monitor Call Execption handler */
	mon_init();

	mon_enter_hyp();	// Secure -> NS.Hyp -> hyp_main()

	/* ___ DEAD END ___ */
}
#ifndef BAREMETAL_GUEST
void nrm_loop(void) 
{
	int i = 0;
	//semi_write0("[nrm] enter\n");
	for( i = 0; i < 10; i++ ) {
		//semi_write0("[nrm] hello\n");
		/* World Switch to Secure through Secure Monitor Call Exception */
		asm ("hvc #0xFFFE");		/* -> sec_loop() */
	}
	//semi_write0("[nrm] done\n");
}
#endif
