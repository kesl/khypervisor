/*
 * For Dynamic instrumentation module
 * Using virtual device.
 */
#include <vdev.h>
#define DEBUG
#include <log/print.h>
#include <monitoring.h>
#include <asm_io.h>
#include <guest.h>
#define MONITOR_BASE_ADDR 0x3FFFD000

struct vdev_monitor_regs {
    uint32_t set_point;
    uint32_t clean_point;
};

static struct vdev_memory_map _vdev_monitor_info = {
   .base = MONITOR_BASE_ADDR,
   .size = 0x1000
};

/* static struct vdev_monitor_regs monitor_regs[NUM_GUESTS_STATIC]; */

static hvmm_status_t vdev_monitor_access_handler(uint32_t write,
        uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    printh("%s: %s offset:%d value:%x\n", __func__,
            write ? "write" : "read", offset,
            write ? *pvalue : (uint32_t) pvalue);
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    if (!write) {
        /* READ */
        /* TODO Read debugging resources */
        switch (offset) {
        case 0x8:
            /* print monitoring list */
            result = kmo_list();
            break;
        case 0x10:
            /* go */
            result = kmo_run();
            break;
        case 0x18:
            /* all clean */
            result = kmo_all_clean();
            break;
        case 0x1C:
            /* memory dump */
            result = kmo_memory_dump();
            break;
        }
    } else {
        /* WRITE */
        switch (offset) {
        case 0x0:
            /* Set monitoring point */
            result = kmo_break(*pvalue, TRAP);
            break;
        case 0x4:
            /* Clean monitoring point */
            result = kmo_clean(*pvalue, TRAP);
            break;
        case 0x8:
            /* read-only register, ignored, but no error */
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0xC:
            /* break */
            result = kmo_break(*pvalue, BREAK_TRAP);
            break;
        case 0x14:
            /* Clean breaking point */
            result = kmo_clean(*pvalue, BREAK_TRAP);
            break;
        }
    }
    return result;
}

static hvmm_status_t vdev_monitor_read(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_monitor_info.base;

    return vdev_monitor_access_handler(0, offset, info->value, info->sas);
}

static hvmm_status_t vdev_monitor_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_monitor_info.base;

    return vdev_monitor_access_handler(1, offset, info->value, info->sas);
}

static int32_t vdev_monitor_post(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint8_t isize = 4;

    if (regs->cpsr & 0x20) /* Thumb */
        isize = 2;

    regs->pc += isize;

    return 0;
}

static int32_t vdev_monitor_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_monitor_info.base;

    if (info->fipa >= _vdev_monitor_info.base &&
        offset < _vdev_monitor_info.size)
        return 0;
    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_monitor_reset(void)
{
    printh("vdev init:'%s'\n", __func__);
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_monitor_ops = {
    .init = vdev_monitor_reset,
    .check = vdev_monitor_check,
    .read = vdev_monitor_read,
    .write = vdev_monitor_write,
    .post = vdev_monitor_post,
};

struct vdev_module _vdev_monitor_module = {
    .name = "K-Hypervisor vDevice monitoring Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_monitor_ops,
};

hvmm_status_t vdev_monitor_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_LOW, &_vdev_monitor_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_monitor_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_monitor_module.name, result);
    }

    return result;
}
vdev_module_low_init(vdev_monitor_init);
