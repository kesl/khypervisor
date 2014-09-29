#ifndef __GUEST_MONITOR_H__
#define __GUEST_MONITOR_H__

#include <arch_types.h>
#define MAX_LENGTH_SYMBOL 50

struct system_map {
    uint32_t address;
    uint8_t type;
    char symbol[MAX_LENGTH_SYMBOL];
};

extern uint32_t shared_memory_start;
extern uint32_t shared_memory_end;
extern uint32_t system_map_start;
void show_symbol(uint32_t va);
void sysmbol_parser_init(void);
int symbol_getter_from_va(uint32_t va, char *symbol);
void monitoring_init(void);
void send_monitoring_data(uint32_t range, uint32_t src);
void reboot(void);
void set_recovery(int cnt);

extern uint32_t loader_start;
extern uint32_t restore_start;
extern uint32_t restore_end;
extern uint32_t guestloader_end;
extern uint32_t restore_guest_start;
extern uint32_t restore_guest_end;
extern uint32_t guest_start;
#endif
