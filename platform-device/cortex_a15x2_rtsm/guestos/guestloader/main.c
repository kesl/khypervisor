#include <guestloader.h>
#include <log/uart_print.h>
#include <arch_types.h>
#if defined(LINUX_GUEST)
#include <loadlinux.h>
#endif
/**
* @brief Copying Guestloader next to guestimage
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
* @brief Copying guestos to start_addr
* @param start_addr Start address of guestos
*/
static void copy_guestos_to_start_addr(uint32_t start_addr)
{
    uint32_t *src = &guest_bin_start;
    uint32_t *end = &guest_bin_end;
    uint32_t *dst = (uint32_t *)start_addr;
    while (src < end)
        *dst++ = *src++;
}
#if defined(LINUX_GUEST)
/**
* @brief Load linux guest
* @param start_addr Start address of linux guest
*/
static void loadlinuxguest(uint32_t start_addr)
{
    /* copy loader next to linux guest */
    copy_loader_next_to_guest();
    /* copy zImage next to 0xA000_8000 */
    copy_guestos_to_start_addr(start_addr);
    /* Set atags for booting linux guest */
    loadlinux_setup_tags((uint32_t *) 0x80000000);
    /* Jump to zImage at start_addr */
    loadlinux_run_zImage(start_addr);
    /* The code flow must not reach here */
    uart_print("[loadlinuxguest] ERROR: CODE MUST NOT REACH HERE\n\r");
    while (1)
        ;
}
#else
/**
* @brief Load bmguest
* @param start_addr Start address of bmguest
*/
static void loadbmguest(uint32_t start_addr)
{
    /* copy loader next to linux bmguest */
    copy_loader_next_to_guest();
    /* copy bmguest next to 0x8000_0000 */
    copy_guestos_to_start_addr(start_addr);
    /* Jump to 0x8000_0000 */
    asm volatile ("mov pc, %0" : : "r" (start_addr) : "memory", "cc");
    /* The code flow must not reach here */
    uart_print("[loadbmguest] ERROR: CODE MUST NOT REACH HERE\n\r");
    while (1)
        ;
}
#endif
void main(void){
    uart_print("\n\r=== starting guestloader.\n\r");
#if defined(LINUX_GUEST)
    loadlinuxguest(0xA0008000);
#else
    loadbmguest(0x80000000);
#endif
}
