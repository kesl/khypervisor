#ifndef __VDEV_H_
#define __VDEV_H_

#include <hvmm_types.h>
#include <guest.h>

enum vdev_access_size {
    VDEV_ACCESS_BYTE = 0,
    VDEV_ACCESS_HWORD,
    VDEV_ACCESS_WORD,
    VDEV_ACCESS_RESERVED
};

enum vdev_level {
    VDEV_LEVEL_LOW = 0,
    VDEV_LEVEL_MIDDLE,
    VDEV_LEVEL_HIGH,
    VDEV_LEVEL_MAX
};

#define VDEV_ERROR -1
#define VDEV_NOT_FOUND -1

struct arch_vdev_trigger_info {
    /** Exception Class */
    uint32_t ec;
    /** Instruction Specific Syndrome */
    uint32_t iss;
    /** Fault Address */
    uint32_t fipa;
    /** Instruction Length */
    uint32_t il;
    enum vdev_access_size sas;
#ifdef ARM64
    uint64_t *value;
#else
    uint32_t *value;
#endif
};

typedef hvmm_status_t (*vdev_callback_t)(uint32_t wnr, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size);

typedef void(*vdev_irq_callback_t)(void *pdata);

struct vdev_memory_map {
    unsigned int base;
    unsigned int size;
};

typedef int (*initcall_t)(void);

extern initcall_t __vdev_module_high_start[];
extern initcall_t __vdev_module_high_end[];
extern initcall_t __vdev_module_middle_end[];
extern initcall_t __vdev_module_low_end[];

#define __define_vdev_module(level, fn, id) \
    static initcall_t __initcall_##fn##id __attribute__((__used__)) \
    __attribute__((__section__(".vdev_module" level ".init"))) = fn

#define vdev_module_high_init(fn)       __define_vdev_module("0", fn, 1)
#define vdev_module_middle_init(fn)     __define_vdev_module("1", fn, 2)
#define vdev_module_low_init(fn)        __define_vdev_module("2", fn, 3)

struct vdev_ops {
    /** Initalize virtual registers */
    hvmm_status_t (*init)(void);

    /**
     *  This function is used by the virtual device framework, when the
     *  trap is occurred, finding the virtual device using the this function.
     */
    int32_t (*check)(struct arch_vdev_trigger_info *, struct arch_regs *);

    /** Read a virtual register */
    int32_t (*read)(struct arch_vdev_trigger_info *, struct arch_regs *);

    /** Write a virtual register */
    int32_t (*write)(struct arch_vdev_trigger_info *, struct arch_regs *);

    /** Currently unused function */
    hvmm_status_t (*enable)(void);

    /** Currently unused function */
    hvmm_status_t (*disable)(void);

    /** Currently unused function */
    hvmm_status_t (*request_irq)(vdev_irq_callback_t, void *user);

    /** Currently unused function */
    hvmm_status_t (*notify_irq)(uint32_t);

    /** Post for remain on the job */
    hvmm_status_t (*post)(struct arch_vdev_trigger_info *, struct arch_regs *);

    /** Save virtual device state */
    hvmm_status_t (*save)(vmid_t vmid);

    /** Restore virtual device state */
    hvmm_status_t (*restore)(vmid_t vmid);

    /** Dump state of the vdev */
    hvmm_status_t (*dump)(void);

};

struct vdev_module {
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

    /** Virtual Device Operation */
    struct vdev_ops *ops;

};

hvmm_status_t vdev_register(int level, struct vdev_module *module);
int32_t vdev_find(int level, struct arch_vdev_trigger_info *info,
        struct arch_regs *regs);
int32_t vdev_read(int level, int num, struct arch_vdev_trigger_info *info,
            struct arch_regs *regs);
int32_t vdev_write(int level, int num, struct arch_vdev_trigger_info *info,
            struct arch_regs *regs);
hvmm_status_t vdev_post(int level, int num, struct arch_vdev_trigger_info *info,
            struct arch_regs *regs);
hvmm_status_t vdev_save(vmid_t vmid);
hvmm_status_t vdev_restore(vmid_t vmid);
hvmm_status_t vdev_init(void);

#endif /* __VDEV_H_ */
