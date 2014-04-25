#include <stdint.h>
#include <asm-arm_inline.h>
#include <log/uart_print.h>
#include <gic.h>
#include <test/tests.h>
#include <drivers/pwm_timer.h>
#include <log/print.h>

/* #define TESTS_ENABLE_PWM_TIMER */

int main()
{
    printh(GUEST_LABEL);
    printh("=== Starting platform main n\r");
#ifdef TESTS_ENABLE_PWM_TIMER
    hvmm_tests_pwm_timer();
#endif
    while (1)
        ;
    return 0;
}
