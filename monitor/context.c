#include <armv7_p15.h>
#include <uart_print.h>
#include <hvmm_trace.h>
#include "context.h"
#include "trap.h"
#include "vmm.h"
#include <print.h>

#define CPSR_MODE_USER  0x10
#define CPSR_MODE_FIQ   0x11
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SVC   0x13
#define CPSR_MODE_MON   0x16
#define CPSR_MODE_ABT   0x17
#define CPSR_MODE_HYP   0x1A
#define CPSR_MODE_UND   0x1B
#define CPSR_MODE_SYS   0x1F

#define __CONTEXT_TRACE_VERBOSE__
#define _valid_vmid(vmid)   ( context_first_vmid() <= vmid && context_last_vmid() >= vmid )

extern void __mon_switch_to_guest_context( struct arch_regs *regs );

static struct hyp_guest_context guest_contexts[NUM_GUEST_CONTEXTS];
static int _current_guest_vmid = VMID_INVALID;
static int _next_guest_vmid = VMID_INVALID;

// uboot test start
/* list of possible tags */
#define ATAG_NONE       0x00000000
#define ATAG_CORE       0x54410001
#define ATAG_MEM        0x54410002
#define ATAG_VIDEOTEXT  0x54410003
#define ATAG_RAMDISK    0x54410004
#define ATAG_INITRD2    0x54420005
#define ATAG_SERIAL     0x54410006
#define ATAG_REVISION   0x54410007
#define ATAG_VIDEOLFB   0x54410008
#define ATAG_CMDLINE    0x54410009
/* structures for each atag */
#define u32 uint32_t
typedef unsigned char u8;
typedef unsigned short u16;

struct atag_header {
        u32 size; /* length of tag in words including this header */
        u32 tag;  /* tag type */
};
struct atag_core {
        u32 flags;
        u32 pagesize;
        u32 rootdev;
};

struct atag_mem {
        u32     size;
        u32     start;
};
struct atag_serialnr {
        u32 low;
        u32 high;
};

struct atag_revision {
        u32 rev;
};
struct atag_cmdline {
        char    cmdline[1];
};
struct atag_ramdisk {
        u32 flags;      /* bit 0 = load, bit 1 = prompt */
        u32 size;       /* decompressed ramdisk size in _kilo_ bytes */
        u32 start;      /* starting block of floppy-based RAM disk image */
};
struct atag_videotext {
        u8              x;           /* width of display */
        u8              y;           /* height of display */
        u16             video_page;
        u8              video_mode;
        u8              video_cols;
        u16             video_ega_bx;
        u8              video_lines;
        u8              video_isvga;
        u16             video_points;
};
struct atag_initrd2 {
        u32 start;      /* physical start address */
        u32 size;       /* size of compressed ramdisk image in bytes */
};
struct atag_videolfb {
        u16             lfb_width;
        u16             lfb_height;
        u16             lfb_depth;
        u16             lfb_linelength;
        u32             lfb_base;
        u32             lfb_size;
        u8              red_size;
        u8              red_pos;
        u8              green_size;
        u8              green_pos;
        u8              blue_size;
        u8              blue_pos;
        u8              rsvd_size;
        u8              rsvd_pos;
};
struct atag {
        struct atag_header hdr;
        union {
                struct atag_core         core;
                struct atag_mem          mem;
                struct atag_videotext    videotext;
                struct atag_ramdisk      ramdisk;
                struct atag_initrd2      initrd2;
                struct atag_serialnr     serialnr;
                struct atag_revision     revision;
                struct atag_videolfb     videolfb;
                struct atag_cmdline      cmdline;
        } u;
};
#define tag_next(t)     ((struct tag *)((u32 *)(t) + (t)->hdr.size))
#define tag_size(type)  ((sizeof(struct atag_header) + sizeof(struct type)) >> 2)
static struct atag *params; /* used to point at the current tag */

static void
setup_core_tag(void * address,long pagesize)
{
    params = (struct atag *)address;         /* Initialise parameters to start at given address */

    params->hdr.tag = ATAG_CORE;            /* start with the core tag */
    params->hdr.size = tag_size(atag_core); /* size the tag */
    params->u.core.flags = 1;               /* ensure read-only */
    params->u.core.pagesize = pagesize;     /* systems pagesize (4k) */
    params->u.core.rootdev = 0;             /* zero root device (typicaly overidden from commandline )*/

    params = tag_next(params);              /* move pointer to next tag */
}

