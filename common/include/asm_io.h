#ifndef __ASM_IO_H__
#define __ASM_IO_H__

#define putw(v, a)   (*(volatile unsigned short *)(a) = (v))
#define putb(v, a)   (*(volatile unsigned char *)(a) = (v))
#define getw(a)      (*(volatile unsigned short *)(a))
#define getb(a)      (*(volatile unsigned char *)(a))
#define putl(v, a)   (*(volatile unsigned int *)(a) = (v))
#define getl(a)      (*(volatile unsigned int *)(a))
#define writel(v, a) ({ uint32_t vl = v; putl(vl, a); })
#define readl(a)     ({ uint32_t vl = getl(a); vl; })
#define writew(v, a) ({ uint16_t vl = v; putw(vl, a); })
#define readw(a)     ({ uint16_t vl = getw(a); vl; })
#define writeb(v, a) ({ uint8_t vl = v; putb(vl, a); })
#define readb(a)     ({ uint8_t vl = getb(a); vl; })
#endif
