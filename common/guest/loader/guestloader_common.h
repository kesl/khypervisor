#ifndef __GUESTLOADER_COMMON__
#define __GUESTLOADER_COMMON_H_
#include <guestloader.h>
#include <arch_types.h>

#define GUEST_TYPE_BM       0
#define GUEST_TYPE_LINUX    1
#define GUEST_TYPE_RTOS     2

#define START_ADDR_BM       0x80000000
#define START_ADDR_ZIMAGE   0xA0008000
#define START_ADDR_RTOS     0x80000000

#define START_ADDR_LINUX    0x80000000

#if defined(LINUX_GUEST)
#define START_ADDR START_ADDR_ZIMAGE
#define GUEST_TYPE GUEST_TYPE_LINUX
#endif

#if defined(RTOS_GUEST)
#define START_ADDR START_ADDR_RTOS
#define GUEST_TYPE GUEST_TYPE_RTOS
#endif

#if defined(BM_GUEST)
#define START_ADDR START_ADDR_BM
#define GUEST_TYPE GUEST_TYPE_BM
#endif

extern uint32_t guest_bin_start;
extern uint32_t guest_bin_end;
extern uint32_t loader_start;

/**
* @brief Loads a guest os.
* @param guest_type Guest type.
*  Types of guest OS are bmguest, Linux guest and RTOS guest.
*/
void loader_boot_guest(uint32_t guest_type);
#endif

