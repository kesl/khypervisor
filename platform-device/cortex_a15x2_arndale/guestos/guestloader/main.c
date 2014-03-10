#include <arch_types.h>
#include <guestloader.h>
#include <log/uart_print.h>
#include <guestloader_common.h>
void main(void)
{
    uart_print("\n\r=== starting guestloader.\n\r");
    loader_boot_guest(GUEST_TYPE);
}
