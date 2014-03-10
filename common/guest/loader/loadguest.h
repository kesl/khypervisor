#ifndef __LOADGUEST__
#define __LOADGUEST_H_
#include <guestloader.h>
#include <arch_types.h>
#define GUEST_TYPE_BM       0
#define GUEST_TYPE_LINUX    1
#define GUEST_TYPE_RTOS     2
#define START_ADDR_BM       0x80000000
#define START_ADDR_LINUX    0xA0008000
#define START_ADDR_RTOS     0x80000000
#if defined(LINUX_GUEST)
#define START_ADDR START_ADDR_LINUX
#define GUEST_TYPE GUEST_TYPE_LINUX
#endif
#if defined(RTOS_GUEST)
#define START_ADDR START_ADDR_BM
#define GUEST_TYPE GUEST_TYPE_BM
#endif
#if defined(BM_GUEST)
#define START_ADDR START_ADDR_RTOS
#define GUEST_TYPE GUEST_TYPE_RTOS
#endif
extern uint32_t guest_bin_start;
extern uint32_t guest_bin_end;
extern uint32_t loader_start;
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
/**
* @brief Load guest.
* @param type Guest type. There are bmguest and Linux guest.
*/
void load_guest(uint32_t type);
#endif

