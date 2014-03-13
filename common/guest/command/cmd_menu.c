#include <cmd_menu.h>
#include <loadguest.h>
#include <version.h>

enum cmd_val {
    HELP,
    BOOT,
    BANNER,
    STATUS,
    BADNUM = -1
};

struct cmd {
    char *key;
    int val;
};

static struct cmd cmdtbl[] = {
    {"help", HELP},
    {"h", HELP},
    {"boot", BOOT},
    {"b", BOOT},
    {"banner", BANNER},
    {"status", STATUS},
    {"s", STATUS}
};

#define NUM_KEY (sizeof(cmdtbl) / sizeof(struct cmd))
#define MAX_ARG_SIZE    32
#define NULL '\0'

/** @brief Return command to integer type from string type.
 *  @param key Command that will be converted to integer type.
 *  @return Integer type command value.
 */
enum cmd_val cmd_ktov(char *key)
{
    int i;
    for (i = 0; i < NUM_KEY; i++) {
        struct cmd *cmd = cmdtbl + i;
        if (strcmp(cmd->key, key) == 0)
            return cmd->val;
    }
    return BADNUM;
}

/** @brief Prints commands and their usage.
 */
void cmd_usage(void)
{
    uart_print("help (h)        - List commands and their usage\n\n"
               "boot (b)        - Boot guestos\n\n"
               "banner          - Print the K-Hypervisor banner\n\n"
               "status (s)      - Print the K-Hypervisor status\n\n");
}

/** @brief Boots guest os.
 */
void cmd_run_guest(void)
{
    uart_print("Booting guest os...\n");
    load_guest(GUEST_TYPE);
}

/** @brief Prints K-Hypervisor's banner.
 */
void cmd_print_banner(void)
{
    uart_print(BANNER_STRING);
}

/** @brief Prints K-hypervisor's status.
 *  @todo (InKyu Han)Not implemented yet
 */
void cmd_dump_hyp_status(void)
{
    uart_print("Dump K-Hypervisor's registers\n");
}

/** @brief Exeutes command line
 *  @param line Command to exeute.
 */
void cmd_exec(char *line)
{
    int argc = 0, pos = 0, cnt = 0;
    char *argv[MAX_ARG_SIZE];

    while (line[pos] && (argc < MAX_ARG_SIZE)) {
        if ((line[pos] == '\r') || (line[pos] == '\n')) {
            line[pos] = NULL;
            break;
        }
        if (line[pos] == ' ') {
            if (cnt > 0) {
                line[pos] = NULL;
                cnt = 0;
            }
        } else {
            if (cnt == 0) {
                argv[argc] = &line[pos];
                argc++;
            }
            cnt++;
        }
        pos++;
    }

    if (argc) {
        switch (cmd_ktov(argv[0])) {
        case HELP:
            cmd_usage();
            break;
        case BOOT:
            cmd_run_guest();
            break;
        case BANNER:
            cmd_print_banner();
            break;
        case STATUS:
            cmd_dump_hyp_status();
            break;
        }
    }
}
