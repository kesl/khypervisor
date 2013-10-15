#ifndef __IO_EMUL_H_
#define __IO_EMUL_H_

#define	writel(a, v)	(*(volatile unsigned int *)(a) = (v))
#define	readl(a)        (*(volatile unsigned int *)(a))

#endif
