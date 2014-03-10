#include <test/test_vtimer.h>
#include <log/uart_print.h>
#include <guestloader.h>
#include <gic.h>
#include "timer.h"

#define BOOT_COUNT  5
#define BOOT_COUNT_RATIO  BOOT_COUNT
#define VTIMER_IRQ 30

int bootcount = BOOT_COUNT * BOOT_COUNT_RATIO;
/**
 * @brief Handler of timer interrupt.
 */
void timer_handler(int irq, void *pregs, void *pdata)
{
    if (0 == (bootcount % BOOT_COUNT_RATIO)) {
        uart_print("Hit any key to stop autoboot : ");
        uart_print_dec(bootcount / BOOT_COUNT_RATIO);
        uart_print("\n");
    }
    if (bootcount == 0)
        guestloader_flag_autoboot(1);
    bootcount--;
}

void timer_init(void)
{
    /* Registers vtimer hander */
    gic_set_irq_handler(VTIMER_IRQ, timer_handler, 0);
    /* Enables receiving virtual timer interrupt */
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
