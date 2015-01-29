/*
 * include/memory.h
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

#ifndef __MEMORY_H__
#define __MEMORY_H__

void memcpy(void *dest, const void *src, uint32_t len);
void memset(void *dest, uint8_t val, uint32_t len);

#endif
