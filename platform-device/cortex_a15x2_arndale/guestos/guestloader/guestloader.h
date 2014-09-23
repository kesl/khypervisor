#ifndef __GUESTLOADER_H__
#define __GUESTLOADER_H__
#include "memmap.cfg"
#define MACHINE_TYPE        4274
#define GUEST_START_ADDR    CFG_LOADER_PHYS_START
#define ZIMAGE_PHYS_ADDR    CFG_GUEST_START
#endif
