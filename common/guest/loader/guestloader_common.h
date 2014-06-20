#ifndef __GUESTLOADER_COMMON__
#define __GUESTLOADER_COMMON_H_
#include <guestloader.h>
#include <arch_types.h>

#define GUEST_TYPE_BM       0
#define GUEST_TYPE_LINUX    1
#define GUEST_TYPE_RTOS     2

#define START_ADDR_BM       0x80000000
//#define START_ADDR_ZIMAGE   0x80008000
#define START_ADDR_ZIMAGE   0x80008040
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

extern uint32_t guest_start;
extern uint32_t guest_end;
extern uint32_t loader_start;
extern uint32_t loader_end;

/**
* @brief Loads a guest os.
* @param guest_os_type Guest os type.
*  Types of guest OS are bmguest, Linux guest and RTOS guest.
*/
void loader_boot_guest(uint32_t guest_os_type);
#endif

