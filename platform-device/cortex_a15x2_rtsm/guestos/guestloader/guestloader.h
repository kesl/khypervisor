#ifndef __GUESTLOADER_H__
#define __GUESTLOADER_H__
#include "memmap.cfg"
#define MACHINE_TYPE        2272
#define GUEST_START_ADDR    LDS_LOADER_PHYS_START
#define ZIMAGE_START_OFFSET 0x8040
/** @brief Flag for autoboot
 *  @param flag If flag is '0', it is auto boot, otherwise CLI boot
 */
void guestloader_flag_autoboot(int flag);

#endif
