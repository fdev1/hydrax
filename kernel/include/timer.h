/*
 * include/timer.h
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

#ifndef __TIMER_H__
#define __TIMER_H__

#include <arch/arch.h>

typedef uint32_t tticks_t;

/**
 * Initialize system timer.
 */
void timer_init(uint32_t);

/*
 * Get timer tick count.
 */
uint32_t timer_getticks(void);

#endif
