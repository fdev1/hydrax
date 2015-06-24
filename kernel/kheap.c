/*
 * kheap.c
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
#include <kheap.h>
#include <mmu.h>
#include <printk.h>
#include "bget/bget.h"

#define KHEAP_DEBUG		0x0
#define KHEAP_START         	0xC0000000
#define KHEAP_INITIAL_SIZE  	0x200000

/*
 *
 */
kheap_t kheap = { 0, 0 };

/*
 * initialize kernel heap
 */
void kheap_init(void)
{

	/* TODO: For better readability lets allocate the heap buffer
	 * with the KALLOC_OPTN_KERNEL flag
	 */

	kheap.buffer = (void*) kalloc(KHEAP_INITIAL_SIZE, 
		NULL, NULL, KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL);
	assert(kheap.buffer != NULL);
	kheap.len = KHEAP_INITIAL_SIZE;

	/* mmu_dump_page((uint32_t) kheap.buffer); */
	bpool(kheap.buffer, kheap.len);
	printk(8, "kheap: allocated %i MB heap at 0x%x-0x%x", 
		kheap.len / 1024 / 1024,
		(intptr_t) kheap.buffer,
		((intptr_t) kheap.buffer) + kheap.len );
}

/*
 * Allocate a buffer on the kernel heap.
 */
void *malloc(uint32_t sz)
{
	if (KHEAP_DEBUG)
		printk(8, "kmalloc(%i)", sz);

	if (sz == NULL)
		return NULL;
	return bget(sz);
}

/* 
 * Free a buffer.
 */
void free(void *ptr)
{
	if (ptr == NULL)	/* this should not be necessary */
		return;

	if (KHEAP_DEBUG)
		printk(8, "kfree(%x)", ptr);

	brel(ptr);
}


