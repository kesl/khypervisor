#ifndef __VDEV_SAMPLE_H__
#define __VDEV_SAMPLE_H__

#include <vdev.h>
/**
 * @brief initialize virtual smaple device and register sample devce for verctor table
 * @param base address of sample device
 * @return value of hvmm status
 */
hvmm_status_t vdev_sample_init(uint32_t base_addr);

#endif
