#ifndef __VDEV_GICD_H__
#define __VDEV_GICD_H__

#include <hvmm_types.h>

hvmm_status_t vdev_gicd_init(uint32_t base_addr);
void vgicd_get_istatus(vmid_t vmid, uint8_t **enabled_int_list, uint32_t inum);
#endif
