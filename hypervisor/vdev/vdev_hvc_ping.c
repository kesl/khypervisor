#include <context.h>
#include <gic.h>
#include <gic_regs.h>
#include <vdev.h>
#define DEBUG
#include <log/print.h>
#include <asm-arm_inline.h>
#include <virqmap.h>


static int32_t vdev_hvc_ping_write(struct arch_vdev_info *info,
                        struct arch_regs *regs)
{
    printh("[hyp] _hyp_hvc_service:ping\n\r");
    context_dump_regs(regs);
    return 0;
}
static int32_t vdev_hvc_ping_check(struct arch_vdev_info *info,
                        struct arch_regs *regs)
{
    if ((info->iss & 0xFFFF) == 0xFFFE)
        return 0;

    return -1;
}

static hvmm_status_t vdev_hvc_ping_reset(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_hvc_ping_ops = {
    .init = vdev_hvc_ping_reset,
    .check = vdev_hvc_ping_check,
    .write = vdev_hvc_ping_write,
};

struct vdev_module _vdev_hvc_ping_module = {
    .name = "K-Hypervisor vDevice HVC Ping Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_hvc_ping_ops,
};

hvmm_status_t vdev_hvc_ping_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_MIDDLE, &_vdev_hvc_ping_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_hvc_ping_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_hvc_ping_module.name, result);
    }

    return result;
}
vdev_module_middle_init(vdev_hvc_ping_init);
