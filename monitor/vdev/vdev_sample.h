#ifndef __VDEV_SAMPLE_H__
#define __VDEV_SAMPLE_H__

#include <vdev.h>

#define VTOUCH_BASE	0x3FFFF000

hvmm_status_t vdev_sample_init(uint32_t base_addr);
#endif
