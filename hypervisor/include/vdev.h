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
/**
 * @brief Initializes virtual device mapping table
 * @return Void
 */
void vdev_init(void);
/**
 * this function definition is not exist. !!
 */
int register_vdev(void);
/**
 * @brief Register new virtual device
 * <br> this function added new virtual device feature which has base address, size, and handler
 * @param new_vdev new virtual device's infomation
 * @return if new virtual device is registed successfully, it returns success, otherwhise returns fail
 */
hvmm_status_t vdev_reg_device(struct vdev_info *new_vdev);

/**
 * @brief Emulates virtual divice
 * <br> When virtual guest reach virtual device,
 * <br> this function is called and handles virtual deivce to write or read
 * @param fipa ISS Bits [39:12] of the faulting intermediate physical address
 * @param wnr synchronous abort that was caused by a write or read operation
 * @param access_size size of virtual device's access
 * @param str general purpose register number to read or write
 * @param regs ARM register
 * <br> The ARM register includes 13 general purpose register r0-r12, 1 Stack Pointer (SP),
 * <br> 1 Link Register (LR), 1 Program Counter (PC)
 * @return if virtual divce is emulated, it returns success, otherwize failed.
 */
hvmm_status_t vdev_emulate(uint32_t fipa, uint32_t wnr,
                enum vdev_access_size access_size, uint32_t srt,
                struct arch_regs *regs);

#endif /* __VDEV_H_ */
