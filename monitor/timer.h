#ifndef __TIMER_H__
#define __TIMER_H__

#include "hvmm_types.h"
#include "arch_types.h"
/**
 *  Implements Timer functionality such as,
 *
 *	- timer channel: periodic callback at a given time interval
 *  - current time: time since boot-up
 *
 *	==Example Usage==
 *
 *	Initialize the timer module
 *		timer_init(timer_sched);
 *
 *	Set interval 10 to timer_sched channel
 *		timer_set_interval( timer_sched, 10 );
 *
 *	Set cb1 callback function to timer_scehd channel.
 *	cb1 call after 10 interval.
 *		timer_set_callback( timer_sched, &cb1 );
 *
 *	Start timer_sched channel. cb1 call after 10 interval.
 *		timer_start( timer_sched );
 *
 *		...
 *
 *	Remove the call back
 *		timer_remove_callback(timer_sched, &cb1);
*/


/*
*	Timer channel
*	Have own's timer interrupt handler.
*/
typedef enum{
    timer_sched = 0,
    TIMER_NUM_MAX_CHANNELS
} timer_channel_t;

/*
*   tv_sec : seconds
*   tv_usec : microseconds
*/
struct timeval{
    long tv_sec;
    long tv_usec;
};

/*
*	timer_callback_t
*/
typedef void(*timer_callback_t)(void *pdata);

///////////////////////////////////
//TODO Need organization this struct.
/*
*	Timer device list sturct.
*/

struct timer_source{
	const char *name;
//	hvmm_status_t (*init) (void);
	hvmm_status_t (*start) (uint32_t interval);
	hvmm_status_t (*stop) (void);
	hvmm_status_t (*add_callback)(int, timer_callback_t);
	hvmm_status_t (*remove_callback)(int, timer_callback_t);
	void (*get_time)(struct timeval*);	
//	uint32_t interval;
};
	
struct timer_channel{
	struct timer_source ts;
	uint32_t interval;
	timer_callback_t callback;
};

/*
 * TODO Need move other source, e.g. module_init.c, list.c
 */
struct timer_source_list{
	struct timer_source source;
	struct timer_source_list *next;
	struct timer_source_list *prev;
};
struct dlist{
	struct dlist *next, *prev;
};

///////////////////////////////////

/**
 * Calling this function is required only once in the entire system prior to calls 
 * to other functions of Timer module.
 */
	hvmm_status_t timer_init(timer_channel_t channel);

/**
 * Starts the timer channel specified by 'channel'. The callback, 
 * if set, will be periodically called until it's unset or the channel stops by 
 * 'timer_stop(timer_channel)'
 */
	hvmm_status_t timer_start(timer_channel_t channel);

/**
 *	Stops the timer channel specified by 'channel' 
 */
	hvmm_status_t timer_stop(timer_channel_t channel);

/**
 * Sets time interval, in (TBD)seconds, for the timer channel. 
 * If the channel has been started and a callback function is set, it will be called 
 * in the next interval
 */
	hvmm_status_t timer_set_interval(timer_channel_t channel, uint32_t interval);

/**
 * Returns the time interval for the timer channel if it was set previously. 
 * Unknown value is returned otherwise.
 */
	hvmm_status_t timer_get_interval(timer_channel_t channel, uint32_t *interval);

/**
 * Adds a callback function for the timer channel.
 */
	hvmm_status_t timer_add_callback(timer_channel_t channel, timer_callback_t handler);
/**
 * Removes the callback function from the timer channel's registered callback function list 
 * if it previously has been added.
 */
	hvmm_status_t timer_remove_callback(timer_channel_t channel, timer_callback_t handler);
/**
 * Returns current time since boot-up.
 */
void timer_get_time(struct timeval* timeval);

/**
 * Register timer source.
 */
hvmm_status_t timer_register(struct timer_source* ts);

void timer_test_scheduling();
#endif
