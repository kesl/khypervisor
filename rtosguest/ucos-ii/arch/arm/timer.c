#include <stdio.h>
#include <ucos_ii.h>

#include <asm-arm/irq.h>
#include <asm-arm/timer.h>
#include <asm-arm/arch_types.h>

static void do_timer_interrupt()
{
    OSTimeTick();
}

void init_time(void)
{
    request_irq(30, do_timer_interrupt, 0, "timer", NULL);
}
