#ifndef __COMMAND_H__
#define __COMMAND_H__
/**
 * @brief Executes command.
 *
 * help (h)        - List commands and their usage
 * boot (b)        - Boot guestos
 * banner (ba)     - Showing K-hypervisor banner
 * status (s)      - Showing hypervisor status
 *
 * @param line The string of command.
 */
void command_exec(char *line);
#endif
