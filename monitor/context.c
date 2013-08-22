#include "context.h"
#include "uart_print.h"
#include "hvmm_trace.h"
#include "trap.h"

#define _valid_vmid(vmid)   ( context_first_vmid() <= vmid && context_last_vmid() >= vmid )

extern void __mon_switch_to_guest_context( struct arch_regs *regs );

static struct hyp_guest_context guest_contexts[NUM_GUEST_CONTEXTS];
static int _current_guest_vmid = VMID_INVALID;
static int _next_guest_vmid = VMID_INVALID;

#ifdef BAREMETAL_GUEST
static void _hyp_fixup_unloaded_guest(void)
{
	extern uint32_t guest_bin_start;
	extern uint32_t guest_bin_end;
	extern uint32_t guest2_bin_start;

	uint32_t *src = &guest_bin_start;
        uint32_t *end = &guest_bin_end;
	uint32_t *dst = &guest2_bin_start;

	HVMM_TRACE_ENTER();

	uart_print("Copying guest0 image to guest1\n\r");
	uart_print(" src:");uart_print_hex32((uint32_t)src); 
	uart_print(" dst:");uart_print_hex32((uint32_t)dst); 
	uart_print(" size:");uart_print_hex32( (uint32_t)(end - src) * sizeof(uint32_t));uart_print("\n\r");

	while(src < end ) {
		*dst++ = *src++;
	}
	uart_print("=== done ===\n\r");
	HVMM_TRACE_EXIT();
}
#endif

void context_dump_regs( struct arch_regs *regs )
{
    int i;
    uart_print( "cpsr:" ); uart_print_hex32( regs->cpsr ); uart_print( "\n\r" );
    uart_print( "  pc:" ); uart_print_hex32( regs->pc ); uart_print( "\n\r" );

#ifdef __CONTEXT_TRACE_VERBOSE__
    uart_print( " gpr:\n\r" );
	for( i = 0; i < ARCH_REGS_NUM_GPR; i++) {
        uart_print( "     " ); uart_print_hex32( regs->gpr[i] ); uart_print( "\n\r" );
	}
#endif
}

static void context_copy_regs( struct arch_regs *regs_dst, struct arch_regs *regs_src )
{
	int i;
	regs_dst->pc = regs_src->pc;
	regs_dst->cpsr = regs_src->cpsr;
	for( i = 0; i < ARCH_REGS_NUM_GPR; i++) {
		regs_dst->gpr[i] = regs_src->gpr[i];
	}
}

/* DEPRECATED: use context_switchto(vmid) and context_perform_switch() 
    void context_switch_to_next_guest(struct arch_regs *regs_current)
 */

static hvmm_status_t context_perform_switch_to_guest_regs(struct arch_regs *regs_current, vmid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
	struct hyp_guest_context *context = 0;
	struct arch_regs *regs = 0;
	
    HVMM_TRACE_ENTER();

    if ( _current_guest_vmid == next_vmid ) {
        /* the same guest? WTF? */
        return HVMM_STATUS_IGNORED;
    }

	/*
	 * We assume VTCR has been configured and initialized in the memory management module
	 */
	/* Disable Stage 2 Translation: HCR.VM = 0 */
	hvmm_mm_stage2_enable(0);

	if ( regs_current != 0 ) {
		/* save the current guest's context */
		context = &guest_contexts[_current_guest_vmid];
		regs = &context->regs;
		context_copy_regs( regs, regs_current );
        vgic_save_status( &context->vgic_status );
	}

	/* The context of the next guest */
	context = &guest_contexts[next_vmid];

	/* Restore Translation Table for the next guest and Enable Stage 2 Translation */
	hvmm_mm_set_vmid_ttbl( context->vmid, context->ttbl );
	hvmm_mm_stage2_enable(1);
    vgic_restore_status( &context->vgic_status );

    /* The next becomes the current */
    _current_guest_vmid = next_vmid;
	if ( regs_current == 0 ) {
		/* init -> hyp mode -> guest */
		/* The actual context switching (Hyp to Normal mode) handled in the asm code */
		__mon_switch_to_guest_context( &context->regs );
	} else {
		/* guest -> hyp -> guest */
		context_copy_regs( regs_current, &context->regs );
	}

    result = HVMM_STATUS_SUCCESS;
    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t context_perform_switch(void)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;

    HVMM_TRACE_ENTER();

    uart_print( "curr:" ); uart_print_hex32( _current_guest_vmid ); uart_print( "\n\r" );
    uart_print( "next:" ); uart_print_hex32( _next_guest_vmid ); uart_print( "\n\r" );

    if ( _current_guest_vmid == VMID_INVALID ) {
        /* very first time, to the default first guest */
        result = context_perform_switch_to_guest_regs( 0, _next_guest_vmid );
        /* DOES NOT COME BACK HERE */
    } else if ( _next_guest_vmid != VMID_INVALID && _current_guest_vmid != _next_guest_vmid ) {
        struct arch_regs *regs = trap_saved_regs();
        if ( (regs->cpsr & 0x1F) != 0x1A ) {
            /* Only if not from Hyp */
            result = context_perform_switch_to_guest_regs( regs, _next_guest_vmid );
            _next_guest_vmid = VMID_INVALID;
        }
    }
    HVMM_TRACE_EXIT();
    return result;
}


