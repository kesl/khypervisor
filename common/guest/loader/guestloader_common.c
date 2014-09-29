#include <guestloader.h>
#include <arch_types.h>
#include <linuxloader.h>
#include <guestloader_common.h>

#define SET_MACHINE_TYPE_TO_R1() \
    asm volatile ("mov r1, %0" : : "r" (MACHINE_TYPE) : "memory", "cc")
#define SET_ATAGS_TO_R2() \
    asm volatile \
    ("mov r2, %0" : : "r" (linuxloader_get_atags_addr()) : "memory", "cc")
#define JUMP_TO_ADDRESS(addr) \
    asm volatile ("mov pc, %0" : : "r" (addr) : "memory", "cc")
#define ADD_PC_TO_OFFSET(offset) \
    asm volatile ("add  %0, pc, %0" : : "r" (offset) : "memory", "cc")

enum guest_image_type {
    LOADER,
    GUEST
};


/*
 * @brief Copies a guest to address.
 * @param img_type Guest Image type you want to copy. LOADER or GUEST.
 * @param dst_addr Destination address.
 */

void copy_image_to(uint32_t *src_addr, uint32_t *end_addr, uint32_t *dst_addr)
{
    uint32_t *src = src_addr;
    uint32_t *end = end_addr;
    uint32_t *dst = dst_addr;
    while (src < end)
        *dst++ = *src++;
}

void loader_boot_guest(uint32_t guest_os_type)
{
    uart_print("Booting guest os...\n");

    uint32_t offset;
    uint32_t pc;

    if (guest_os_type == GUEST_TYPE_LINUX) {

        linuxloader_setup_atags(START_ADDR_LINUX);
        /* r1 : machine type
         * r2 : atags address
         */
        SET_MACHINE_TYPE_TO_R1();
        SET_ATAGS_TO_R2();
    } else {
        /* Copies loader to next to guest */
        copy_image_to(&loader_start, &loader_end, &guest_end);
        /* Jump pc to (pc + offset). */
        offset = ((uint32_t)(&guest_end - &loader_start) * sizeof(uint32_t));
        ADD_PC_TO_OFFSET(offset);
        JUMP_TO_ADDRESS(offset);
        /* Copies guest to start address */
        copy_image_to(&guest_start, &guest_end, (uint32_t *)START_ADDR);
    }
    /* Jump to start address of guest */
    JUMP_TO_ADDRESS(START_ADDR);

    /* The code must not reach here */
    uart_print("[loadbmguest] ERROR: CODE MUST NOT REACH HERE\n\r");
    while (1)
        ;
}
