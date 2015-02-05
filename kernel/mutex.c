/*
 * mutex.c
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
#include <mutex.h>
#include <scheduler.h>
#include <kheap.h>
#include <unistd.h>

/*
 * Put the current task to sleep until the mutex
 * is acquired.
 */
void mutex_waitsleep(mutex_t *s)
{
	register int i = 1;
	arch_exchange_rm(i, s->s);
	while (i)
	{
		mutex_waiter_t waiter;
		mutex_attach_waiter(&s->waiters, &s->waiters_lock, &waiter);
		while (waiter.waiting)
			pause();
		mutex_detach_waiter(&s->waiters, &s->waiters_lock, &waiter);
		arch_exchange_rm(i, s->s);
	}
}

/*
 * Release a mutex and signal any sleeping waiters.
 */
void mutex_releasesleep(mutex_t *m)
{
	register int i = 0;
	arch_exchange_rm(i, m->s);
	assert(i == 1);
	if (likely(m->waiters != NULL))
	{
		m->waiters->waiting = 0;
		killtask(m->waiters->pid, SIGCONT);
	}
}


