#include "sched_policy.h"
#include "context.h"

vmid_t sched_policy_determ_next(void)
{
    vmid_t next = context_next_vmid(context_current_vmid());
    if ( next == VMID_INVALID )
		next = context_first_vmid();
    return next;
}
