#include <guest.h>
#include <gic.h>
#include <gic_regs.h>
#include <vdev.h>
#define DEBUG
#include <log/print.h>
#include <asm-arm_inline.h>
#include <virqmap.h>

static int32_t vdev_hvc_stay_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    printh("[hyp] _hyp_hvc_service:stay\n\r");
    return 0;
}

static int32_t vdev_hvc_stay_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    if ((info->iss & 0xFFFF) == 0xFFFF)
        return 0;

    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_hvc_stay_reset_values(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_hvc_stay_ops = {
    .init = vdev_hvc_stay_reset_values,
    .check = vdev_hvc_stay_check,
    .write = vdev_hvc_stay_write,
};

struct vdev_module _vdev_hvc_stay_module = {
    .name = "K-Hypervisor vDevice HVC Stay Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_hvc_stay_ops,
};

hvmm_status_t vdev_hvc_stay_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;


    result = vdev_register(VDEV_LEVEL_MIDDLE, &_vdev_hvc_stay_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_hvc_stay_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_hvc_stay_module.name, result);
    }

    return result;
}
vdev_module_middle_init(vdev_hvc_stay_init);
