#include <stdint.h>
#include <asm-arm_inline.h>
#include <log/uart_print.h>
#include <gic.h>
#include <test/tests.h>
#include <drivers/sp804_timer.h>

//#define TESTS_ENABLE_SP804_TIMER

int main()
{
    uart_print(GUEST_LABEL); uart_print("\n\r=== Starting platform main \n\r");

#ifdef TESTS_ENABLE_SP804_TIMER
    /* Test the SP804 timer */
    hvmm_tests_sp804_timer();
#endif

    while (1)
        ;

    return 0;
}
