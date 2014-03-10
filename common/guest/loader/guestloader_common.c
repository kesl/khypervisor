#include <guestloader.h>
#include <arch_types.h>
#include <linuxloader.h>
#include <guestloader_common.h>

#define SET_MACHINE_TYPE_TO_R0() \
    asm volatile ("mov r1, %0" : : "r" (MACHINE_TYPE) : "memory", "cc")
#define SET_ATAGS_TO_R1() \
    asm volatile \
    ("mov r2, %0" : : "r" (linuxloader_get_atags_addr()) : "memory", "cc")
#define JUMP_TO_ADDRESS(addr) \
    asm volatile ("mov pc, %0" : : "r" (addr) : "memory", "cc")
#define ADD_IR_TO_OFFSET(offset) \
    asm volatile ("add  lr, lr, %0" : : "r" (offset) : "memory", "cc")
#define ADD_PC_TO_OFFSET(offset) \
    asm volatile ("add  pc, pc, %0" : : "r" (offset) : "memory", "cc")

/**
* @brief Relocates a guestloader to next to a guestimage.
*/
void relocate_guestloader_to_next_to_guest(void)
{
    uint32_t *src = &loader_start;
    uint32_t *dst = &guest_bin_end;
    uint32_t *end = &guest_bin_start;
    uint32_t offset;

    while (src < end)
        *dst++ = *src++;
    offset = ((uint32_t)(&guest_bin_end - &loader_start) * sizeof(uint32_t));
    ADD_IR_TO_OFFSET(offset);
    offset -= 4;
    ADD_PC_TO_OFFSET(offset);
}

/**
* @brief Relocates a guest to address start_addr.
* @param start_addr The start address of the guest os.
*/
void relocate_guest_to_address(uint32_t start_addr)
{
    uint32_t *src = &guest_bin_start;
    uint32_t *end = &guest_bin_end;
    uint32_t *dst = (uint32_t *)start_addr;

    while (src < end)
        *dst++ = *src++;
}

void loader_boot_guest(uint32_t guest_type)
{
    uart_print("Booting guest os...\n");

    relocate_guestloader_to_next_to_guest();
    relocate_guest_to_address(START_ADDR);

    switch (guest_type) {
    case GUEST_TYPE_LINUX:
        linuxloader_setup_atags(START_ADDR_LINUX);
        /* r0 : machine number
         * r1 : atags address
         */
        SET_MACHINE_TYPE_TO_R0();
        SET_ATAGS_TO_R1();
        break;
    }
    /* Jump to start address of guest
     * Linux (zImage) : 0xA000_8000
     * RTOS           : 0x8000_0000
     * BMGUEST        : 0x8000_0000
     */
    JUMP_TO_ADDRESS(START_ADDR);

    /* The code flow must not reach here */
    uart_print("[loadbmguest] ERROR: CODE MUST NOT REACH HERE\n\r");
    while (1)
        ;
}
