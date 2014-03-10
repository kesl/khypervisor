#ifndef __ASM_IO_H__
#define __ASM_IO_H__
#define putl(v, a)   (*(volatile unsigned int *)(a) = (v))
#define getl(a)      (*(volatile unsigned int *)(a))
#define writel(v, a) ({ uint32_t vl = v; putl(vl, a); })
#define readl(a)     ({ uint32_t vl = getl(a); vl; })
#endif
