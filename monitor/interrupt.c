#include "hvmm_types.h"
#include "gic.h"

hvmm_status_t hvmm_interrupt_init(void)
{
	hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

	ret = gic_init();
	return ret;
}
