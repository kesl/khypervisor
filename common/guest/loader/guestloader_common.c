#include <guestloader.h>
#include <arch_types.h>
#include <linuxloader.h>
#include <guestloader_common.h>

/*
 * ARMv8 Linux Primary Booting core registers
 *
 * X0 - Physical address of device tree blob (dtb)
 * X1 - 0
 * X2 - 0
 * X3 - 0
 *
 */
#define SET_AND_JUMP_TO_LINUX() \
    asm volatile ( \
    "mov x5, %0;" \
    "mov x4, %1;" \
    "mov x0, x5;" \
    "mov x1, xzr;" \
    "mov x2, xzr;" \
    "mov x3, xzr;" \
    "br x4" \
    : : "r" (START_FDT_ADDR), "r" (START_ADDR) : "memory", "cc")

#define ADD_PC_TO_OFFSET(addr) \
    ({ \
    uint64_t rval; \
    asm volatile ( \
    "adr x10, goal;" \
    "add %0, x10, %1;" \
    :"=r" (rval) : "r" (addr) : "memory", "cc"); \
    rval; })

#define JUMP_TO_ADDRESS(offset) \
    asm volatile ("mov x0, %0;" \
    "br x0;" \
    : : "r" (offset) : "memory", "cc")

/* Set the jump goal */
#define GOAL() \
    asm volatile ("goal:" :::)
enum guest_image_type {
    LOADER,
    GUEST
};


/*
 * @brief Copies a guest to address.
 * @param img_type Guest Image type you want to copy. LOADER or GUEST.
 * @param dst_addr Destination address.
 */

void copy_image_to(uint64_t *src_addr, uint64_t *end_addr, uint64_t *dst_addr)
{
    uint64_t *src = src_addr;
    uint64_t *end = end_addr;
    uint64_t *dst = dst_addr;
    while (src < end)
        *dst++ = *src++;
}

void loader_boot_guest(uint32_t guest_os_type)
{
    uart_print("Booting guest os...\n");

    uint64_t offset;
    uint64_t pc;

    asm volatile ("msr daifset, 0x2" : : : "memory");

    if (guest_os_type == GUEST_TYPE_LINUX) {

        /*
         * x0 : Physical address of device tree blob
         */
        SET_AND_JUMP_TO_LINUX();
    } else {
        uart_print("loader_start:");
        uart_print_hex64(&loader_start);
        uart_print("\n");
        uart_print("loader_end:");
        uart_print_hex64(&loader_end);
        uart_print("\n");
        uart_print("guest_start:");
        uart_print_hex64(&guest_start);
        uart_print("\n");
        uart_print("guest_end:");
        uart_print_hex64(&guest_end);
        uart_print("\n");
        /* Copies loader to next to guest */
        copy_image_to(&loader_start, &loader_end, &guest_end);
        /* Jump pc to (pc + offset). */
        offset = ((uint64_t)(&guest_end - &loader_start) * sizeof(uint64_t));
        offset = ADD_PC_TO_OFFSET(offset);
        JUMP_TO_ADDRESS(offset);
        GOAL();
        /* Copies guest to start address */
        copy_image_to(&guest_start, &guest_end, (uint64_t *)START_ADDR);
        /* Jump to start address of guest */
        JUMP_TO_ADDRESS(START_ADDR);
    }

    /* The code must not reach here */
    uart_print("[loadbmguest] ERROR: CODE MUST NOT REACH HERE\n\r");
    while (1)
        ;
}
