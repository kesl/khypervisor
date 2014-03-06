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
/**
  * fuction pointer for virtual device callback
 */
typedef hvmm_status_t (*vdev_callback_t)(uint32_t wnr, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size);
/**
 * virtual deivce infomation
 */
struct vdev_info {
    char *name;
    unsigned int base;
    unsigned int size;
    vdev_callback_t handler;
};
/**
 * @brief initializes virtual device array
 */
void vdev_init(void);
/**i was't able to find this definition. */
int register_vdev(void);
/**
 * @brief register a new virtual device
 * @param new virtual divice's infomation
 * @return value of hvmm status
 */
hvmm_status_t vdev_reg_device(struct vdev_info *new_vdev);
/**
 * @brief emulates virtual divices
 * @param address range: base ~ base + size
 * @param flag mode 0: read, 1: write
 * @param size of virtual divice's access
 * @param architecture register gpr number
 * @param current architure registers
 * @return value of hvmm status
 */
hvmm_status_t vdev_emulate(uint32_t fipa, uint32_t wnr,
                enum vdev_access_size access_size, uint32_t srt,
                struct arch_regs *regs);

#endif /* __VDEV_H_ */
