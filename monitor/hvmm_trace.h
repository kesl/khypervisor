#ifndef __HVMM_TRACE_H__
#define __HVMM_TRACE_H__
#include "uart_print.h"

#define HVMM_TRACE_ENTER()	{uart_print( __FUNCTION__ );  uart_print("() - enter\n\r");}
#define HVMM_TRACE_EXIT()	{uart_print( __FUNCTION__ );  uart_print("() - exit\n\r");}
#define HVMM_TRACE_HEX32(label, value)      {uart_print(label); uart_print_hex32(value); uart_print("\n\r");}

#endif
