#include <gic.h>
#include <cli.h>
#include <version.h>
#include <asm-arm_inline.h>
#include <log/uart_print.h>
#include "drivers/timer.h"
#include <guestloader_common.h>
#define DEBUG
#include <log/print.h>
#include <guestloader_common.h>
#include <guest_monitor.h>
#include "drivers/gpio.h"

#define MAX_CMD_STR_SIZE    256
#define PROMPT  "kboot# "
#define BOOTCMD "boot"

volatile int autoboot;

static void guestloader_init(void)
{
    /* Initializes serial */
    uart_init();
    /* Initializes GIC */
    gic_init();
#ifdef _MON_
    monitoring_init();
#endif
    /* Ready to accept irqs with GIC. Enable it now */
    irq_enable();
    /* Initializes timer */
    timer_init();
    /* Initializes autoboot flag */
    autoboot = 0;
}

void guestloader_flag_autoboot(int flag)
{
    autoboot = flag;
}

static void guestloader_autoboot(void)
{
    timer_disable();
    cli_exec_cmd(BOOTCMD);
}

static void guestloader_cliboot(void)
{
    char input_cmd[MAX_CMD_STR_SIZE];
    /* Disable timer for guest os */
    timer_disable();

    while (1) {
        uart_print(PROMPT);
        uart_gets(input_cmd, MAX_CMD_STR_SIZE);
        cli_exec_cmd(input_cmd);
    }
}

void main(int boot_status)
{
#if _SMP_
    while (1)
        ;
#endif
    /*If Booting status is reboot, run this function. */
    if (boot_status)
        reboot();
    /* Initializes guestloder */
    guestloader_init();
    /* Show Hypervisor Banner */
    uart_print(BANNER_STRING);

#ifdef MONITOR_GUEST
    guestloader_cliboot();
#endif
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
