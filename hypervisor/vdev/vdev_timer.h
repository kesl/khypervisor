#ifndef __VDEV_TIMER_H__
#define __VDEV_TIMER_H__

#include <vdev.h>
/**
 * @brief Initialize virtual timer and register virtual timer as virtual device.
 * This function initialzed virtual timer and register virtual timer.
 * @param base_addr base address of virtual timer.
 * @return It returns success when virtual timer is registerd.
 */
hvmm_status_t vdev_timer_init(uint32_t base_addr);
typedef void (*vtimer_changed_status_callback_t)(vmid_t vmid, uint32_t status);
/**
 * @brief Calls a handler of registered virtual timer when it was changed by
 * virtual guest or others.
 * @param callback Pointer of virtual timer handler.
 * @return void.
 */
void vtimer_set_callback_chagned_status(
        vtimer_changed_status_callback_t callback);


#endif
