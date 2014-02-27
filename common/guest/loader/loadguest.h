#ifndef __LOADGUEST__
#define __LOADGUEST_H_
#include <guestloader.h>
#include <arch_types.h>
/**
* @brief Copying Guestloader next to guestimage.
*/
void copy_loader_next_to_guest(void);
/**
* @brief Copying guestos to start_addr.
* @param start_addr Start address of guestos.
*/
void copy_guestos_to_start_addr(uint32_t start_addr);
/**
* @brief Set tags and Load linux guest.
* @param start_addr Start address of linux guest.
*/
void load_linuxguest(uint32_t start_addr);
/**
* @brief Load bmguest.
* @param start_addr Start address of bmguest.
*/
void load_bmguest(uint32_t start_addr);
#endif

