#include "context.h"
#include "uart_print.h"
#include "hvmm_trace.h"

extern void __mon_switch_to_guest_context( struct arch_regs *regs );

static struct hyp_guest_context guest_contexts[NUM_GUEST_CONTEXTS];
static int current_guest = 0;

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

static void context_copy_regs( struct arch_regs *regs_dst, struct arch_regs *regs_src )
{
	int i;
	regs_dst->pc = regs_src->pc;
	regs_dst->cpsr = regs_src->cpsr;
	regs_dst->sp = regs_src->sp;
	regs_dst->lr = regs_src->lr;
	for( i = 0; i < ARCH_REGS_NUM_GPR; i++) {
		regs_dst->gpr[i] = regs_src->gpr[i];
	}
}

void context_switch_to_next_guest(struct arch_regs *regs_current)
{
	struct hyp_guest_context *context = 0;
	struct arch_regs *regs = 0;
	
	/*
	 * We assume VTCR has been configured and initialized in the memory management module
	 */
	/* Disable Stage 2 Translation: HCR.VM = 0 */
	hvmm_mm_stage2_enable(0);

	if ( regs_current != 0 ) {
		/* save the current guest's context if any */
		context = &guest_contexts[current_guest];
		regs = &context->regs;
		context_copy_regs( regs, regs_current );
	}

	if ( regs_current != 0 ) {
		/* load the next guest */
		current_guest = (current_guest + 1) % NUM_GUEST_CONTEXTS;
	} else {
		/* There is no current guest switching from. 
		 * We choose the very first one as the initial guest to switch to 
		 */
		current_guest = 0;
	}

	/* The context of the chosen next guest */
	context = &guest_contexts[current_guest];

	/* Restore Translation Table for the next guest and Enable Stage 2 Translation */
	hvmm_mm_set_vmid_ttbl( context->vmid, context->ttbl );
	hvmm_mm_stage2_enable(1);
	
	if ( regs_current == 0 ) {
		/* init -> hyp mode -> guest */
		/* The actual context switching (Hyp to Normal mode) handled in the asm code */
		__mon_switch_to_guest_context( &context->regs );
	} else {
		/* guest -> hyp -> guest */
		context_copy_regs( regs_current, &context->regs );
	}
}

void context_dump_regs( struct arch_regs *regs )
{
        uart_print( "cpsr:" ); uart_print_hex32( regs->cpsr ); uart_print( "\n\r" );
        uart_print( "pc:" ); uart_print_hex32( regs->pc ); uart_print( "\n\r" );
        uart_print( "sp:" ); uart_print_hex32( regs->sp ); uart_print( "\n\r" );
        uart_print( "lr:" ); uart_print_hex32( regs->lr ); uart_print( "\n\r" );
}

static void context_switch_to_initial_guest(void)
{
	struct hyp_guest_context *context = 0;
	struct arch_regs *regs = 0;

	uart_print("[hyp] switch_to_initial_guest:\n\r");

	/* Select the first guest context to switch to. */
	current_guest = 0;
	context = &guest_contexts[current_guest];

	/* Dump the initial register values of the guest for debugging purpose */
	regs = &context->regs;
	context_dump_regs( regs );

	/* Context Switch with current context == none */
	context_switch_to_next_guest( 0 );
}

void context_switch_guest(void)
{
	uart_print("[hyp] switch_guest: enter, not coming back\n\r");
	context_switch_to_initial_guest();
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
	regs->sp	= 0;	// uninitialized
	regs->lr	= 0;	// uninitialized

	/* regs->gpr[] = whatever */
	context->vmid = 0;
	context->ttbl = hvmm_mm_vmid_ttbl(context->vmid);

	/* Guest 2 @guest2_bin_start */
	context = &guest_contexts[1];
	regs = &context->regs;
	regs->pc = 0x00000000;	// PA: 0xB0000000
	regs->cpsr 	= 0;	// uninitialized
	regs->sp	= 0;	// uninitialized
	regs->lr	= 0;	// uninitialized

	/* regs->gpr[] = whatever */
	context->vmid = 1;
	context->ttbl = hvmm_mm_vmid_ttbl(context->vmid);

#ifdef BAREMETAL_GUEST
	/* Workaround for unloaded bmguest.bin at 0xB0000000@PA */
	_hyp_fixup_unloaded_guest();
#endif
	uart_print("[hyp] init_guests: return\n\r");
}
