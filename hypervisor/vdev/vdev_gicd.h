#ifndef __VDEV_GICD_H__
#define __VDEV_GICD_H__

#include <hvmm_types.h>
/**
 * @brief Initialize virtual gic and register virtual gic as virtual device
 * <br> This function initialzed virtual gic
 * <br> and register gic timer as gic device using vdev_reg_device() function
 * @param base_addr base address of virtual gic deivce
 * @return if virtual gic is registerd, it returns success, otherwise failed
 */
hvmm_status_t vdev_gicd_init(uint32_t base_addr);
typedef void (*vgicd_changed_istatus_callback_t)(vmid_t vmid,
                uint32_t istatus, uint8_t word_offset);
/**
 * @brief Calls a handler of registered virtual gic, when it was changed by virtual guest
 * @param callback pointer of virtual gic handler
 * @return Void
 */
void vgicd_set_callback_changed_istatus(
        vgicd_changed_istatus_callback_t callback);
#endif
