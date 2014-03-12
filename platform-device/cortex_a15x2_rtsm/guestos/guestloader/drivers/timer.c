#include <test/test_vtimer.h>
#include <log/uart_print.h>
#include <guestloader.h>
#include "timer.h"
#include "gic.h"

#define BOOT_COUNT  5
#define BOOT_RATIO  5

int bootcount = BOOT_COUNT * BOOT_RATIO;

/**
 * @brief   ISR of timer interrupt.
 */
void timer_handler(int irq, void *pregs, void *pdata)
{
    if (0 == (bootcount % BOOT_RATIO)) {
        uart_print("Hit any key to stop autoboot : ");
        uart_print_dec(bootcount / BOOT_RATIO);
        uart_print("\n");
    }
    if (bootcount == 0)
        guestloader_flag_autoboot(1);
    bootcount--;
}

void timer_init(void)
{
    /* Registers vtimer hander */
    gic_set_irq_handler(30, timer_handler, 0);
    /* enable receiving virtual timer interrupt */
    timer_enable();
}

void timer_disable(void)
{
    /* Disables receiving virtual timer interrupt. */
    vtimer_mask(1);
}

void timer_enable(void)
{
    /* Enables receiving virtual timer interrupt. */
    vtimer_mask(0);
}
