#include <hvmm_trace.h>
#include <armv7_p15.h>
#include "trap.h"
#include "context.h"
#include "gic.h"
#include "sched_policy.h"
#include "abort_trap_handler.h"

/* By holding the address to the saved regs struct, 
   context or other modules can access to this structure
   through trap_saved_regs() call when it's needed. 
   For example, copying register values for context switching can be 
   performed this way.
 */

static struct arch_regs *_trap_hyp_saved_regs = 0;

hvmm_status_t _hyp_trap_dabort( struct arch_regs *regs )
{

    _trap_hyp_saved_regs = regs;

    context_dump_regs( regs );
    hyp_abort_infinite();

    return HVMM_STATUS_UNKNOWN_ERROR;
}

hvmm_status_t _hyp_trap_irq( struct arch_regs *regs )
{

    HVMM_TRACE_ENTER();
    _trap_hyp_saved_regs = regs;

    gic_interrupt(0, regs);

    HVMM_TRACE_EXIT();

    context_perform_switch();
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t _hyp_trap_unhandled( struct arch_regs *regs )
{
    _trap_hyp_saved_regs = regs;

    context_dump_regs( regs );
    hyp_abort_infinite();

    return HVMM_STATUS_UNKNOWN_ERROR;
}

/*
 * hvc #imm handler
 *
 * HYP Syndrome Register(HSR) 
 * EC[31:26] is an exception class for the exception that is taken to HYP mode
 * IL[25] is an instruction length for the trapped insruction that is 16 bit or 32 bit
 * ISS[24:0] is an instruction-specific syndrome for the instruction information included. It depends on EC field.
 * END OF HSR DESCRIPTION FROM ARM DDI0406_C ARCHITECTURE MANUAL
 */ 
hyp_hvc_result_t _hyp_hvc_service(struct arch_regs *regs)
{
		unsigned int hsr = read_hsr();
		unsigned int iss = hsr & 0x1FFFFFF;
		unsigned int ec = (hsr >> 26);

		_trap_hyp_saved_regs = regs;

		uart_print("[hvc] _hyp_hvc_service: enter\n\r");

    if ( ec == 0x12 && iss == 0xFFFF ) {
        uart_print("[hvc] enter hyp\n\r");
        context_dump_regs( regs );
        return HYP_RESULT_STAY;
    }

    switch( iss ) {
        case 0xFFFE:
            /* hyp_ping */
            uart_print("[hyp] _hyp_hvc_service:ping\n\r");
            context_dump_regs( regs );
            break;
        case 0xFFFD:
            {
                /* hsvc_yield() */
                uart_print("[hyp] _hyp_hvc_service:yield\n\r");
                context_dump_regs( regs );
                context_switchto(sched_policy_determ_next());
            }
            break;
        default:
            if ( ec == 0x20 ) {
                // Prefetch Abort routed to Hyp mode
            }
            if ( ec == 0x24)
            {
				uart_print("[hyp] data abort handler: hsr.iss="); uart_print_hex32(iss); uart_print("\n\r");
				abort_trap_handler(iss, regs);

				return HYP_RESULT_ERET;

				break;
            }

            uart_print("[hyp] _hyp_hvc_service:unknown hsr.iss="); uart_print_hex32( iss ); uart_print("\n\r" );
            uart_print("[hyp] hsr.ec="); uart_print_hex32( ec ); uart_print("\n\r" );
            uart_print("[hyp] hsr="); uart_print_hex32( hsr ); uart_print("\n\r" );
            context_dump_regs( regs );

            hyp_abort_infinite();
            break;
    }
    uart_print("[hyp] _hyp_hvc_service: done\n\r");

    context_perform_switch();
    return HYP_RESULT_ERET;
}

/* API */

struct arch_regs * trap_saved_regs(void)
{
    return _trap_hyp_saved_regs;
}
