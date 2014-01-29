#include "vdev_timer.h"
#include <context.h>

#include <log/print.h>

struct vdev_timer_regs{
    uint32_t timer_mask;
};

static vdev_info_t _vdev_info;
static struct vdev_timer_regs regs[NUM_GUESTS_STATIC]; 

static vtimer_changed_status_callback_t _write_status = 0;

void vtimer_set_callback_chagned_status(vtimer_changed_status_callback_t callback)
{
    _write_status = callback;
}

static void vtimer_changed_status( vmid_t vmid, uint32_t status )
{
    if( _write_status != 0 ) {
        _write_status( vmid, status );
    }
}

static hvmm_status_t access_handler(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    printh( "%s: %s offset:%d value:%x\n", __FUNCTION__, write ? "write" : "read", offset, write ? *pvalue : (uint32_t) pvalue );
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    unsigned int vmid = context_current_vmid();
    if (!write) {
        // READ
        switch (offset){
        case 0x0:
            *pvalue = regs[vmid].timer_mask;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    } else { 
        //WRITE
        switch (offset){
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
    if ( result == HVMM_STATUS_SUCCESS ) {
        printh("%s: vdev registered:'%s'\n", __FUNCTION__, _vdev_info.name);
    } else {
        printh("%s: Unable to register vdev:'%s' code=%x\n", __FUNCTION__, _vdev_info.name, result);
    }
    return result;
}
