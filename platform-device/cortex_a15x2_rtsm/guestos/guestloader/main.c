#include <gic.h>
#include <command.h>
#include <version.h>
#include <asm-arm_inline.h>
#include <log/uart_print.h>
#include "drivers/timer.h"

int autoboot;

void guestloader_init(void)
{
    /* Initializes serial */
    uart_init();
    /* Initializes GIC */
    gic_init();
    /* Ready to accept irqs with GIC. Enable it now */
    irq_enable();
    /* Enable timer */
    timer_init();
    /* Initializes autoboot flag */
    autoboot = 0;
}

void guestloader_flag_autoboot(int flag)
{
    autoboot = flag;
}

void guestloader_autoboot(void)
{
    /* Disable timer timer for guest os */
    timer_disable();
    command_exec("boot");
}

#define MAX_CMD_STR_SIZE    256
void guestloader_cliboot(void)
{
    char line[MAX_CMD_STR_SIZE];
    /* Disable timer timer for guest os */
    timer_disable();
    while (1) {
        uart_print("kboot# ");
        uart_gets(line, MAX_CMD_STR_SIZE, '\n');
        command_exec(line);
    }
}

void main(void)
{
    /* Initializes guestloder */
    guestloader_init();
    /* Show Hypervisor Banner */
    uart_print(BANNER_STRING);
    /* Auto boot or CLI boot */
    while (1) {
        /* Auto boot */
        if (autoboot)
            guestloader_autoboot();
        /* Use CLI, if press any key */
        else if (uart_tst_fifo())
            guestloader_cliboot();
    }
}
