#ifndef __MONITOR_LOADER_H__
#define __MONITOR_LOADER_H__
#include <arch_types.h>
#define NUM_DEBUG_CMD 5

int monitoring_cmd(void);
void monitoring_reboot(void);
void monitoring_allset(uint32_t va);
void monitoring_register(void);
#endif
