#include "hvmm_types.h"
#include "gic.h"
#include "uart_print.h"
#include "armv7_p15.h"
#include "context.h"
#include "hvmm_trace.h"
#include "vgic.h"
#include "virq.h"

hvmm_status_t hvmm_interrupt_init(void)
{
	hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

	/* Route IRQ/IFQ to Hyp Exception Vector */
	{
		uint32_t hcr;

		hcr = read_hcr(); uart_print( "hcr:"); uart_print_hex32(hcr); uart_print("\n\r");
		hcr |= HCR_IMO | HCR_FMO;
		write_hcr( hcr );
		hcr = read_hcr(); uart_print( "hcr:"); uart_print_hex32(hcr); uart_print("\n\r");
	} 

    /* Physical Interrupt: GIC Distributor & CPU Interface */
	ret = gic_init();

    /* Virtual Interrupt: GIC Virtual Interface Control */
    if ( ret == HVMM_STATUS_SUCCESS ) ret = vgic_init();
    if ( ret == HVMM_STATUS_SUCCESS ) ret = vgic_enable(1);
    if ( ret == HVMM_STATUS_SUCCESS ) ret = virq_init();

	return ret;
}
