#ifndef __MM_H__
#define __MM_H__

#include <hvmm_types.h>
#include "arch_types.h"

/**
 * @brief Enum values of the stage2 memory attribute.
 *
 * \ref Memory_attribute "Memory attribute configuration"
 */
enum memattr {
    MEMATTR_SO = 0x0,               /* Strongly Ordered */
    MEMATTR_DM = 0x1,               /* Device memory */
    MEMATTR_NORMAL_ONC = 0x4,       /* Outer Non-cacheable */
    MEMATTR_NORMAL_OWT = 0x8,
    MEMATTR_NORMAL_OWB = 0xC,
    MEMATTR_NORMAL_INC = 0x1,
    MEMATTR_NORMAL_IWT = 0x2,
    MEMATTR_NORMAL_IWB = 0x3,
};

/**
 * @brief Memory map descriptor.
 *
 * Memory map information descriptor.
 * - label Name of the descriptor.
 * - va Intermediate physical address(IPA).
 * - pa Physical address.
 * - size Size of this memory area.
 * - attr Memory attribute value.
 */
struct memmap_desc {
    char *label;
    uint64_t va;
    uint64_t pa;
    uint32_t size;
    enum memattr attr;
};

struct memory_ops {
    /** Initalize Memory state */
    hvmm_status_t (*init)(struct memmap_desc **, struct memmap_desc **);

    /** Allocate heap memory */
    void * (*alloc)(unsigned long size);

    /** Free heap memory */
    void (*free)(void *ap);

    /** Save guest memory structure */
    hvmm_status_t (*save)(void);

    /** Restore guest memory structure */
    hvmm_status_t (*restore)(vcpuid_t);

    /** Dump state of the memory */
    hvmm_status_t (*dump)(void);
};

struct memory_module {
    /** tag must be initialized to HAL_TAG */
    uint32_t tag;

    /**
     * Version of the module-specific device API. This value is used by
     * the derived-module user to manage different device implementations.
     * The user who uses this module is responsible for checking
     * the module_api_version and device version fields to ensure that
     * the user is capable of communicating with the specific module
     * implementation.
     *
     */
    uint32_t version;

    /** Identifier of module */
    const char *id;

    /** Name of this module */
    const char *name;

    /** Author/owner/implementor of the module */
    const char *author;

    /** Memory Operation */
    struct memory_ops *ops;
};

extern struct memory_module _memory_module;
extern uint32_t _guest_bin_start;
extern uint32_t _guest2_bin_start;
extern uint32_t _guest_secondary_bin_start;

void memory_free(void *ap);
void *memory_alloc(unsigned long size);
hvmm_status_t memory_save(void);
hvmm_status_t memory_restore(vcpuid_t vmid);
hvmm_status_t memory_init(struct memmap_desc **guest0,
                    struct memmap_desc **guest1);

#endif
