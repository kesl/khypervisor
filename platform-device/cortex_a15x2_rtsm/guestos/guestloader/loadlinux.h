#ifndef __LOADLINUX_H__
#define __LOADLINUX_H__
#include "arch_types.h"

/** 
* @brief Set tags that linux uses for booting.
* @param *src Address of tags
*/
void loadlinux_setup_tags( uint32_t *src );
/**
* @brief Input tags and machine id to r1, r2 register. And Mov pc to zImage's start address.
* @param start_addr Start address of compressed kernel.
*/
void loadlinux_run_zImage( uint32_t start_addr );

#endif //__LOADLINUX_H__
