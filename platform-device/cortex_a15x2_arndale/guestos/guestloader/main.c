#include <guestloader.h>
#include <log/uart_print.h>
#include <arch_types.h>
#include <loadlinux.h>
/**
* @brief Copying Guestloader next to guestimage.
*/
static void copy_loader_next_to_guest(void)
{
    uint32_t *src = &loader_start;
    uint32_t *dst = &guest_bin_end;
    uint32_t *end = &guest_bin_start;
    uint32_t offset;
    while (src < end)
        *dst++ = *src++;
    offset = ((uint32_t)(&guest_bin_end - &loader_start) * sizeof(uint32_t));
    offset -= 4;
    asm volatile ("add  pc, pc, %0" : : "r" (offset) : "memory", "cc");
}
/**
* @brief Copying guestos to start_addr.
* @param start_addr Start address of guestos.
*/
static void copy_guestos_to_start_addr(uint32_t start_addr)
{
    uint32_t *src = &guest_bin_start;
    uint32_t *end = &guest_bin_end;
    uint32_t *dst = (uint32_t *)start_addr;
    while (src < end)
        *dst++ = *src++;
}
/**
* @brief Set tags and Load linux guest.
* @param start_addr Start address of linux guest.
*/
static void load_linuxguest(uint32_t start_addr)
{
    /* Set atags for booting linux guest */
    loadlinux_setup_tags((uint32_t *) 0x80000000);
    /* Jump to zImage at start_addr */
    loadlinux_run_zImage(start_addr);
}
/**
* @brief Load bmguest.
* @param start_addr Start address of bmguest.
*/
static void load_bmguest(uint32_t start_addr)
{
    /* Jump to 0x8000_0000 */
    asm volatile ("mov pc, %0" : : "r" (start_addr) : "memory", "cc");
}
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
    case GUEST_TYPE_RTOS:
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
