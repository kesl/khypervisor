#include <log/uart_print.h>
#include <command.h>
#include <version.h>

#define MAX_CMD_STR_SIZE    256

void main(void)
{
    char line[MAX_CMD_STR_SIZE];
    uart_init();
    uart_print(BANNER_STRING);
    while (1) {
        uart_print("kboot# ");
        uart_gets(line, MAX_CMD_STR_SIZE, '\n');
        load_exec(line);
    }
}
