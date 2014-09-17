#include <monitoring_loader.h>
#include <arch_types.h>

#define VDEV_MONITORING_BASE 0x3FFFD000

volatile uint32_t *base_set =   (uint32_t *) VDEV_MONITORING_BASE;
volatile uint32_t *base_clean = (uint32_t *) (VDEV_MONITORING_BASE + 0x4);
volatile uint32_t *base_list =  (uint32_t *) (VDEV_MONITORING_BASE + 0x8);
volatile uint32_t *base_break = (uint32_t *) (VDEV_MONITORING_BASE + 0xC);
volatile uint32_t *base_go =    (uint32_t *) (VDEV_MONITORING_BASE + 0x10);
volatile uint32_t *base_break_clean =
                                (uint32_t *) (VDEV_MONITORING_BASE + 0x14);

#define monitoring_list()  (*base_list)
#define NUM_MONITORING_CMD 8

#define MAX_INPUT_SIZE    256
#define MAX_CMD_SIZE    32
#define NULL '\0'
#define SPACE ' '

enum monitoring_cmd_type {
    MONITORING_SET,
    MONITORING_CLEAN,
    MONITORING_LIST,
    MONITORING_BREAK,
    MONITORING_GO,
    MONITORING_HELP,
    MONITORING_EXIT,
    MONITORING_BREAK_CLEAN,
    MONITORING_NOINPUT
};

struct monitoring_cmd {
    char *input_cmd;
    enum monitoring_cmd_type cmd_type;
};

static struct monitoring_cmd monitoring_cmd_type_map_tbl[NUM_MONITORING_CMD] = {
    {"set", MONITORING_SET},
    {"clean", MONITORING_CLEAN},
    {"list", MONITORING_LIST},
    {"break", MONITORING_BREAK},
    {"go", MONITORING_GO},
    {"help", MONITORING_HELP},
    {"break_clean", MONITORING_BREAK_CLEAN},
    {"exit", MONITORING_EXIT}
};

static void monitoring_help(void)
{
    uart_print("help            - Display this information\n"
               "set <address>   - Set monitoring point\n"
               "break <address> - Set monitoring point and break point\n"
               "list            - Display setted pointres\n"
               "go              - unlock break\n"
               "clean <address> - Clean monitoring point\n"
               "break_clean <address> - Clean breaking point\n"
               "exit            - exit monitoring mode\n");
}

static void monitoring_clean(char **argv, int argc)
{
    uint32_t va;

    if ((argc != 2) || !((argv[1][0] == '0') && (argv[1][1] == 'x'))) {
        monitoring_help();
        return;
    }
    uart_print(argv[1]);
    va = arm_hexstr2uint(argv[1]);
    if (va > 0xFFFFFFFF) {
        monitoring_help();
        return;
    }
    uart_print("clean monitoring point\n");
    uart_print_hex32(va);
    uart_print("\n");
    *base_clean = va;
}

static void monitoring_set(char **argv, int argc)
{
    uint32_t va;
    if ((argc != 2) || !((argv[1][0] == '0') && (argv[1][1] == 'x'))) {
        monitoring_help();
        return;
    }
    va = arm_hexstr2uint(argv[1]);
    if (va > 0xFFFFFFFF) {
        monitoring_help();
        return;
    }
    uart_print("set_monitoring_point\n");
    uart_print_hex32(va);
    uart_print("\n");
    *base_set = va;
}

static void monitoring_break(char **argv, int argc)
{
    uint32_t va;
    if ((argc != 2) || !((argv[1][0] == '0') && (argv[1][1] == 'x'))) {
        monitoring_help();
        return;
    }
    va = arm_hexstr2uint(argv[1]);
    if (va > 0xFFFFFFFF) {
        monitoring_help();
        return;
    }
    uart_print("set_monitoring_point and break mode\n");
    uart_print_hex32(va);
    uart_print("\n");
    *base_break = va;
}
static void monitoring_break_clean(char **argv, int argc)
{
    uint32_t va;

    if ((argc != 2) || !((argv[1][0] == '0') && (argv[1][1] == 'x'))) {
        monitoring_help();
        return;
    }
    uart_print(argv[1]);
    va = arm_hexstr2uint(argv[1]);
    if (va > 0xFFFFFFFF) {
        monitoring_help();
        return;
    }
    uart_print("clean breaking point\n");
    uart_print_hex32(va);
    uart_print("\n");
    *base_break_clean = va;
}

static void monitoring_go()
{
    uart_print("Go!\n");
    *base_go;
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
        case MONITORING_EXIT:
            return 0;
        }
        {
        int i;
            for (i = 0; i < 32; i++)
                argv[i] = NULL;
        }
    }
    return 0;
}

