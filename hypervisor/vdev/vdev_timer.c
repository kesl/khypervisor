#include "vdev_timer.h"
#include <context.h>

#include <log/print.h>

struct vdev_timer_regs {
    uint32_t timer_mask;
};

static struct vdev_info _vdev_info;
static struct vdev_timer_regs regs[NUM_GUESTS_STATIC];

static vtimer_changed_status_callback_t _write_status;

void vtimer_set_callback_chagned_status(
        vtimer_changed_status_callback_t callback)
{
    _write_status = callback;
}
/**
 * @brief Calls whenever virtual guest reach virtual timer device
 * @param vmid virtual guest id
 * @param status value that virtual guest choose to receive or not receive the interrupt
 * @return Void
 */
static void vtimer_changed_status(vmid_t vmid, uint32_t status)
{
    if (_write_status != 0)
        _write_status(vmid, status);
}
/**
 * @brief Handles virtual timer device
 * @param write the choice that the argument Pvalue's status is weather written or read
 * @param offset length from virtual timer device base address
 * @param pvalue address of data to write or read
 * @param access_size size of virtual timer device access
 * @return if it excute virtual timer handler to read or write on device
 * <br> it turns success, otherwise bad access
 */
static hvmm_status_t access_handler(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size)
{
    printh("%s: %s offset:%d value:%x\n", __func__,
            write ? "write" : "read",
            offset, write ? *pvalue : (uint32_t) pvalue);
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    unsigned int vmid = context_current_vmid();
    if (!write) {
        /* READ */
        switch (offset) {
        case 0x0:
            *pvalue = regs[vmid].timer_mask;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    } else {
        /* WRITE */
        switch (offset) {
        case 0x0:
            regs[vmid].timer_mask = *pvalue;
            vtimer_changed_status(vmid, *pvalue);
            result = HVMM_STATUS_SUCCESS;
                break;
        }
    }
    return result;
}

hvmm_status_t vdev_timer_init(uint32_t base_addr)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    _vdev_info.name     = "vtimer";
    _vdev_info.base     = base_addr;
    _vdev_info.size     = sizeof(struct vdev_timer_regs);
    _vdev_info.handler  = access_handler;
    result = vdev_reg_device(&_vdev_info);
    if (result == HVMM_STATUS_SUCCESS)
        printh("%s: vdev registered:'%s'\n", __func__, _vdev_info.name);
    else
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_info.name, result);

    return result;
}
