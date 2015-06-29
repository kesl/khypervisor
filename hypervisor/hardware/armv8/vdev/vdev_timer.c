#include <vdev.h>
#define DEBUG
#include <log/print.h>
#include <timer.h>
#include <interrupt.h>
#include <smp.h>

#define VTIMER_BASE_ADDR 0x3FFFE000
#define VTIMER_IRQ 30

struct vdev_vtimer_regs {
    uint64_t vtimer_mask;
};

static struct vdev_memory_map _vdev_timer_info = {
    .base = VTIMER_BASE_ADDR,
    .size = sizeof(struct vdev_vtimer_regs),
};

static struct vdev_vtimer_regs vtimer_regs[NUM_GUESTS_STATIC];
static int _timer_status[NUM_GUESTS_STATIC] = {0, };

static void vtimer_changed_status(vmid_t vmid, uint32_t status)
{
    _timer_status[vmid] = status;
}

static hvmm_status_t vdev_vtimer_access_handler(uint32_t write,
        uint32_t offset, uint64_t *pvalue, enum vdev_access_size access_size)
{
    printH("%s: %s offset:%d value:%x\n", __func__,
            write ? "write" : "read",
            offset, write ? *pvalue : (uint64_t) pvalue);
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    unsigned int vmid = guest_current_vmid();
    if (!write) {
        /* READ */
        switch (offset) {
        case 0x0:
            *pvalue = vtimer_regs[vmid].vtimer_mask;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    } else {
        /* WRITE */
        switch (offset) {
        case 0x0:
            vtimer_regs[vmid].vtimer_mask = *pvalue;
            vtimer_changed_status(vmid, *pvalue);
            result = HVMM_STATUS_SUCCESS;
                break;
        }
    }
    return result;
}

static int32_t vdev_vtimer_read(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_timer_info.base;

    return vdev_vtimer_access_handler(0, offset, info->value, info->sas);
}

static int32_t vdev_vtimer_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_timer_info.base;

    return vdev_vtimer_access_handler(1, offset, info->value, info->sas);
}

static int32_t vdev_vtimer_post(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint8_t isize = 4;

    if (regs->cpsr & 0x20) /* Thumb */
        isize = 2;

    regs->pc += isize;

    return 0;
}

static int32_t vdev_vtimer_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_timer_info.base;

    if (info->fipa >= _vdev_timer_info.base &&
        offset < _vdev_timer_info.size)
        return 0;
    return VDEV_NOT_FOUND;
}

void callback_timer(void *pdata)
{
    vmid_t vmid = guest_current_vmid();

    if (_timer_status[vmid] == 0)
        interrupt_guest_inject(vmid, VTIMER_IRQ, 0, INJECT_SW);
}

static hvmm_status_t vdev_vtimer_reset(void)
{
    int i;
    struct timer_val timer;
    uint32_t cpu = smp_processor_id();

    if (!cpu) {
        for (i = 0; i < NUM_GUESTS_STATIC; i++)
            _timer_status[i] = 1;
    }

    timer.interval_us = GUEST_SCHED_TICK;
    timer.callback = &callback_timer;

    timer_set(&timer, GUEST_TIMER);

    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_hvc_vtimer_ops = {
    .init = vdev_vtimer_reset,
    .check = vdev_vtimer_check,
    .read = vdev_vtimer_read,
    .write = vdev_vtimer_write,
    .post = vdev_vtimer_post,
};

struct vdev_module _vdev_hvc_vtimer_module = {
    .name = "K-Hypervisor vDevice vTimer Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_hvc_vtimer_ops,
};

hvmm_status_t vdev_vtimer_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    write_cntvoff(VTIMER_BASE_ADDR);

    result = vdev_register(VDEV_LEVEL_LOW, &_vdev_hvc_vtimer_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_hvc_vtimer_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_hvc_vtimer_module.name, result);
    }

    return result;
}
vdev_module_low_init(vdev_vtimer_init);
