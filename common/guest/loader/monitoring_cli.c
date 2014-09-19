#include <monitoring_cli.h>
#include <monitoring.h>
#include <arch_types.h>
#include <gic.h>
#define DEBUG
#include <log/print.h>
#include <log/uart_print.h>

#define VDEV_MONITORING_BASE 0x3FFFD000

volatile uint32_t *base_set =   (uint32_t *) VDEV_MONITORING_BASE;
volatile uint32_t *base_clean = (uint32_t *) (VDEV_MONITORING_BASE + 0x4);
volatile uint32_t *base_list =  (uint32_t *) (VDEV_MONITORING_BASE + 0x8);
volatile uint32_t *base_break = (uint32_t *) (VDEV_MONITORING_BASE + 0xC);
volatile uint32_t *base_go =    (uint32_t *) (VDEV_MONITORING_BASE + 0x10);
volatile uint32_t *base_break_clean =
                                (uint32_t *) (VDEV_MONITORING_BASE + 0x14);
volatile uint32_t *base_all_clean =
                                (uint32_t *) (VDEV_MONITORING_BASE + 0x18);
volatile uint32_t *base_memory_dump =
                                (uint32_t *) (VDEV_MONITORING_BASE + 0x1C);

#define monitoring_list()  (*base_list)
#define NUM_MONITORING_CMD MONITORING_NOINPUT

#define MAX_INPUT_SIZE    256
#define MAX_CMD_SIZE    32
#define NULL '\0'
#define SPACE ' '

enum monitoring_cmd_type {
    MONITORING_SET = 0,
    MONITORING_CLEAN,
    MONITORING_LIST,
    MONITORING_BREAK,
    MONITORING_GO,
    MONITORING_HELP,
    MONITORING_EXIT,
    MONITORING_BREAK_CLEAN,
    MONITORING_ALL_CLEAN,
    MONITORING_MEMORY_DUMP,
    MONITORING_NOINPUT
};

enum cmd_status {
    CMD_STATUS_SUCCESS = 0,
    CMD_STATUS_ERROR = -1
};

struct monitoring_cmd {
    char *input_cmd;
    enum monitoring_cmd_type cmd_type;
};


static struct monitoring_cmd monitoring_cmd_type_map_tbl[NUM_MONITORING_CMD] = {
    {"mp", MONITORING_SET},
    {"mc", MONITORING_CLEAN},
    {"ls", MONITORING_LIST},
    {"bp", MONITORING_BREAK},
    {"go", MONITORING_GO},
    {"h", MONITORING_HELP},
    {"bc", MONITORING_BREAK_CLEAN},
    {"clear", MONITORING_ALL_CLEAN},
    {"x", MONITORING_MEMORY_DUMP},
    {"exit", MONITORING_EXIT}
};

static void monitoring_help(void)
{
    uart_print("h                   - Display list of commands\n"
               "mp <address>        - Set monitoring point\n"
               "bp <address>        - Set break point\n"
               "ls                  - Display setted pointres\n"
               "go                  - unlock break\n"
               "mc <address>        - Clean monitoring point\n"
               "bc <address>        - Clean breaking point\n"
               "clear               - Clean all monitoring point\n"
               "x <range> <address> - Dump memory. [x 20 0x30000000]\n"
               "exit                - exit monitoring mode\n");
}

static enum cmd_status check_vaild_va(char **argv, int argc, uint32_t *va)
{
    int i = 0;
    if ((argc != 2) || !((argv[1][0] == '0') && (argv[1][1] == 'x'))) {
        monitoring_help();
        return CMD_STATUS_ERROR;
    }
    while (argv[1][i])
        i++;
    *va = arm_hexstr2uint(argv[1]);
    if (i > 10) {
        monitoring_help();
        return CMD_STATUS_ERROR;
    }
    return CMD_STATUS_SUCCESS;
}

static void monitoring_clean(char **argv, int argc)
{
    uint32_t va;
    if (check_vaild_va(argv, argc, &va) == CMD_STATUS_SUCCESS) {
        printh("clean monitoring point %x\n", va);
        *base_clean = va;
    }
}

