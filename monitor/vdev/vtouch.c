#include "vtouch.h"
#include <print.h>

virtual_devices_info vtouch_info;
static vdev_touch vtouch;

//virtual_devices_info vtouch_init(void)
void vtouch_init(void)
{
	int result;
	vtouch_info.name 	= "touch";
	vtouch_info.base	= VTOUCH_BASE; 
	vtouch_info.size 	= VDEV_SIZE;
	vtouch_info.handler = touch_handler;

	result = add_vdev_callback(vtouch_info);

	if(result == 1)
		printh("register!!\n");

	printh("error!!\n");
}

vdev_callback_t touch_handler(unsigned int wnr, unsigned int offsets, unsigned int srt, struct arch_regs *regs) {

	if (!wnr) {

		switch (offsets){
		case 0x0:
			regs->gpr[srt] = vtouch.axis_x;		
		break;
		case 0x4:
			regs->gpr[srt] = vtouch.axis_y;		
		break;
		case 0x8:
			regs->gpr[srt] = vtouch.axis_x + vtouch.axis_y;		
		break;
		case 0x16:
		break;
		}
	}
	else { //WRITE
		switch (offsets){
		case 0x0:
			vtouch.axis_x = regs->gpr[srt];
		break;
		case 0x4:
			vtouch.axis_y = regs->gpr[srt];
		break;
		case 0x8:
			//do nothing
			//vtouch.axis_z = regs->gpr[srt];
		break;
		case 0x16:
		break;
		}
	}
}
