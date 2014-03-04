#ifndef __VDEV_TIMER_H__
#define __VDEV_TIMER_H__

#include <vdev.h>
/**
 * @brief initialize virtual timer device and register virtual devce for verctor table
 * @param base address of timer
 * @return value of hvmm status
 */
hvmm_status_t vdev_timer_init(uint32_t base_addr);
typedef void (*vtimer_changed_status_callback_t)(vmid_t vmid, uint32_t status);
/**
 * @brief setting callback fuction
 * @param vtimer_changed_status_callback_t callback fuction porinter for callback
 */
void vtimer_set_callback_chagned_status(
        vtimer_changed_status_callback_t callback);


#endif
