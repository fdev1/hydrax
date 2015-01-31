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

#include <sys/types.h>
#include <assert.h>

/*
 * Generic memcpy
 */
#if !defined(ARCH_HAS_MEMCPY)
static inline void memcpy(void *dest, const void *src, uint32_t len)
{
    const uint8_t *sp = (const uint8_t *)src;
    uint8_t *dp = (uint8_t *) dest;
    for(; len != 0; len--) 
	    *dp++ = *sp++;
}
#endif

/*
 * Generic memset
 */
#if !defined(ARCH_HAS_MEMSET)
static inline void memset(void *dest, uint8_t val, size_t len)
{
	uint8_t *p = (uint8_t *) dest;

	while ( len-- )
		*p++ = val;
}
#endif

/*
 * Gemeric memmove
 */
#if !defined(ARCH_HAS_MEMMOVE)
static inline void memmove(void *dest, const void *src, size_t len)
{
	assert(0);
}
#endif

#endif
