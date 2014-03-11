#include <context.h>
#include <guest.h>
#include <gic.h>
#include <gic_regs.h>
#include <vdev.h>
#define DEBUG
#include <log/print.h>
#include <asm-arm_inline.h>
#include <virqmap.h>

static int32_t vdev_hvc_yield_write(struct arch_vdev_info *info,
                        struct arch_regs *regs)
{
    printh("[hyp] _hyp_hvc_service:yield\n\r");
    context_dump_regs(regs);
    context_switchto(sched_policy_determ_next());
    return 0;
}
static int32_t vdev_hvc_yield_check(struct arch_vdev_info *info,
                        struct arch_regs *regs)
{
    if ((info->iss & 0xFFFF) == 0xFFFD)
        return 0;

    return -1;
}

static hvmm_status_t vdev_hvc_yield_reset_values(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_hvc_yield_ops = {
    .init = vdev_hvc_yield_reset_values,
    .check = vdev_hvc_yield_check,
    .write = vdev_hvc_yield_write,
};

struct vdev_module _vdev_hvc_yield_module = {
    .name = "K-Hypervisor vDevice HVC Yield Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_hvc_yield_ops,
};

hvmm_status_t vdev_hvc_yield_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_MIDDLE, &_vdev_hvc_yield_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_hvc_yield_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_hvc_yield_module.name, result);
    }

    return result;
}
vdev_module_middle_init(vdev_hvc_yield_init);
