/*
 * isr.h
 * 
 * Author: Fernando Rodriguez
 * Email: frodriguez.developer@outlook.com
 * 
 * Copyright 2014-2015 Fernando Rodriguez
 * All rights reserved.
 * 
 * This code is distributed for educational purposes
 * only. It may not be distributed without written 
 * permission from the author.
 *
 */

#ifndef __ARCH_ISR_H__
#define __ARCH_ISR_H__

struct __registers;

#include <stdint.h>

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47


/*
 * ISR handler function type;
 */
typedef void (*isr_t)(struct __registers*);

/*
 * clear interrupt flag
 */
#if defined(KERNEL_CODE)
static inline void arch_clear_interrupt(struct __registers *regs)
{
        if (regs->int_no >= 40)
		outb(0xA0, 0x20);
	outb(0x20, 0x20);
}
#endif

#endif

