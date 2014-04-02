#include <asm-arm_inline.h>
#include <log/uart_print.h>
#include <gic.h>
#include <test/tests.h>
#include <trap.h>
#include <drivers/sp804_timer.h>

/*
#define TESTS_ENABLE_SP804_TIMER
*/
#define TESTS_TRAP_WFI
#define TESTS_TRAP_SMC
#define TESTS_TRAP_SCTLR
#define TESTS_TRAP_DDCISW
#define TESTS_TRAP_ACTLR

int main()
{
    int val;
    uart_print(GUEST_LABEL);
    uart_print("\n\r=== Starting platform main\n\r");
#ifdef TESTS_ENABLE_SP804_TIMER
    /* Test the SP804 timer */
    hvmm_tests_sp804_timer();
#endif
#ifdef TESTS_TRAP_WFI
    WFI();
    WFE();
#endif
#ifdef TESTS_TRAP_SMC
    SVC();
/*    SMC(); */
#endif
#ifdef TESTS_TRAP_SCTLR
    WRITE_SCTLR(val);
    READ_SCTLR(val);
#endif
#ifdef TESTS_TRAP_DDCISW
    WRITE_DCCISW(val);
#endif
#ifdef TESTS_TRAP_ACTLR
    WRITE_ACTLR(val);
    READ_ACTLR(val);
#endif

    while (1)
        ;
    return 0;
}
