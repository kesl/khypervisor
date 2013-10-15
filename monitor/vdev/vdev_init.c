#include <context.h>
#include <print.h>
#include "vtouch.h"
#include "vlcd.h"

virtual_devices_info vdev_list[MAX_VDEV];

void init_vdev(void)
{
	int i = 0;
	for (i = 0; i < MAX_VDEV; i++) {

		if ( i == 0 ) {
			vdev_list[i].name = "uart";
			vdev_list[i].base = VUART_BASE;
			vdev_list[i].size = VDEV_SIZE;
			vdev_list[i].handler = 0x1;
		}
		else if ( i == 1 ) {
			vdev_list[i].name = 0;
			vdev_list[i].base = 0;
			vdev_list[i].size = 0;
			vdev_list[i].handler	= 0x0 ;
		}
		else if ( i == 2 ) {
			vdev_list[i].name = 0;
			vdev_list[i].base = 0;
			vdev_list[i].size = 0;
			vdev_list[i].handler	= 0x0 ;
		}
	}

	vtouch_init();
	vlcd_init();
}

int add_vdev_callback(virtual_devices_info new_vdev)
{
	int i = 0;
	for (i = 0; i < MAX_VDEV; i++) {

		//if (vdev_list[i].name == new_vdev.name) {
			if (vdev_list[i].handler == 0x0 ) {
				printh("Add callback %s\n", vdev_list[i].name);
				vdev_list[i].name = new_vdev.name;
				vdev_list[i].base = new_vdev.base;
				vdev_list[i].size = new_vdev.size;
				vdev_list[i].handler = new_vdev.handler;
				return 1;
			}
		//}
	}
	return -1;
}

int find_vdev_handler(unsigned int fipa, unsigned int wnr, unsigned int srt, struct arch_regs *regs) {

	int i = 0;
	unsigned int offset;
	for (i = 0; i < MAX_VDEV; i++){
		offset = fipa - vdev_list[i].base;
		if ((0 <= offset) && (offset < VDEV_SIZE)) {
			vdev_list[i].handler(wnr, offset, srt, regs);
			return i;
		}
	}
	return -1;
}
