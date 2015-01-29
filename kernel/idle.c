/*
 * idle.c
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
#include <arch/platform.h>
#include <idle.h>
#include <printk.h>

/*
 * This is the idle loop. It simply puts the cpu
 * to sleep until an external event occurs.
 */
void idle(void)
{
	while (1)
		arch_sleep();
}

