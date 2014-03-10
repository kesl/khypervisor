#include <cli.h>
#include <guestloader_common.h>
#include <version.h>

#define hvc_dump_status()     asm("hvc #0xFFFC")

/* Command line interface's command type. */
enum cmd_type {
    HELP,
    BOOT,
    STATUS
};

#define NUM_CMD 3

/* For converting a string to command type. */
struct cmd {
    char *input_cmd;
    enum cmd_type cmd_type;
};

/* Command type mapping table. */
struct cmd cmd_type_map_tbl[NUM_CMD] = {
    {"help", HELP},
    {"boot", BOOT},
    {"status", STATUS},
};

/** @brief Converts input command to a command type.
 *  @param input Input command that will be converted to command type
 *  @return Command type. If input is wrong, then returns help.
 */
enum cmd_type convert_to_cmd_type(char *input_cmd)
{
    int i;

    for (i = 0; i < NUM_CMD; i++) {
        if (strcmp(cmd_type_map_tbl[i].input_cmd, input_cmd) == 0)
            return cmd_type_map_tbl[i].cmd_type;
    }
    return HELP;
}

/** @brief Prints commands and their usage.
 */
void print_cli_usage(void)
{
    uart_print("help         - List commands and their usage\n"
               "boot         - Boot guestos\n"
               "status       - Print the K-Hypervisor status\n");
}

/** @brief Prints K-hypervisor's status.
 *
 *  Prints K-hypervisor's banked registers (sp, lr, spsr), and vmid.
 */
void dump_hyp_status(void)
{
    uart_print("Dump K-Hypervisor's registers\n");
    hvc_dump_status();
}

#define MAX_ARG_SIZE    32
#define NULL '\0'
#define SPACE ' '
/** @brief Returns oprand of command to argv(argument value),
 *         the number of argv to argc(argument count).
 *
 *  ex) copy 0x10 0x20
 *      argv[0] = copy, argv[1] = 0x10, argv[2] = 0x20.
 *      argc = 3.
 *  @param cmd Input command.
 *  @param argv Argument value. To be stored command.
 *  @param argc Argument count, the number of argument value.
 */
void get_op_argv_argc(char *cmd, char **argv, int *argc)
{
    int pos = 0, cnt = 0;
    while (cmd[pos] && (*argc < MAX_ARG_SIZE)) {
        if ((cmd[pos] == '\r') || (cmd[pos] == '\n')) {
            cmd[pos] = NULL;
            break;
        } else if (cmd[pos] == SPACE) {
            if (cnt > 0) {
                cmd[pos] = NULL;
                cnt = 0;
            }
        } else {
            if (cnt == 0) {
                argv[*argc] = &cmd[pos];
                (*argc)++;
            }
            cnt++;
        }
        pos++;
    }
}

void cli_exec_cmd(char *cmd)
{
    int argc = 0, pos = 0, cnt = 0;
    char *argv[MAX_ARG_SIZE];

    get_op_argv_argc(cmd, argv, &argc);

    if (argc) {
        switch (convert_to_cmd_type(argv[0])) {
        case HELP:
            print_cli_usage();
            break;
        case BOOT:
            loader_boot_guest(GUEST_TYPE);
            break;
        case STATUS:
            dump_hyp_status();
            break;
        }
    }
}