void context_switch_to_initial_guest(void)
{
	struct hyp_guest_context *context = 0;
	struct arch_regs *regs = 0;

	uart_print("[hyp] switch_to_initial_guest:\n\r");

	/* Select the first guest context to switch to. */
	_current_guest_vmid = VMID_INVALID;
	context = &guest_contexts[0];

	/* Dump the initial register values of the guest for debugging purpose */
	regs = &context->regs;
	context_dump_regs( regs );

	/* Context Switch with current context == none */
    context_switchto(0);
    context_perform_switch();
}

void context_init_guests(void)
{
	struct hyp_guest_context *context;
	struct arch_regs *regs = 0;
	
	uart_print("[hyp] init_guests: enter\n\r");


	/* Guest 1 @guest_bin_start */
	context = &guest_contexts[0];
	regs = &context->regs;
	regs->pc = 0x00000000;	// PA:0xA0000000
	regs->cpsr 	= 0;	// uninitialized

	/* regs->gpr[] = whatever */
	context->vmid = 0;
	context->ttbl = hvmm_mm_vmid_ttbl(context->vmid);

	/* Guest 2 @guest2_bin_start */
	context = &guest_contexts[1];
	regs = &context->regs;
	regs->pc = 0x00000000;	// PA: 0xB0000000
	regs->cpsr 	= 0;	// uninitialized

	/* regs->gpr[] = whatever */
	context->vmid = 1;
	context->ttbl = hvmm_mm_vmid_ttbl(context->vmid);

#ifdef BAREMETAL_GUEST
	/* Workaround for unloaded bmguest.bin at 0xB0000000@PA */
	_hyp_fixup_unloaded_guest();
#endif
	uart_print("[hyp] init_guests: return\n\r");
}

vmid_t context_first_vmid(void)
{
    /* FIXME:Hardcoded for now */
    return 0;
}

vmid_t context_last_vmid(void)
{
    /* FIXME:Hardcoded for now */
    return 1;
}

vmid_t context_next_vmid(vmid_t ofvmid)
{
    vmid_t next = VMID_INVALID;
    if ( ofvmid == VMID_INVALID ) {
        next = context_first_vmid();
    } else if ( ofvmid < context_last_vmid() ) {
        /* FIXME:Hardcoded */
        next = ofvmid + 1;
    }
    return next;
}

vmid_t context_current_vmid(void)
{
    return _current_guest_vmid;
}

vmid_t context_waiting_vmid(void)
{
    return _next_guest_vmid;
}

hvmm_status_t context_switchto(vmid_t vmid)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;

    HVMM_TRACE_ENTER();

    /* valid and not current vmid, switch */
    if ( vmid != context_current_vmid() ) {
        if ( !_valid_vmid(vmid) ) {
            result = HVMM_STATUS_BAD_ACCESS;
        } else {
            _next_guest_vmid = vmid;
            result = HVMM_STATUS_SUCCESS;

            uart_print("switching to vmid:"); uart_print_hex32((uint32_t) vmid ); uart_print("\n\r");
        }
    }

    HVMM_TRACE_EXIT();
    return result;
}
