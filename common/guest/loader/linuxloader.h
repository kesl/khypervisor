#ifndef __LINUXLOADER_H__
#define __LINUXLOADER_H__
#include "arch_types.h"

extern unsigned initrd_start;
extern unsigned initrd_end;

/**
* @brief Sets tags that linux uses for booting.
* @param *src Base address of tags.
*/
void linuxloader_setup_atags(uint32_t src);

/**
* @brief Returns atags address.
* @return Atags address.
*/
uint32_t *linuxloader_get_atags_addr(void);

#endif
