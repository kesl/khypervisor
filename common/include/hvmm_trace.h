#ifndef __HVMM_TRACE_H__
#define __HVMM_TRACE_H__

#include <log/uart_print.h>

#ifdef DEBUG
#define HVMM_TRACE_ENTER()              \
    do {                                \
        uart_print(__func__);           \
        uart_print("() - enter\n\r");   \
    } while (0)

#define HVMM_TRACE_EXIT()               \
    do {                                \
        uart_print(__func__);           \
        uart_print("() - exit\n\r");    \
    } while (0)

#define HVMM_TRACE_HEX32(label, value)  \
    do {                                \
        uart_print(label);              \
        uart_print_hex32(value);        \
        uart_print("\n\r");             \
    } while (0)
#else
#define HVMM_TRACE_ENTER()
#define HVMM_TRACE_EXIT()
#define HVMM_TRACE_HEX32(label, value)
#endif

#define hyp_abort_infinite() { while (1) ; }

#endif
