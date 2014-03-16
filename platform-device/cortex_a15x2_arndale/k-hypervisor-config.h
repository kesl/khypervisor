#ifndef KHYPERVISOR_CONFIG_H
#define KHYPERVISOR_CONFIG_H

#include <config/cfg_platform.h>

#define USEC 1000000

#define NUM_GUESTS_STATIC       2
#define COUNT_PER_USEC (CFG_CNTFRQ/USEC)
#define GUEST_SCHED_TICK 100000

#endif  /* KHYPERVISOR_CONFIG_H */
