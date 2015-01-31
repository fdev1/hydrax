/*
 * isr.c
 * 
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
 */

#include <arch/arch.h>
#include <isr.h>
#include <printk.h>
#include <unistd.h>

#define DEBUG_ISR 		(0)

/*
 * isr table
 */
static isr_t isr_table[256] = { 0 };

/*
 * Default ISR handler
 */
void isr_handler(registers_t regs)
{
	if (DEBUG_ISR)
	{

		const char *isr_desc[] =
		{
			/* 00 */ "Divide Error",
			/* 01 */ "Reserved",
			/* 02 */ "Non-maskable external interrupt",
			/* 03 */ "Breakpoint",
			/* 04 */ "Overflow",
			/* 05 */ "BOUND Range Exceeded",
			/* 06 */ "Invalid Op-Code",
			/* 07 */ "No FPU",
			/* 08 */ "Double Fault",
			/* 09 */ "FPU Segment Overrun",
			/* 10 */ "Invalid TSS",
			/* 11 */ "Segment Not Present",
			/* 12 */ "Stack-Segment Fault",
			/* 13 */ "General Protection",
			/* 14 */ "Page Fault",
			/* 15 */ "Reserved"
		};

		printk(7, "Received interrupt: %i (%s)", regs.int_no, isr_desc[regs.int_no]);
		printk(8, "isr_handler: pid: %i", getpid());
		printk(8, "isr_handler: eip: 0x%x", regs.eip);
	}

	assert(regs.int_no != 0x80);

	if (isr_table[regs.int_no])
	{
		isr_t handler = isr_table[regs.int_no];
		handler(&regs);
	}
}

/*
 * Default IRQ handler
 */
void irq_handler(registers_t regs)
{
	assert(regs.int_no != 0x80);
	arch_clear_interrupt(&regs);
	if (isr_table[regs.int_no] != 0)
		isr_table[regs.int_no](&regs);
} 

/*
 * Register an ISR
 */
void register_isr(unsigned int n, isr_t isr)
{
	isr_table[n] = isr;
} 

