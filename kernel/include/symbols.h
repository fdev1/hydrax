/*
 * include/symbols.h
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

#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#include <arch/arch.h>

void symbols_init(void);
char* getsym(uint32_t addr);


#endif
