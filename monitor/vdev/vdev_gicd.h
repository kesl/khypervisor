#ifndef __VDEV_GICD_H__
#define __VDEV_GICD_H__

#include <hvmm_types.h>

hvmm_status_t vdev_gicd_init(uint32_t base_addr);
typedef void (*vgicd_changed_istatus_callback_t)(vmid_t vmid, uint32_t istatus, uint8_t word_offset);

void vgicd_set_callback_changed_istatus(vgicd_changed_istatus_callback_t callback);
#endif
