/*
 * include/kheap.h
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

#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <arch/platform.h>

typedef struct __kheap
{
	void *buffer;
	uint32_t len;
}
kheap_t;

/*
 * initialize kernel heap
 */
void kheap_init(void);

void *malloc(uint32_t);

void free(void*);

#endif

