#ifndef __MONITORING_LOADER_H__
#define __MONITORING_LOADER_H__

#define NUM_DEBUG_CMD 5
/*
enum debug_cmd_type {
    DEBUG_SET,
    DEBUG_CLEAN,
    DEBUG_LIST,
    DEBUG_GO,
    DEBUG_EXIT,
    DEBUG_NOINPUT
};

struct debug_cmd {
    char *input_cmd;
    enum debug_cmd_type cmd_type;
};

struct debug_cmd debug_cmd_type_map_tbl[NUM_DEBUG_CMD] = {
    {"set", DEBUG_SET},
    {"clean", DEBUG_CLEAN},
    {"list", DEBUG_LIST},
    {"go", DEBUG_GO},
    {"exit", DEBUG_EXIT}
};
*/
int monitoring_cmd(void);

#endif
