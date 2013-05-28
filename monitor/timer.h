#ifndef __TIMER_H__
#define __TIMER_H__

typedef enum {
	TIMER_STATUS_SUCCESS = 0,
	TIMER_STATUS_UNKNOWN_ERROR = -1,
	/* Add other error status from here */
} timer_status_t;

timer_status_t timer_init(void);
#endif
