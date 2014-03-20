#ifndef __ASM_IO_H__
#define __ASM_IO_H__
#define dsb() asm volatile("dsb" : : : "memory")
/* Write Memory barrier */
#define iormb()          dsb()
/* Read Memory barrier */
#define iowmb()          dsb()
#define putl(v, a)   (*(volatile unsigned int *)(a) = (v))
#define getl(a)      (*(volatile unsigned int *)(a))
#define writel(v, a) ({ uint32_t vl = v; iowmb(); putl(vl, a); vl; })
#define readl(a)     ({ uint32_t vl = getl(a); iormb(); vl; })
#endif
