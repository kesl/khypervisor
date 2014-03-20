#include <gic.h>
#include <cli.h>
#include <version.h>
#include <asm-arm_inline.h>
#include <log/uart_print.h>
#include "drivers/timer.h"

#define MAX_CMD_STR_SIZE    256
#define PROMPT  "kboot# "
#define BOOTCMD "boot"

int autoboot;

void guestloader_init(void)
{
    /* Initializes serial */
    uart_init();
    /* Initializes GIC */
    gic_init();
    /* Ready to accept irqs with GIC. Enable it now */
    irq_enable();
    /* Initializes timer */
    timer_init();
    /* Initializes autoboot flag */
    autoboot = 1;
}

void guestloader_flag_autoboot(int flag)
{
    autoboot = flag;
}

void guestloader_autoboot(void)
{
    /* Disable timer for guest os */
    timer_disable();
    cli_exec_cmd(BOOTCMD);
}

void guestloader_cliboot(void)
{
    char line[MAX_CMD_STR_SIZE];
    /* Disable timer for guest os */
    timer_disable();
    while (1) {
        uart_print(PROMPT);
        uart_gets(line, MAX_CMD_STR_SIZE);
        cli_exec_cmd(line);
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
