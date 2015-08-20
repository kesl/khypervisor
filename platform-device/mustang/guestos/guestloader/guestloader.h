#ifndef __GUESTLOADER_H__
#define __GUESTLOADER_H__
#include "memmap.cfg"
#define GUEST_START_ADDR    LDS_LOADER_PHYS_START
#define ZIMAGE_PHYS_ADDR    LDS_ZIMAGE_START
#define FDT_PHYS_ADDR       LDS_FDT_START
/** @brief Flag for autoboot
 *  @param flag If flag is '0', it is auto boot, otherwise CLI boot
 */
void guestloader_flag_autoboot(int flag);

#endif
