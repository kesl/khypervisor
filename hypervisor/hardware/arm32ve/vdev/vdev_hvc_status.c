#define DEBUG
#include <guest.h>
#include <gic.h>
#include <gic_regs.h>
#include <vdev.h>
#include <log/print.h>
#include <asm-arm_inline.h>

static int32_t vdev_hvc_status_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t spsr, lr, sp;

    printh("[hyp] : Dump K-Hypervisor's registers\n\r");
    printh(" - banked regs\n");
    asm volatile(" mrs     %0, sp_usr\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_usr\n\t" : "=r"(lr) : : "memory", "cc");
    printh(" - usr: sp:%x lr:%x\n", sp, lr);
    asm volatile(" mrs     %0, spsr_svc\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    printh(" - svc: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
    asm volatile(" mrs     %0, spsr_irq\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_irq\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_irq\n\t" : "=r"(lr) : : "memory", "cc");
    printh(" - irq: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
    printh(" - Current guest's vmid is %d\n", guest_current_vmid());
    return 0;
}

static int32_t vdev_hvc_status_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    if ((info->iss & 0xFFFF) == 0xFFFC)
        return 0;

    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_hvc_status_reset(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_hvc_status_ops = {
    .init = vdev_hvc_status_reset,
    .check = vdev_hvc_status_check,
    .write = vdev_hvc_status_write,
};

struct vdev_module _vdev_hvc_status_module = {
    .name = "K-Hypervisor vDevice HVC Status Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_hvc_status_ops,
};

hvmm_status_t vdev_hvc_status_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_MIDDLE, &_vdev_hvc_status_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_hvc_status_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_hvc_status_module.name, result);
    }

    return result;
}
vdev_module_middle_init(vdev_hvc_status_init);
