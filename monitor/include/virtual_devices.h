#ifndef __VDEV_H_
#define __VDEV_H_
#include <context.h>

#define VDEV_SIZE	0x1000
#define MAX_VDEV	5

#define VUART_BASE	0x3FFFD000

typedef void(* vdev_callback_t) (unsigned int wnr, unsigned int offsets, unsigned int srt, struct arch_regs *regs);

typedef struct {
	unsigned char *name;
	unsigned int base;
	unsigned int size;
	vdev_callback_t handler;
} virtual_devices_info;

void init_vdev(void);
int register_vdev(void);
int add_vdev_callback(virtual_devices_info new_vdev);
int find_vdev_handler(unsigned int fipa, unsigned int wnr, unsigned int srt, struct arch_regs *regs);

#endif //__VDEV_H_
