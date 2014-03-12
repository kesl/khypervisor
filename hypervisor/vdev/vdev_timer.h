#ifndef __VDEV_TIMER_H__
#define __VDEV_TIMER_H__

#include <vdev.h>
/**
 * @brief Initialize virtual timer and register virtual timer as virtual device
 * <br> This function initialzed virtual timer
 * <br> and register virtual timer as virtual device using vdev_reg_device() function
 * @param base_addr base address of virtual timer
 * @return if virtual timer is registerd, it returns success, otherwise failed
 */
hvmm_status_t vdev_timer_init(uint32_t base_addr);
typedef void (*vtimer_changed_status_callback_t)(vmid_t vmid, uint32_t status);
/**
 * @brief Calls a handler of registered virtual timer, when it was changed by virtual guest
 * @param callback pointer of virtual timer handler
 * @return Void
 */
void vtimer_set_callback_chagned_status(
        vtimer_changed_status_callback_t callback);


#endif
