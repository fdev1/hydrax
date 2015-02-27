/*
 * include/ucontext.h
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

#ifndef __UCONTEXT_H__
#define __UCONTEXT_H__

#include <stdint.h>
#include <arch/scheduler.h>

typedef task_state_t mcontext_t;

typedef struct __ucontext
{
	struct __ucontext *uc_link;
	sigset_t uc_sigmask;
	struct __stack uc_stack;
	mcontext_t uc_mcontext;	
}
ucontext_t;

int  getcontext(ucontext_t *);
void makecontext(ucontext_t *, void (*)(void), int, ...);
int  setcontext(const ucontext_t *);
int  swapcontext(ucontext_t *, const ucontext_t *);

#endif
