/*
 * cmos.c
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

#include <arch/platform.h>
#include <assert.h>

/*
 * read a byte from cmos
 */
unsigned char cmos_readb(unsigned char offset)
{
	unsigned char c;
	asm volatile(
		"mov %1, %%al;"
		"out %%al, $0x70;"
		"in $0x71, %%al;"
		"mov %%al, %0;"
		: "=r" (c)
		: "r" (offset));
	return c;
}

/*
 * reads the cmos. You must pass a 128-byte buffer as 
 * argument.
 */
void cmos_read(unsigned char* buf, unsigned int offset, unsigned int len)
{
	unsigned char i;
	assert(offset < 128);
	assert(len <= 128);
	assert((offset + len) < 128);
 
	for (i = offset; i < (offset + len); i++)
	{
		asm volatile(
			"cli;"
			"mov %0, %%al;"
			"out %%al, $0x70;"
			"in $0x75, %%al;"
			"sti;"
			"mov %%al, %1"
			: "=r" (i)
			: "r" (buf[i]));
	}
}

/*
 * writes to cmos. You must pass a 128-byte buffer as
 * argument
 */
void cmos_write(unsigned char* buf)
{
	unsigned char i;
 
	for(i = 0; i < 128; i++)
	{
		asm volatile(
			"cli;"
			"mov %0, %%al;"
			"out %%al, $0x70;"
			"mov %1, %%al;"
			"out %%al, $0x71;"
			"sti;"
			: : "r" (i), "r" (buf[i]));
	}
}

