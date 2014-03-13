#ifndef __ASM_IO_H__
#define __ASM_IO_H__

#define dsb() asm volatile("dsb" : : : "memory")
#define __arch_putl(v,a)        (*(volatile unsigned int *)(a) = (v))
#define __arch_getl(a)          (*(volatile unsigned int *)(a))
/* Write Memory barrier */
#define __iormb()          dsb()
/* Read Memory barrier */
#define __iowmb()          dsb()
#define writel(v,c) ({ uint32_t __v = v; __iowmb(); __arch_putl(__v,c); __v; })
#define readl(c)    ({ uint32_t __v = __arch_getl(c); __iormb(); __v; })
#endif