static void monitoring_set(char **argv, int argc)
{
    uint32_t va;
    if (check_vaild_va(argv, argc, &va) == CMD_STATUS_SUCCESS) {
        printh("set monitoring point %x\n", va);
        *base_set = va;
    }
}

static void monitoring_break(char **argv, int argc)
{
    uint32_t va;
    if (check_vaild_va(argv, argc, &va) == CMD_STATUS_SUCCESS) {
        printh("set monitoring porint and break mode %x\n", va);
        *base_break = va;
    }
}
static void monitoring_break_clean(char **argv, int argc)
{
    uint32_t va;
    if (check_vaild_va(argv, argc, &va) == CMD_STATUS_SUCCESS) {
        printh("clean breaking point %x\n", va);
        *base_break_clean = va;
    }
}

static void monitoring_go(void)
{
    printh("Go!\n");
    *base_go;
}

static void monitoring_all_clean(void)
{
    printh("Clean all monitoring point!\n");
    *base_all_clean;
}

static void monitoring_dump_memory(char **argv, int argc)
{
    uint32_t va, cnt, i = 0;
    if ((argc != 3) || !((argv[2][0] == '0') && (argv[2][1] == 'x'))) {
        monitoring_help();
        return;
    }
    cnt = arm_str2int(argv[1]);
    va = arm_hexstr2uint(argv[2]);
    while (argv[2][i])
        i++;
    /* 1048512 is max shared memory range */
    if (i > 10 || cnt > 1048512) {
        monitoring_help();
        return;
    }
    send_monitoring_data(cnt, va);
    *base_memory_dump;
}
static enum monitoring_cmd_type convert_to_monitoring_cmd_type(char *input_cmd)
{
    int i;

    for (i = 0; i < NUM_MONITORING_CMD; i++) {
        if (strcmp(monitoring_cmd_type_map_tbl[i].input_cmd, input_cmd) == 0)
            return monitoring_cmd_type_map_tbl[i].cmd_type;
    }
    return MONITORING_HELP;
}

static enum monitoring_cmd_type get_monitoring_cmd
                                    (char *input_cmd, char **argv, int *p_argc)
{
    int pos = 0, argc = 0, cnt = 0;
    while (input_cmd[pos] && (argc < MAX_CMD_SIZE)) {
        if ((input_cmd[pos] == '\r') || (input_cmd[pos] == '\n')) {
            input_cmd[pos] = NULL;
            break;
        } else if (input_cmd[pos] == SPACE) {
            if (cnt > 0) {
                input_cmd[pos] = NULL;
                cnt = 0;
            }
        } else if (input_cmd[pos] != SPACE) {
            if (cnt == 0) {
                argv[argc] = &input_cmd[pos];
                argc++;
            }
            cnt++;
        }
        pos++;
    }
    *p_argc = argc;
    if (argc)
        return convert_to_monitoring_cmd_type(argv[0]);
    return MONITORING_NOINPUT;
}

int monitoring_cmd(void)
{
    volatile unsigned int *base_set = (unsigned int *) VDEV_MONITORING_BASE;
    char input_cmd[MAX_INPUT_SIZE];
    char *argv[MAX_CMD_SIZE];
    int argc;
    while (1) {
        uart_print("monitoring# ");
        uart_gets(input_cmd, MAX_INPUT_SIZE);
        switch (get_monitoring_cmd(input_cmd, argv, &argc)) {
        case MONITORING_SET:
            monitoring_set(argv, argc);
            break;
        case MONITORING_CLEAN:
            monitoring_clean(argv, argc);
            break;
        case MONITORING_LIST:
            monitoring_list();
            break;
        case MONITORING_BREAK:
            monitoring_break(argv, argc);
            break;
        case MONITORING_GO:
            monitoring_go();
            break;
        case MONITORING_HELP:
            monitoring_help();
            break;
        case MONITORING_BREAK_CLEAN:
            monitoring_break_clean(argv, argc);
            break;
        case MONITORING_ALL_CLEAN:
            monitoring_all_clean();
            break;
        case MONITORING_MEMORY_DUMP:
            monitoring_dump_memory(argv, argc);
            break;
        case MONITORING_EXIT:
            monitoring_all_clean();
            return 0;
        }
    }
    return 0;
}

