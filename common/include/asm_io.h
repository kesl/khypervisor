#ifndef __ASM_IO_H__
#define __ASM_IO_H__

/* IO */

/* byte, 8 */
#define putb(v, a)   (*(volatile unsigned char *)(a) = (v))
#define getb(a)      (*(volatile unsigned char *)(a))

/* word, 16 */
#define putw(v, a)   (*(volatile unsigned short *)(a) = (v))
#define getw(a)      (*(volatile unsigned short *)(a))

/* long, 32 */
#define putl(v, a)   (*(volatile unsigned int *)(a) = (v))
#define getl(a)      (*(volatile unsigned int *)(a))

/* quadword, 64 */
#define putq(v, a)   (*(volatile unsigned long long *)(a) = (v))
#define getq(a)      (*(volatile unsigned long long *)(a))

/* Wrapper */

/* byte, 8 */
#define writeb(v, a) ({ uint8_t vl = v; putb(vl, a); })
#define readb(a)     ({ uint8_t vl = getb(a); vl; })

/* word, 16 */
#define writew(v, a) ({ uint16_t vl = v; putw(vl, a); })
#define readw(a)     ({ uint16_t vl = getw(a); vl; })

/* long, 32 */
#define writel(v, a) ({ uint32_t vl = v; putl(vl, a); })
#define readl(a)     ({ uint32_t vl = getl(a); vl; })

/* quadword, 64 */
#define writeq(v, a) ({ uint64_t vl = v; putq(vl, a); })
#define readq(a)     ({ uint64_t vl = getq(a); vl; })

#endif
