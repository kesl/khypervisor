#ifndef __CMD_MENU_H__
#define __CMD_MENU_H__

/**
 * @brief Executes command.
 *
 * help (h)        - List commands and their usage
 * boot (b)        - Boot guestos
 * banner          - Print The K-Hypervisor banner
 * status (s)      - Print The K-Hypervisor status
 *
 * @param line The string of command.
 */
void cmd_exec(char *line);
#endif
