#include <command.h>
#include <loadguest.h>
#include <version.h>
#define ARM_MAX_ARG_SIZE    32
void load_cmd_help(int argc, char **argv)
{
    uart_print("load_cmd_help : help\n\r");
    uart_print("help(h)        - List commands and their usage\n");
    uart_print("\n");
    uart_print("boot(b)        - Boot guestos\n");
    uart_print("\n");
    uart_print("banner(ba)     - Showing K-hypervisor banner\n");
    uart_print("\n");
    uart_print("status(s)      - Showing hypervisor status\n");
    uart_print("\n");

}
void load_cmd_boot(int argc, char **argv)
{
    uart_print("load_cmd_boot : booting guestos...\n");
    load_guest(GUEST_TYPE);
}
void load_cmd_banner(int argc, char **argv)
{
    uart_print("load_cmd_show : show banner\n");
    uart_print(BANNER_STRING);
}
void load_cmd_status(int argc, char **argv)
{
    uart_print("load_cmd_status : show status\n");
}
void load_exec(char *line)
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
        if ((strcmp(argv[0], "help") == 0)
                || (strcmp(argv[0], "h") == 0)) {
            load_cmd_help(argc, argv);
        } else if ((strcmp(argv[0], "boot") == 0)
                || (strcmp(argv[0], "b") == 0)) {
            load_cmd_boot(argc, argv);
        } else if ((strcmp(argv[0], "banner") == 0)
                || (strcmp(argv[0], "ba") == 0)) {
            load_cmd_banner(argc, argv);
        } else if ((strcmp(argv[0], "status") == 0)
                || (strcmp(argv[0], "s") == 0)) {
            load_cmd_status(argc, argv);
        }
    }
}
