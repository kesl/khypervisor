#ifndef __TIMER_H__
#define __TIMER_H__

/**
 * @brief Registers timer handler and enables timer interrupt.
 */
void timer_init(void);
/**
 * @brief Disables receiving virtual timer interrupt.
 */
void timer_disable(void);
/**
 * @brief Enables receiving virtual timer interrupt.
 */
void timer_enable(void);
#endif
