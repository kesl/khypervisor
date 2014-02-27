#include <arch_types.h>
#include <guestloader.h>
#include <log/uart_print.h>
#include <loadlinux.h>
#include <loadguest.h>
/**
* @brief Load guest.
* @param type Guest type. There are bmguest and Linux guest.
*/
static void load_guest(uint32_t type)
{
    copy_loader_next_to_guest();
    copy_guestos_to_start_addr(START_ADDR);
    switch (type) {
    case GUEST_TYPE_BM:
        load_bmguest(START_ADDR);
        break;
    case GUEST_TYPE_LINUX:
        load_linuxguest(START_ADDR);
        break;
    default:
        uart_print("[loadbmguest] ERROR: CANT'T NOT MATCH GUEST TYPE\n\r");
        break;
    }
    /* The code flow must not reach here */
    uart_print("[loadbmguest] ERROR: CODE MUST NOT REACH HERE\n\r");
    while (1)
        ;
}
void main(void)
{
    uart_print("\n\r=== starting guestloader.\n\r");
    load_guest(GUEST_TYPE);
}
