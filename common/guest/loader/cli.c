#include <cli.h>
#include <guestloader_common.h>
#include <version.h>

#define hvc_dump_status()     asm("hvc #0xFFFC")

/* Guest Loader's command line interface command type. */
enum cmd_type {
    HELP,
    BOOT,
    STATUS,
    NOINPUT
};

#define NUM_CMD 3

/* For converting a string to command type. */
struct cmd {
    char *input_cmd;
    enum cmd_type cmd_type;
};

/* Command type mapping table. */
static struct cmd cmd_type_map_tbl[NUM_CMD] = {
    {"help", HELP},
    {"boot", BOOT},
    {"status", STATUS}
};

/** @brief Converts input command to a command type.
 *  @param input Input command that will be converted to command type
 *  @return Command type. If input is wrong, then returns help.
 */
static enum cmd_type convert_to_cmd_type(char *input_cmd)
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
static void print_cli_usage(void)
{
    uart_print("help         - List commands and their usage\n"
               "boot         - Boot guestos\n"
               "status       - Print the K-Hypervisor status\n");
}

/** @brief Prints K-hypervisor's status.
 *
 *  Prints K-hypervisor's banked registers (sp, lr, spsr), and vmid.
 */
static void dump_hyp_status(void)
{
    uart_print("Dump K-Hypervisor's registers\n");
    hvc_dump_status();
}

#define MAX_ARG_SIZE    32
#define NULL '\0'
#define SPACE ' '
/** @brief Gets command type from input command.
 *
 *  @param input_cmd Input command.
 *  @return Command type.
 */
static enum cmd_type get_cmd_type(char *input_cmd)
{
    int pos = 0;
    while (input_cmd[pos]) {
        if ((input_cmd[pos] == '\r') || (input_cmd[pos] == '\n'))
            break;
        else if (input_cmd[pos] != SPACE)
            return convert_to_cmd_type(&input_cmd[pos]);
        pos++;
    }
    return NOINPUT;
}

void cli_exec_cmd(char *cmd)
{
    switch (get_cmd_type(cmd)) {
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
