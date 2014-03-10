#ifndef __GUESTLOADER_H__
#define __GUESTLOADER_H__

#define MACHINE_TYPE      2272

/** @brief Flag for autoboot
 *  @param flag If flag is '0', it is auto boot, otherwise CLI boot
 */
void guestloader_flag_autoboot(int flag);

#endif
