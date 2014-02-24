#ifndef __VDEV_TIMER_H__
#define __VDEV_TIMER_H__

#include <vdev.h>

hvmm_status_t vdev_timer_init(uint32_t base_addr);
typedef void (*vtimer_changed_status_callback_t)(vmid_t vmid, uint32_t status);

void vtimer_set_callback_chagned_status(vtimer_changed_status_callback_t callback);


#endif
