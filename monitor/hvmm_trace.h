#ifndef __HVMM_TRACE_H__
#define __HVMM_TRACE_H__
#include "uart_print.h"

#define HVMM_TRACE_ENTER()	{uart_print( __FUNCTION__ );  uart_print("() - enter\n\r");}
#define HVMM_TRACE_EXIT()	{uart_print( __FUNCTION__ );  uart_print("() - exit\n\r");}


#endif
