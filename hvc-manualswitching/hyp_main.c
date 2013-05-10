#include "uart_print.h"

#define read_hsr()              ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c5, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})
#define read_cpsr()              ({ unsigned int rval; asm volatile(\
                                " mrs     %0, cpsr\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})
typedef enum {
	HYP_RESULT_ERET	= 0,
	HYP_RESULT_STAY = 1
} hyp_hvc_result_t;

#define ARCH_REGS_NUM_GPR	13
struct arch_regs {
        unsigned int cpsr; /* CPSR */
        unsigned int pc; /* Program Counter */
        unsigned int gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
        unsigned int sp; /* Stack Pointer */
        unsigned int lr; /* Link Register */
} __attribute((packed));

extern void __mon_switch_to_guest_context( struct arch_regs *regs );

void hyp_switch_to_next_guest(struct arch_regs *regs_current);

void uart_dump_regs( struct arch_regs *regs ) 
{
	uart_print( "cpsr:" ); uart_print_hex32( regs->cpsr ); uart_print( "\n\r" );
	uart_print( "pc:" ); uart_print_hex32( regs->pc ); uart_print( "\n\r" );
	uart_print( "sp:" ); uart_print_hex32( regs->sp ); uart_print( "\n\r" );
	uart_print( "lr:" ); uart_print_hex32( regs->lr ); uart_print( "\n\r" );
}

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
			// hyp_ping
			uart_print("[hyp] _hyp_hvc_service:ping\n\r");
			uart_dump_regs( regs );
			break;
		case 0xFFFD:
			// hsvc_yield()
			uart_print("[hyp] _hyp_hvc_service:yield\n\r");
			uart_dump_regs( regs );
			hyp_switch_to_next_guest(regs);
			break;
		default:
			uart_print("[hyp] _hyp_hvc_service:unknown iss=");
			uart_print_hex32( iss );
			uart_print("\n\r" );
			uart_dump_regs( regs );
			break;
	}
	uart_print("[hyp] _hyp_hvc_service: done\n\r");
	return HYP_RESULT_ERET;
}

#define NUM_GUEST_CONTEXTS		2
static struct arch_regs guest_contexts[NUM_GUEST_CONTEXTS];
static int current_guest = 0;
extern void *guest_start;
extern void *guest2_start;

void hyp_init_guests(void)
{
	struct arch_regs *context = 0;
	
	uart_print("[hyp] init_guests: enter\n\r");

	uart_print("[hyp] init_guests: guest_start");
	uart_print_hex32( (unsigned int) &guest_start); uart_print("\n\r");

	// Guest 1 @guest_start
	context = &guest_contexts[0];
	context->pc = (unsigned int) &guest_start;
	context->cpsr 	= 0;	// uninitialized
	context->sp	= 0;	// uninitialized
	context->lr	= 0;	// uninitialized
	// context->gpr[] = whatever

	uart_print("[hyp] init_guests: guest2_start");
	uart_print_hex32( (unsigned int) &guest2_start); uart_print("\n\r");

	// Guest 2 @guest2_bin_start
	context = &guest_contexts[1];
	context->pc = (unsigned int) &guest2_start;
	context->cpsr 	= 0;	// uninitialized
	context->sp	= 0;	// uninitialized
	context->lr	= 0;	// uninitialized
	// context->gpr[] = whatever


	//__mon_install_guest();
	uart_print("[hyp] init_guests: return\n\r");
}

void hyp_switch_to_next_guest(struct arch_regs *regs_current)
{
	struct arch_regs *context = 0;
	int i;
	// store
	context = &guest_contexts[current_guest];
	context->pc = regs_current->pc;
	context->cpsr = regs_current->cpsr;
	context->sp = regs_current->sp;
	context->lr = regs_current->lr;
	for( i = 0; i < ARCH_REGS_NUM_GPR; i++) {
		context->gpr[i] = regs_current->gpr[i];
	}

	// load the next guest
	current_guest = (current_guest + 1) % NUM_GUEST_CONTEXTS;
	context = &guest_contexts[current_guest];
	__mon_switch_to_guest_context( context );
}

void hyp_switch_to_initial_guest(void)
{
	struct arch_regs *context = 0;

	current_guest = 0;
	context = &guest_contexts[current_guest];

	uart_print("[hyp] switch_to_initial_guest:\n\r");
	uart_dump_regs( context );
	__mon_switch_to_guest_context( context );
}

void hyp_switch_guest(void)
{
	uart_print("[hyp] switch_guest: enter, not coming back\n\r");
	hyp_switch_to_initial_guest();
}

void hyp_main(void)
{
	uart_print("[hyp_main] Starting...\n\r");
	hyp_init_guests();

	hyp_switch_guest();

	uart_print("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n\r");
}

