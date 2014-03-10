#include <command.h>
#include <loadguest.h>
#include <version.h>

void command_help(int argc, char **argv)
{
    uart_print("load_cmd_help : help\n"
               "help (h)        - List commands and their usage\n\n"
               "boot (b)        - Boot guestos\n\n"
               "banner (ba)     - Showing K-hypervisor banner\n\n"
               "status (s)      - Showing hypervisor status\n\n");
}

void command_boot(int argc, char **argv)
{
    uart_print("command_boot : booting guestos...\n");
    load_guest(GUEST_TYPE);
}

void command_banner(int argc, char **argv)
{
    uart_print("command_show : show banner\n");
    uart_print(BANNER_STRING);
}

/* @todo Not implemented yet */
void command_status(int argc, char **argv)
{
    uart_print("command_status : show status\n");
}

#define ARM_MAX_ARG_SIZE    32

void command_exec(char *line)
{
    int argc = 0, pos = 0, cnt = 0;
    char *argv[ARM_MAX_ARG_SIZE];

    while (line[pos] && (argc < ARM_MAX_ARG_SIZE)) {
        if ((line[pos] == '\r') ||
                (line[pos] == '\n')) {
            line[pos] = '\0';
            break;
        }
        if (line[pos] == ' ') {
            if (cnt > 0) {
                line[pos] = '\0';
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
        if ((strcmp(argv[0], "help") == 0) ||
        (strcmp(argv[0], "h") == 0)) {
            command_help(argc, argv);
        } else if ((strcmp(argv[0], "boot") == 0) ||
        (strcmp(argv[0], "b") == 0)) {
            command_boot(argc, argv);
        } else if ((strcmp(argv[0], "banner") == 0) ||
        (strcmp(argv[0], "ba") == 0)) {
            command_banner(argc, argv);
        } else if ((strcmp(argv[0], "status") == 0) ||
        (strcmp(argv[0], "s") == 0)) {
            command_status(argc, argv);
        }
    }
}
