/*
 * include/printk.h
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

#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <arch/platform.h>

void kprintf(const char *fmt, ...);

/*
 * Writes a string to the screen if the specified
 * log level is greater than the current log level.
 * It will also write it to an in-memory log once I
 * get the timme to implement it.
 */
void printk(const unsigned int level, const char *fmt, ...);

/*
 * Causes a delay in-between printks 
 */
void printk_delay(const uint32_t delay);

#endif

