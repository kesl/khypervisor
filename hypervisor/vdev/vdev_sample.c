#include "vdev_sample.h"
#include <context.h>

#include <log/print.h>

struct vdev_sample_regs {
    uint32_t axis_x;
    uint32_t axis_y;
    uint32_t axis_z;
};

static vdev_info_t _vdev_info;
static struct vdev_sample_regs regs[NUM_GUESTS_STATIC];

hvmm_status_t access_handler(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    printh("%s: %s offset:%d value:%x\n", __FUNCTION__, write ? "write" : "read", offset, write ? *pvalue : (uint32_t) pvalue);
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    unsigned int vmid = context_current_vmid();
    if (!write) {
        /* READ */
        switch (offset) {
        case 0x0:
            *pvalue = regs[vmid].axis_x;
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0x4:
            *pvalue = regs[vmid].axis_y;
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0x8:
            *pvalue = regs[vmid].axis_x + regs[vmid].axis_y;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    } else {
        /* WRITE */
        switch (offset) {
        case 0x0:
            regs[vmid].axis_x = *pvalue;
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0x4:
            regs[vmid].axis_y = *pvalue;
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0x8:
            /* read-only register, ignored, but no error */
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }
    return result;
}

hvmm_status_t vdev_sample_init(uint32_t base_addr)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    _vdev_info.name     = "sample";
    _vdev_info.base     = base_addr;
    _vdev_info.size     = sizeof(struct vdev_sample_regs);
    _vdev_info.handler  = access_handler;
    result = vdev_reg_device(&_vdev_info);
    if (result == HVMM_STATUS_SUCCESS) {
        printh("%s: vdev registered:'%s'\n", __FUNCTION__, _vdev_info.name);
    } else {
        printh("%s: Unable to register vdev:'%s' code=%x\n", __FUNCTION__, _vdev_info.name, result);
    }
    return result;
}
