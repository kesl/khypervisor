#include <context.h>
#include <vdev.h>
#include <hvmm_trace.h>

#include <log/print.h>

#define MAX_VDEV    5

static struct vdev_info vdev_list[MAX_VDEV];

void vdev_init(void)
{
    int i = 0;
    for (i = 0; i < MAX_VDEV; i++) {
        vdev_list[i].name = 0;
        vdev_list[i].base = 0;
        vdev_list[i].size = 0;
        vdev_list[i].handler = 0x0;
    }
}
/**
 * @brief Registers a new virtual device.
 * <br> This function added a new virtual device including device name,\
 *  base address, size, and handler.
 * @param new_vdev information of new virtual device.
 * @return When new virtual device registered successful,\
 *  it returns success. otherwise it returns failed.
 */
hvmm_status_t vdev_reg_device(struct vdev_info *new_vdev)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    int i = 0;
    HVMM_TRACE_ENTER();
    for (i = 0; i < MAX_VDEV; i++) {
        if (vdev_list[i].handler == 0x0) {
            vdev_list[i].name = new_vdev->name;
            vdev_list[i].base = new_vdev->base;
            vdev_list[i].size = new_vdev->size;
            vdev_list[i].handler = new_vdev->handler;
            printh("vdev:Registered vdev '%s' at index %d\n",
                    vdev_list[i].name, i);
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }
    if (result != HVMM_STATUS_SUCCESS) {
        printh("vdev:Failed registering vdev '%s', max %d full\n",
                new_vdev->name, MAX_VDEV);
    }
    HVMM_TRACE_EXIT();
    return result;
}
/**
 * @brief Emulates virtual divice.
 * <br> When virtual guest reach virtual device and hypervisor has\
 * virtual device.
 * this function is called and handles virtual deivce to write or read.
 * @param fipa HPFAR[39:12] of the faulting intermediate physical address.
 * @param wnr synchronous abort that was caused by a write or read operation.
 * @param access_size size of virtual device's access.
 * @param srt syndrome register transfer.
 * The value of the Rt operand of the faulting instruction witch is \
 * the destination register for a load operation and the source register for\
 * a stroe operation.
 * @param regs ARM registers.
 * <br> The ARM register includes 13 general purpose register r0-r12,\
 * 1 Stack Pointer (SP), 1 Link Register (LR), 1 Program Counter (PC).
 * @return When virtual divice emulated, it returns success, otherwise failed.
 */
hvmm_status_t vdev_emulate(uint32_t fipa, uint32_t wnr,
                enum vdev_access_size access_size, uint32_t srt,
                struct arch_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_NOT_FOUND;
    int i = 0;
    uint32_t offset;
    uint8_t isize = 4;
    HVMM_TRACE_ENTER();
    if (regs->cpsr & 0x20) {
        /* Thumb */
        isize = 2;
    }
    for (i = 0; i < MAX_VDEV; i++) {
        if (vdev_list[i].base == 0)
            break;

        offset = fipa - vdev_list[i].base;
        if (fipa >= vdev_list[i].base && offset < vdev_list[i].size &&
                vdev_list[i].handler != 0) {
            /* fipa is in the rage: base ~ base + size */
            printh("vdev: found %s for fipa %x srt:%x "
                    "gpr[srt]:%x write:%d vmid:%d\n",
                    vdev_list[i].name, fipa, srt, regs->gpr[srt], wnr,
                    context_current_vmid());
            result = vdev_list[i].handler(wnr, offset,
                    &(regs->gpr[srt]), access_size);
            if (wnr == 0)
                printh("vdev: result:%x\n", regs->gpr[srt]);

            /* Update PC regardless handling result */
            regs->pc += isize;
            break;
        } else {
            printh("vdev: fipa %x base %x not matched\n",
                    fipa, vdev_list[i].base);
        }
    }
    HVMM_TRACE_EXIT();
    return result;
}
