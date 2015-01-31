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
 * 
 */

#include <arch/arch.h>
#include <timer.h>

/**
 *
 */
void arch_timer_init(uint32_t frequency)
{
	uint32_t divisor = 1193180 / frequency;
	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

	outb(0x43, 0x36);
	outb(0x40, l);
	outb(0x40, h);
} 

