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
#include "uart_print.h"

#ifdef __MONITOR_CALL_HVC__
#define hsvc_ping()	asm("hvc #0xFFFE")
#define hsvc_yield()	asm("hvc #0xFFFD")
#else
#define SWITCH_MANUAL() asm("smc #0")
#endif


void nrm_loop(void) 
{
	uart_print("[bmg] starting...\n\r");
	int i = 0;
	for( i = 0; i < 20; i++ ) {
		uart_print("[bmg] iteration "); uart_print_hex32( i ); uart_print( "\n\r" );

#ifdef __MONITOR_CALL_HVC__
		if (i & 0x1) {
			uart_print( "[bmg] hsvc_ping()\n\r" );
			hsvc_ping();		// hvc ping
			uart_print( "[bmg] returned from hsvc_ping() \n\r" );
		} else {
			uart_print( "[bmg] hsvc_yield()\n\r" );
			hsvc_yield();		// hvc manual switch
			uart_print( "[bmg] returned from hsvc_yield() \n\r" );
		}
#else
		SWITCH_MANUAL();	// -> sec_loop() in the monitor
#endif
	}
	uart_print("[bmg] done\n\r");
}
