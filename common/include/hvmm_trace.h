#ifndef __HVMM_TRACE_H__
#define __HVMM_TRACE_H__

#include <log/uart_print.h>

#ifdef DEBUG
#define HVMM_TRACE_ENTER()  {uart_print(__FUNCTION__);  uart_print("() - enter\n\r"); }
#define HVMM_TRACE_EXIT()   {uart_print(__FUNCTION__);  uart_print("() - exit\n\r"); }
#define HVMM_TRACE_HEX32(label, value)      {uart_print(label); uart_print_hex32(value); uart_print("\n\r"); }
#else
#define HVMM_TRACE_ENTER()
#define HVMM_TRACE_EXIT()
#define HVMM_TRACE_HEX32(label, value)
#endif

#define hyp_abort_infinite() { while (1) ; }

#endif
