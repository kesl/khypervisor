#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <asm-arm/ptrace.h>

struct irqaction {
	void  (*handler)();
	unsigned long flags;
	const char *name;
	struct irqaction *next;
	int irq;
	void *dev_id;
};

struct irq_desc {
	struct irqaction	*action;	/* IRQ action list */
	unsigned int		status;		/* IRQ status */
	unsigned int		depth;		/* nested irq disables */
};

extern struct irq_desc irq_desc[34];

extern void unhand();
void init_IRQ();
int request_irq(unsigned int irq, void (*handler)(void), 
		unsigned long irqflags, const char *devname, void *dev_id);
void free_irq(unsigned int irq, void *dev_id);


#endif /* _INTERRUPT_H_ */

