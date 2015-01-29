/*
 * include/arch/x86/arch.h
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

#ifndef __ARCH_H__
#define __ARCH_H__

#define ARCH_STACK_START				(0xDFFFFFFC)
#define ARCH_STACK_INITIAL_SIZE		(0x8000)

#define MAX_FILENAME				(255)
#define MAX_PATH					(255)
#define MAX_PATH_NODES				(64)

/*
 * fixed with integers
 */
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef char int8_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;


typedef int size_t;
typedef int pid_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef unsigned int time_t;
typedef int bool;

/*
 * pointer sized datatypes
 */
typedef unsigned int intptr_t;
typedef unsigned int pintptr_t;

#if defined(KERNEL_CODE)
/*
 * port IO
 */
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

/*
 * Panic the system
 */
void arch_panic(const char *message, const char *file, uint32_t line);

/*
 * Reboot the system
 */
void reboot(void);
#endif


#define NULL 0
#define CHAR_BIT		(8)
/* #define static */

/* 
 * Macros to enable/disable interrupts
 */
#define true					(1)
#define false					(0)
#define arch_enable_interrupts()	asm volatile("sti")
#define arch_disable_interrupts()	asm volatile("cli")
#define arch_sleep()			asm volatile("hlt")
#define arch_break()			asm volatile("int $0x3")
#define arch_halt()				do { arch_sleep(); } while (1)
#define panic(msg) 				arch_panic(msg, __FILE__, __LINE__)


#define arch_atomic_increment32(var)	asm volatile("lock; inc (%0)" : : "r" (&var))
#define arch_atomic_decrement32(var)	asm volatile("lock; dec (%0)" : : "r" (&var))


#if 0
#define likely(x)      				(x)
#define unlikely(x)    				(x)
#else
#define likely(x)      				__builtin_expect(!!(x), 1)
#define unlikely(x)    				__builtin_expect(!!(x), 0)
#endif

#if defined(KERNEL_CODE)
#	ifndef __ASSERT_H__
#		if defined(NDEBUG)
#			define assert(expr)			(void) 0
#		else
#			define assert(expr) 		\
				if (unlikely(!(expr)))		\
					arch_panic("assert failed (" #expr ")", __FILE__, __LINE__)
#		endif
#	endif
#else
#	ifndef __ASSERT_H__
#		include <unistd.h>
#		if defined(NDEBUG)
#			define assert(expr)			(void) 0
#		else
#			define assert(expr)			exit(-1)
#		endif
#	endif
#endif

/*
 * Atomically exchange a register and memory location
 */
#define arch_exchange_rm(reg, mem)	asm volatile("xchg %0, %1" : "+r" (reg), "+m" (mem));

void arch_dump_stack(unsigned int len);
void arch_dump_stack_trace(void);

/*
 * Allocate stack memory.
 */
static inline void *alloca(size_t sz)
{
	unsigned char *p;
	if (unlikely(sz & 0x3))
	{
		sz &= 0xFFFFFFFC;
		sz += 0x4;
	}
	asm volatile(
		"mov %%esp, %0;"
		"sub %1, %%esp;" : 
		"=r" (p) : "r" (sz));
	return (void*) p;	
}

#endif

