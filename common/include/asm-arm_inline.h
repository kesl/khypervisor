#ifndef __ASM_ARM_INLINE__
#define __ASM_ARM_INLINE__

#define isb() asm volatile("isb" : : : "memory")
#define irq_enable() asm volatile("cpsie i" : : : "memory")

#endif
