/*
 * include/isr.h
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

#ifndef __ISR_H__
#define __ISR_H__

#include <arch/platform.h>
#include <arch/isr.h>

/*
 * Register interrupt handler
 */
void register_isr(unsigned int n, isr_t isr);

#endif

