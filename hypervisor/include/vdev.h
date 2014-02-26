#ifndef __VDEV_H_
#define __VDEV_H_

#include <context.h>
#include <hvmm_types.h>

enum vdev_access_size {
    VDEV_ACCESS_BYTE = 0,
    VDEV_ACCESS_HWORD = 1,
    VDEV_ACCESS_WORD = 2,
    VDEV_ACCESS_RESERVED = 3
};

/*
 * @write   0: read, 1: write
 * @offset  offset in bytes from base address
 * @pvalue  address to input value (if write) or output value (if read)
 * return: HVMM_STATUS_SUCCESS if successful, failed otherwise
 */
typedef hvmm_status_t (*vdev_callback_t)(uint32_t wnr, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size);

struct vdev_info {
    char *name;
    unsigned int base;
    unsigned int size;
    vdev_callback_t handler;
};

void vdev_init(void);
int register_vdev(void);
hvmm_status_t vdev_reg_device(struct vdev_info *new_vdev);
hvmm_status_t vdev_emulate(uint32_t fipa, uint32_t wnr,
                enum vdev_access_size access_size, uint32_t srt,
                struct arch_regs *regs);

#endif /* __VDEV_H_ */
