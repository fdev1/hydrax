/*
 * include/arch/x86/assert.h
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

#ifndef __ASSERT_H__
#define __ASSERT_H__

#if defined(KERNEL_CODE)
#	if defined(NDEBUG)
#		define assert(expr)			(void) 0
#	else
#		define assert(expr) 		\
			if (unlikely(!(expr)))		\
				arch_panic("assert failed (" #expr ")", __FILE__, __LINE__)
#	endif
#else
#	if defined(NDEBUG)
#		define assert(expr)			(void) 0
#	else
#		define assert(expr)			exit(-1)
#	endif
#endif

#endif
