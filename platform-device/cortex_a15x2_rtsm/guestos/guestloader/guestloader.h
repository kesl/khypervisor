#ifndef __GUESTLOADER_H__
#define __GUESTLOADER_H__

#include "memmap.cfg"
#define MACHINE_TYPE        2272
#define GUEST_START_ADDR    CFG_LOADER_PHYS_START
#define ZIMAGE_PHYS_ADDR    CFG_GUEST_START

/** @brief Flag for autoboot
 *  @param flag If flag is '0', it is auto boot, otherwise CLI boot
 */
void guestloader_flag_autoboot(int flag);

#endif
