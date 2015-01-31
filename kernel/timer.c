/*
 * timer.c
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
#include <arch/timer.h>
#include <timer.h>
#include <isr.h>
#include <printk.h>
#include <scheduler.h>

/*
 * private
 */
static uint32_t tick = 0;

/**
 *
 */
/*static*/ void timer_callback(registers_t *regs)
{
	tick++;
	schedule();
}

/*
 *
 */
uint32_t timer_getticks(void)
{
	return tick;
}

/**
 *
 */
void timer_init(uint32_t frequency)
{
	tick = 0;
	register_isr(IRQ0, &timer_callback);
	arch_timer_init(frequency);
	printk(7, "timer_init: timer running");
} 

