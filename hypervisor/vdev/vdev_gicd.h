#ifndef __VDEV_GICD_H__
#define __VDEV_GICD_H__

#include <hvmm_types.h>
/**
 * @brief Initialize virtual gic and register virtual gic as virtual device.
 * @param base_addr Base address of virtual gic deivce.
 * @return It returns success, when virtual gic is registerd.
 */
hvmm_status_t vdev_gicd_init(uint32_t base_addr);
typedef void (*vgicd_changed_istatus_callback_t)(vmid_t vmid,
                uint32_t istatus, uint8_t word_offset);
/**
 * @brief Calls a handler of registered virtual gic when gic_mask of
 * virtual guest or was changed.
 * @param callback Pointer of virtual gic handler.
 * @return void.
 */
void vgicd_set_callback_changed_istatus(
        vgicd_changed_istatus_callback_t callback);
#endif
