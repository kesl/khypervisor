
#include "hyp_config.h"
#include "uart_print.h"
#include "mm.h"
#include "armv7_p15.h"
#include "arch_types.h"

#define NUM_GUEST_CONTEXTS		NUM_GUESTS_STATIC
#define ARCH_REGS_NUM_GPR	13

//#define __DISABLE_VMM__

typedef enum {
	HYP_RESULT_ERET	= 0,
	HYP_RESULT_STAY = 1
} hyp_hvc_result_t;

struct arch_regs {
        unsigned int cpsr; /* CPSR */
        unsigned int pc; /* Program Counter */
        unsigned int gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
        unsigned int sp; /* Stack Pointer */
        unsigned int lr; /* Link Register */
} __attribute((packed));

struct hyp_guest_context {
	struct arch_regs regs;
	lpaed_t *ttbl;
	vmid_t vmid;
};

extern void __mon_switch_to_guest_context( struct arch_regs *regs );


static struct hyp_guest_context guest_contexts[NUM_GUEST_CONTEXTS];
static int current_guest = 0;


static void hyp_switch_to_next_guest(struct arch_regs *regs_current);

void uart_dump_regs( struct arch_regs *regs ) 
{
	uart_print( "cpsr:" ); uart_print_hex32( regs->cpsr ); uart_print( "\n\r" );
	uart_print( "pc:" ); uart_print_hex32( regs->pc ); uart_print( "\n\r" );
	uart_print( "sp:" ); uart_print_hex32( regs->sp ); uart_print( "\n\r" );
	uart_print( "lr:" ); uart_print_hex32( regs->lr ); uart_print( "\n\r" );
}

void hyp_abort_infinite(void)
{
	while(1);
}

/* TODO:
	- Move trap handlers to trap.c 
 */
hvmm_status_t _hyp_trap_dabort( struct arch_regs *regs )
{

	uart_dump_regs( regs );
	hyp_abort_infinite();
}
/*
 * hvc #imm handler
 */
hyp_hvc_result_t _hyp_hvc_service(struct arch_regs *regs)
{
	unsigned int hsr = read_hsr();
	unsigned int iss = hsr & 0xFFFF;
	unsigned int ec = (hsr >> 26);
	uart_print("[hvc] _hyp_hvc_service: enter\n\r");

	if ( ec == 0x12 && iss == 0xFFFF ) {
		uart_print("[hvc] enter hyp\n\r");
		uart_dump_regs( regs );
		return HYP_RESULT_STAY;
	}

	switch( iss ) {
		case 0xFFFE:
			/* hyp_ping */
			uart_print("[hyp] _hyp_hvc_service:ping\n\r");
			uart_dump_regs( regs );
			break;
		case 0xFFFD:
			/* hsvc_yield() */
			uart_print("[hyp] _hyp_hvc_service:yield\n\r");
			uart_dump_regs( regs );
			hyp_switch_to_next_guest(regs);
			break;
		default:
			uart_print("[hyp] _hyp_hvc_service:unknown hsr.iss="); uart_print_hex32( iss ); uart_print("\n\r" );
			uart_print("[hyp] hsr.ec="); uart_print_hex32( ec ); uart_print("\n\r" );
			uart_print("[hyp] hsr="); uart_print_hex32( hsr ); uart_print("\n\r" );
			uart_dump_regs( regs );
			if ( ec == 0x20 ) {
				// Prefetch Abort routed to Hyp mode
			}
			hyp_abort_infinite();
			break;
	}
	uart_print("[hyp] _hyp_hvc_service: done\n\r");
	return HYP_RESULT_ERET;
}

void hyp_init_guests(void)
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

	uart_print("[hyp] init_guests: return\n\r");
}

static void hyp_switch_to_next_guest(struct arch_regs *regs_current)
{
	struct hyp_guest_context *context = 0;
	struct arch_regs *regs = 0;
	int i;
	uint32_t hcr;
	
	/*
	 * We assume VTCR has been configured and initialized in the memory management module
	 */
#ifndef __DISABLE_VMM__
	/* Disable Stage 2 Translation: HCR.VM = 0 */
	hvmm_mm_stage2_enable(0);
#endif
	if ( regs_current != 0 ) {
		/* save the current guest's context if any */
		context = &guest_contexts[current_guest];
		regs = &context->regs;
		regs->pc = regs_current->pc;
		regs->cpsr = regs_current->cpsr;
		regs->sp = regs_current->sp;
		regs->lr = regs_current->lr;
		for( i = 0; i < ARCH_REGS_NUM_GPR; i++) {
			regs->gpr[i] = regs_current->gpr[i];
		}
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

#ifndef __DISABLE_VMM__
	/* Restore Translation Table for the next guest and Enable Stage 2 Translation */
	hvmm_mm_set_vmid_ttbl( context->vmid, context->ttbl );

	hvmm_mm_stage2_enable(1);
#endif
	
	/* The actual context switching (Hyp to Normal mode) handled in the asm code */
	__mon_switch_to_guest_context( &context->regs );
}

void hyp_switch_to_initial_guest(void)
{
	struct hyp_guest_context *context = 0;
	struct arch_regs *regs = 0;

	uart_print("[hyp] switch_to_initial_guest:\n\r");

	/* Select the first guest context to switch to. */
	current_guest = 0;
	context = &guest_contexts[current_guest];

	/* Dump the initial register values of the guest for debugging purpose */
	regs = &context->regs;
	uart_dump_regs( regs );

	/* Context Switch with current context == none */
	hyp_switch_to_next_guest( 0 );
}

void hyp_switch_guest(void)
{
	uart_print("[hyp] switch_guest: enter, not coming back\n\r");
	hyp_switch_to_initial_guest();
}

void hyp_main(void)
{
	hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;
	uart_print("[hyp_main] Starting...\n\r");

	/* Initialize Memory Management */
#ifndef __DISABLE_VMM__
	ret = hvmm_mm_init();
#endif

	/* Initialize Interrupt Management */
	ret = hvmm_interrupt_init();
	if ( ret != HVMM_STATUS_SUCCESS ) {
		uart_print("[hyp_main] interrupt initialization failed...\n\r");
	}

	/* Initialize Guests */
	hyp_init_guests();

	/* Switch to the first guest */
	hyp_switch_guest();

	/* The code flow must not reach here */
	uart_print("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n\r");
	hyp_abort_infinite();
}

