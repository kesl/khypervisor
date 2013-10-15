#ifndef __VTOUCH_H_
#define __VTOUCH_H_

#include <virtual_devices.h>

#define VTOUCH_BASE	0x3FFFF000

typedef struct {
	unsigned int axis_x;
	unsigned int axis_y;
	unsigned int axis_z;
} vdev_touch;

vdev_callback_t touch_handler(unsigned int wnr, unsigned int offsets, unsigned int srt, struct arch_regs *regs);

void vtouch_init(void);
#endif