static void setup_revision_tag(void)
{
    params->hdr.tag = ATAG_REVISION;
    params->hdr.size = tag_size (atag_revision);
    params->u.revision.rev = 0xcfdfdfdf;
    params = tag_next (params);
}

char *strcpy(char *dest, const char *src)
{
    uint32_t i;
    for (i = 0; src[i] != '\0'; ++i)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

static void
setup_cmdline_tag(const char * line)
{
    int linelen = strlen(line);

    if(!linelen)
        return;                             /* do not insert a tag for an empty commandline */

    params->hdr.tag = ATAG_CMDLINE;         /* Commandline tag */
    params->hdr.size = (sizeof(struct atag_header) + linelen + 1 + 4) >> 2;

    strcpy(params->u.cmdline.cmdline,line); /* place commandline into tag */

    params = tag_next(params);              /* move pointer to next tag */
}

static void
setup_mem_tag(u32 start, u32 len)
{
    params->hdr.tag = ATAG_MEM;             /* Memory tag */
    params->hdr.size = tag_size(atag_mem);  /* size tag */

    params->u.mem.start = start;            /* Start of memory area (physical address) */
    params->u.mem.size = len;               /* Length of area */

    params = tag_next(params);              /* move pointer to next tag */
}

static void
setup_end_tag(void)
{
    params->hdr.tag = ATAG_NONE;            /* Empty tag ends list */
    params->hdr.size = 0;                   /* zero length */
}

// uboot test end

#ifdef BAREMETAL_GUEST
static void _hyp_fixup_unloaded_guest(void)
{
	extern uint32_t guest_bin_start;
	extern uint32_t guest_bin_end;
	extern uint32_t guest2_bin_start;

	uint32_t *src = &guest_bin_start;
    uint32_t *end = &guest_bin_end;
//	uint32_t *dst = &guest2_bin_start;
	uint32_t *dst = &guest_bin_start + (0x20008000/4);

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

static char *_modename(uint8_t mode)
{
    char *name = "Unknown";
    switch(mode) {
        case CPSR_MODE_USER:
            name = "User";
            break;
        case CPSR_MODE_FIQ:
            name = "FIQ";
            break;
        case CPSR_MODE_IRQ:
            name = "IRQ";
            break;
        case CPSR_MODE_SVC:
            name = "Supervisor";
            break;
        case CPSR_MODE_MON:
            name = "Monitor";
            break;
        case CPSR_MODE_ABT:
            name = "Abort";
            break;
        case CPSR_MODE_HYP:
            name = "Hyp";
            break;
        case CPSR_MODE_UND:
            name = "Undefined";
            break;
        case CPSR_MODE_SYS:
            name = "System";
            break;
    }
    return name;
}

void context_dump_regs( struct arch_regs *regs )
{
    uart_print( "cpsr:" ); uart_print_hex32( regs->cpsr ); uart_print( "\n\r" );
    uart_print( "  pc:" ); uart_print_hex32( regs->pc ); uart_print( "\n\r" );

#ifdef __CONTEXT_TRACE_VERBOSE__
    {
        int i;
        uart_print( " gpr:\n\r" );
	    for( i = 0; i < ARCH_REGS_NUM_GPR; i++) {
            uart_print( "     " ); uart_print_hex32( regs->gpr[i] ); uart_print( "\n\r" );
	    }
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

/* banked registers */

void context_init_banked(struct arch_regs_banked *regs_banked)
{
    regs_banked->spsr_usr = 0;
    regs_banked->sp_usr = 0;
    regs_banked->lr_usr = 0;
    regs_banked->spsr_svc = 0;
    regs_banked->sp_svc = 0;
    regs_banked->lr_svc = 0;
    regs_banked->spsr_abt = 0;
    regs_banked->sp_abt = 0;
    regs_banked->lr_abt = 0;
    regs_banked->spsr_und = 0;
    regs_banked->sp_und = 0;
    regs_banked->lr_und = 0;
    regs_banked->spsr_irq = 0;
    regs_banked->sp_irq = 0;
    regs_banked->lr_irq = 0;
    regs_banked->spsr_fiq = 0;
    regs_banked->lr_fiq = 0;
    regs_banked->r8_fiq = 0;
    regs_banked->r9_fiq = 0;
    regs_banked->r10_fiq = 0;
    regs_banked->r11_fiq = 0;
    regs_banked->r12_fiq = 0;
	//Cortex-A15 processor does not support sp_fiq
}

void context_save_banked(struct arch_regs_banked *regs_banked)
{
	/* SVC banked register */
    asm volatile (" mrs     %0, spsr_svc\n\t"
                          :"=r" (regs_banked->spsr_svc)::"memory", "cc");
    asm volatile (" mrs     %0, sp_svc\n\t"
                          :"=r" (regs_banked->sp_svc)::"memory", "cc");
    asm volatile (" mrs     %0, lr_svc\n\t"
                          :"=r" (regs_banked->lr_svc)::"memory", "cc");

	/* ABT banked register */
    asm volatile (" mrs     %0, spsr_abt\n\t"
                          :"=r" (regs_banked->spsr_abt)::"memory", "cc");
    asm volatile (" mrs     %0, sp_abt\n\t"
                          :"=r" (regs_banked->sp_abt)::"memory", "cc");
    asm volatile (" mrs     %0, lr_abt\n\t"
                          :"=r" (regs_banked->lr_abt)::"memory", "cc");

	/* UND banked register */
    asm volatile (" mrs     %0, spsr_und\n\t"
                          :"=r" (regs_banked->spsr_und)::"memory", "cc");
    asm volatile (" mrs     %0, sp_und\n\t"
                          :"=r" (regs_banked->sp_und)::"memory", "cc");
    asm volatile (" mrs     %0, lr_und\n\t"
                          :"=r" (regs_banked->lr_und)::"memory", "cc");

    /* IRQ banked register */
    asm volatile (" mrs     %0, spsr_irq\n\t"
                          :"=r" (regs_banked->spsr_irq)::"memory", "cc");
    asm volatile (" mrs     %0, sp_irq\n\t"
                          :"=r" (regs_banked->sp_irq)::"memory", "cc");
    asm volatile (" mrs     %0, lr_irq\n\t"
                          :"=r" (regs_banked->lr_irq)::"memory", "cc");

	/* FIQ banked register  R8_fiq ~ R12_fiq, LR and SPSR */
    asm volatile (" mrs     %0, spsr_fiq\n\t"
                          :"=r" (regs_banked->spsr_fiq)::"memory", "cc");
    asm volatile (" mrs     %0, lr_fiq\n\t"
                          :"=r" (regs_banked->lr_fiq)::"memory", "cc");
    asm volatile (" mrs     %0, r8_fiq\n\t"
                          :"=r" (regs_banked->r8_fiq)::"memory", "cc");
    asm volatile (" mrs     %0, r9_fiq\n\t"
                          :"=r" (regs_banked->r9_fiq)::"memory", "cc");
    asm volatile (" mrs     %0, r10_fiq\n\t"
                          :"=r" (regs_banked->r10_fiq)::"memory", "cc");
    asm volatile (" mrs     %0, r11_fiq\n\t"
                          :"=r" (regs_banked->r11_fiq)::"memory", "cc");
    asm volatile (" mrs     %0, r12_fiq\n\t"
                          :"=r" (regs_banked->r12_fiq)::"memory", "cc");

}

void context_restore_banked(struct arch_regs_banked *regs_banked)
{
	/* SVC banked register */
    asm volatile (" msr    spsr_svc, %0\n\t"
                          ::"r" (regs_banked->spsr_svc) :"memory", "cc");
    asm volatile (" msr    sp_svc, %0\n\t"
                          ::"r" (regs_banked->sp_svc) :"memory", "cc");
    asm volatile (" msr    lr_svc, %0\n\t"
                          ::"r" (regs_banked->lr_svc) :"memory", "cc");

	/* ABT banked register */
    asm volatile (" msr    spsr_abt, %0\n\t"
                          ::"r" (regs_banked->spsr_abt) :"memory", "cc");
    asm volatile (" msr    sp_abt, %0\n\t"
                          ::"r" (regs_banked->sp_abt) :"memory", "cc");
    asm volatile (" msr    lr_abt, %0\n\t"
                          ::"r" (regs_banked->lr_abt) :"memory", "cc");

	/* UND banked register */
    asm volatile (" msr    spsr_und, %0\n\t"
                          ::"r" (regs_banked->spsr_und) :"memory", "cc");
    asm volatile (" msr    sp_und, %0\n\t"
                          ::"r" (regs_banked->sp_und) :"memory", "cc");
    asm volatile (" msr    lr_und, %0\n\t"
                          ::"r" (regs_banked->lr_und) :"memory", "cc");

	/* IRQ banked register */
    asm volatile (" msr     spsr_irq, %0\n\t"
                          ::"r" (regs_banked->spsr_irq) :"memory", "cc");
    asm volatile (" msr     sp_irq, %0\n\t"
                          ::"r" (regs_banked->sp_irq) :"memory", "cc");
    asm volatile (" msr     lr_irq, %0\n\t"
                          ::"r" (regs_banked->lr_irq) :"memory", "cc");

	/* FIQ banked register */
    asm volatile (" msr     spsr_fiq, %0\n\t"
                          ::"r" (regs_banked->spsr_fiq) :"memory", "cc");
    asm volatile (" msr     lr_fiq, %0\n\t"
                          ::"r" (regs_banked->lr_fiq) :"memory", "cc");
    asm volatile (" msr    r8_fiq, %0\n\t"
                          ::"r" (regs_banked->r8_fiq) :"memory", "cc");
    asm volatile (" msr    r9_fiq, %0\n\t"
                          ::"r" (regs_banked->r9_fiq) :"memory", "cc");
    asm volatile (" msr    r10_fiq, %0\n\t"
                          ::"r" (regs_banked->r10_fiq) :"memory", "cc");
    asm volatile (" msr    r11_fiq, %0\n\t"
                          ::"r" (regs_banked->r11_fiq) :"memory", "cc");
    asm volatile (" msr    r12_fiq, %0\n\t"
                          ::"r" (regs_banked->r12_fiq) :"memory", "cc");
}

/* Co-processor state management: init/save/restore */
void context_init_cops(struct arch_regs_cop *regs_cop)
{
    regs_cop->vbar = 0;
    regs_cop->ttbr0 = 0;
    regs_cop->ttbr1 = 0;
    regs_cop->ttbcr = 0;
}

void context_save_cops(struct arch_regs_cop *regs_cop)
{
    regs_cop->vbar = read_vbar();
    regs_cop->ttbr0 = read_ttbr0();
    regs_cop->ttbr1 = read_ttbr1();
    regs_cop->ttbcr = read_ttbcr();
}

void context_restore_cops(struct arch_regs_cop *regs_cop)
{
    write_vbar(regs_cop->vbar);
	write_ttbr0(regs_cop->ttbr0);
	write_ttbr1(regs_cop->ttbr1);
	write_ttbcr(regs_cop->ttbcr);
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
	vmm_stage2_enable(0);

	if ( regs_current != 0 ) {
		/* save the current guest's context */
		context = &guest_contexts[_current_guest_vmid];
		regs = &context->regs;
		context_copy_regs( regs, regs_current );
        context_save_cops( &context->regs_cop );
        context_save_banked( &context->regs_banked );
        vgic_save_status( &context->vgic_status );
        printh( "context: saving vmid[%d] mode(%x):%s pc:0x%x\n", 
                _current_guest_vmid, 
                regs->cpsr & 0x1F, 
                _modename(regs->cpsr & 0x1F),
                regs->pc
       );
	}

	/* The context of the next guest */
	context = &guest_contexts[next_vmid];

	/* Restore Translation Table for the next guest and Enable Stage 2 Translation */
	vmm_set_vmid_ttbl( context->vmid, context->ttbl );
	vmm_stage2_enable(1);
    vgic_restore_status( &context->vgic_status );
    
    printh( "context: restoring vmid[%d] mode(%x):%s pc:0x%x\n", 
            next_vmid, 
            context->regs.cpsr & 0x1F, 
            _modename(context->regs.cpsr & 0x1F),
            context->regs.pc
   );

    /* The next becomes the current */
    _current_guest_vmid = next_vmid;
	if ( regs_current == 0 ) {
		/* init -> hyp mode -> guest */
		/* The actual context switching (Hyp to Normal mode) handled in the asm code */
		__mon_switch_to_guest_context( &context->regs );
	} else {
		/* guest -> hyp -> guest */
		context_copy_regs( regs_current, &context->regs );
        context_restore_cops( &context->regs_cop );
        context_restore_banked( &context->regs_banked );
	}

    result = HVMM_STATUS_SUCCESS;
    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t context_perform_switch(void)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;

    if ( _current_guest_vmid == VMID_INVALID ) {
        printh("context: launching the first guest\n");
        /* very first time, to the default first guest */
        result = context_perform_switch_to_guest_regs( 0, _next_guest_vmid );
        /* DOES NOT COME BACK HERE */
    } else if ( _next_guest_vmid != VMID_INVALID && _current_guest_vmid != _next_guest_vmid ) {
        struct arch_regs *regs = trap_saved_regs();
        if ( (regs->cpsr & 0x1F) != 0x1A ) {
            uart_print( "curr:" ); uart_print_hex32( _current_guest_vmid ); uart_print( "\n\r" );
            uart_print( "next:" ); uart_print_hex32( _next_guest_vmid ); uart_print( "\n\r" );

            /* Only if not from Hyp */
            result = context_perform_switch_to_guest_regs( regs, _next_guest_vmid );
            _next_guest_vmid = VMID_INVALID;
        }
    }
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

static void setup_tags(uint32_t *src)
{
    char *commandline = "root=/dev/ram console=ttyAMA0, 38400n8 mem=768M mtdparts=armflash:1M@0x800000(uboot),7M@0x1000000(kernel),24M@0x2000000(initrd) mmci.fmax=190000 devtmpfs.mount=0 vmalloc=256M rdinit=/sbin/init";
    setup_core_tag(src+(0x100/4), 4096);       /* standard core tag 4k pagesize */
    setup_cmdline_tag(commandline);    /* commandline setting root device */
    setup_revision_tag();
    //setup_mem_tag(0x80000000, 0x2000000); 
    //setup_mem_tag(0xa0000000, 0x2000000); 
    setup_mem_tag(src, 0x20000000); 
    setup_mem_tag(src+0x20000000, 0x10000000); 
    setup_end_tag();                    /* end of tags */
}

void context_init_guests(void)
{
	struct hyp_guest_context *context;
	struct arch_regs *regs = 0;

	extern uint32_t guest_bin_start;
    uint32_t *src = &guest_bin_start;
	
	uart_print("[hyp] init_guests: enter\n\r");


	/* Guest 1 @guest_bin_start */
	context = &guest_contexts[0];
	regs = &context->regs;
//	regs->pc = 0x80000000;	// PA:0xA0000000
	regs->pc = 0xA0008000;	// PA:0xA0000000
	regs->cpsr = 0x1d3;	    // supervisor, interrupt disabled
//    regs->gpr[1] = (read_midr() >> 16) & 0xF;
    regs->gpr[1] = 2272;  //vexpress
    /* */
    regs->gpr[2] = 0x80000100;//src+(0x100/4);
    

	/* regs->gpr[] = whatever */
	context->vmid = 0;
	context->ttbl = vmm_vmid_ttbl(context->vmid);
    context_init_cops( &context->regs_cop );
    context_init_banked( &context->regs_banked );

	/* Guest 2 @guest2_bin_start */
	context = &guest_contexts[1];
	regs = &context->regs;
	regs->pc = 0x80000000;	// PA: 0xB0000000
	regs->cpsr = 0x1d3;	// supervisor, interrupt disabled

	/* regs->gpr[] = whatever */
	context->vmid = 1;
	context->ttbl = vmm_vmid_ttbl(context->vmid);
    context_init_cops( &context->regs_cop );
    context_init_banked( &context->regs_banked );

#ifdef BAREMETAL_GUEST
	/* Workaround for unloaded bmguest.bin at 0xB0000000@PA */
	_hyp_fixup_unloaded_guest();
#endif
    setup_tags(src);
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
