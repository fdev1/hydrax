/*
 * semaphore.c
 * 
 * This is a generic mutex based semaphore implementation.
 * A more optimal platform specific may be provided.
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

#if defined(KERNEL_CODE)
#	undef KERNEL_CODE
#endif
#define KERNEL_CODE

#include <semaphore.h>
#include <scheduler.h>
#include <mutex.h>
#include <kheap.h>
#include <printk.h>
#include <unistd.h>

/*
 * Try to acquiere a semaphore slot.
 */
__attribute__((weak)) 
bool semaphore_try(semaphore_t *s)
{
	if (unlikely(!mutex_try(&s->exclusive)))
		return false;
	if (unlikely(!mutex_try((mutex_t*)&s->enter)))
	{
		mutex_release(&s->exclusive);
		return false;
	}
	if (likely(s->cnt < s->max_cnt))
	{
		s->cnt++;
		mutex_release((mutex_t*) &s->enter);
		mutex_release(&s->exclusive);
		return true;
	}
	mutex_release((mutex_t*) &s->enter);
	mutex_release(&s->exclusive);
	return false;
}

/*
 * Try to exclusively acquire a semaphore.
 * 
 * WARNING: This version gets low priority so it may
 * be difficult getting exclusive access to a busy semaphore
 * while using this function.
 * 
 */
__attribute__((weak)) 
bool semaphore_try_excl(semaphore_t *s)
{
	if (unlikely(!mutex_try(&s->exclusive)))
		return false;
	
	if (unlikely(!mutex_try((mutex_t*) &s->enter)))
	{
		mutex_release(&s->exclusive);
		return false;
	}
	if (likely(s->cnt == 0))
	{
		mutex_release((mutex_t*) &s->enter);
		return true;
	}
	mutex_release((mutex_t*) &s->enter);
	mutex_release(&s->exclusive);
	return false;
}

/*
 * Wait for a semaphore slot.
 */
__attribute__((weak)) 
void semaphore_wait(semaphore_t *s)
{
	mutex_wait(&s->exclusive);
	while (1)
	{
		mutex_busywait((mutex_t*) &s->enter);
		if (likely(s->cnt < s->max_cnt))
		{
			s->cnt++;
			mutex_release((mutex_t*) &s->enter);
			mutex_release(&s->exclusive);
			return;
		}
		mutex_release((mutex_t*) &s->enter);
		yield();
	}
}

/*
 * Wait for exclusive access to semaphore.
 */
__attribute__((weak))
void semaphore_wait_excl(semaphore_t *s)
{
	mutex_wait(&s->exclusive);
	while (1)
	{
		mutex_wait((mutex_t*) &s->enter);
		if (likely(s->cnt == 0))
		{
			mutex_release((mutex_t*) &s->enter);
			return;
		}
		mutex_release((mutex_t*) &s->enter);
		yield();
	}
}

/*
 * Spin the CPU while we wait for a semaphore slot.
 */
__attribute__((weak))
void semaphore_busywait(semaphore_t *s)
{
	mutex_wait(&s->exclusive);
	while (1)
	{
		mutex_busywait((mutex_t*) &s->enter);
		if (likely(s->cnt < s->max_cnt))
		{
			s->cnt++;
			mutex_release((mutex_t*) &s->enter);
			mutex_release(&s->exclusive);
			return;
		}
		mutex_release((mutex_t*) &s->enter);
	}
}

/*
 * Spin the CPU until we get exclusive access to the
 * semaphore.
 */
__attribute__((weak))
void semaphore_busywait_excl(semaphore_t *s)
{
	mutex_busywait(&s->exclusive);
	while (1)
	{
		mutex_busywait((mutex_t*) &s->enter);
		if (likely(s->cnt == 0))
		{
			mutex_release((mutex_t*) &s->enter);
			return;
		}
		mutex_release((mutex_t*) &s->enter);
	}
}

/*
 * Sleep until a semaphore slot is acquired.
 */
__attribute__((weak))
void semaphore_waitsleep(semaphore_t *s)
{
	int i = 0;
	mutex_waitsleep(&s->exclusive);
	while (1)
	{
		mutex_waiter_t waiter;
		mutex_wait((mutex_t*) &s->enter);
		if (likely(s->cnt < s->max_cnt))
		{
			s->cnt++;
			mutex_release((mutex_t*) &s->enter);
			mutex_release(&s->exclusive);
			return;
		}
		mutex_release((mutex_t*) &s->enter);
		mutex_attach_waiter(&s->waiters, &s->waiters_lock, &waiter);
		while (waiter.waiting)		
			pause();
		mutex_detach_waiter(&s->waiters, &s->waiters_lock, &waiter);
	}
}

/*
 * Sleep until we get exclusive access to the semaphore
 */
__attribute__((weak))
void semaphore_waitsleep_excl(semaphore_t *s)
{
	mutex_waitsleep(&s->exclusive);
	while (1)
	{
		mutex_waiter_t waiter;
		mutex_wait((mutex_t*) &s->enter);
		if (likely(s->cnt == 0))
		{
			mutex_release((mutex_t*) &s->enter);
			return;
		}
		mutex_release((mutex_t*) &s->enter);
		mutex_attach_waiter(&s->waiters, &s->waiters_lock, &waiter);
		while (waiter.waiting)		
			pause();
		mutex_detach_waiter(&s->waiters, &s->waiters_lock, &waiter);
	}
}

/*
 * Signal a semaphore and wakeup any sleeping waiters.
 * TODO: This needs to sleep if it can signal the semaphore.
 */
__attribute__((weak))
void semaphore_signal_sleep(semaphore_t *s)
{
	while (1)
	{
		mutex_wait((mutex_t*) &s->enter);
		if (likely(s->cnt > 0))
		{
			s->cnt--;
			if (likely(s->waiters != NULL))
			{
				s->waiters->waiting = 0;
				pthread_kill(s->waiters->pid, SIGCONT);
			}
			mutex_release((mutex_t*) &s->enter);
			return;
		}
		mutex_release((mutex_t*) &s->enter);
	}	
}


/*
 * Signal a semaphore. If the semaphore cannot
 * be signal it blocks until it can.
 */
__attribute__((weak))
void semaphore_signal(semaphore_t *s)
{
	while (1)
	{
		mutex_wait((mutex_t*) &s->enter);
		if (likely(s->cnt > 0))
		{
			s->cnt--;
			mutex_release((mutex_t*) &s->enter);
			return;
		}
		mutex_release((mutex_t*) &s->enter);
	}
}

/*
 * Signal an exclusively held semaphore.
 */
__attribute__((weak))
void semaphore_signal_excl(semaphore_t *s)
{
	mutex_release(&s->exclusive);
}

/*
 * Try to signal a semaphore annd return
 * true if successful.
 */
__attribute__((weak))
bool semaphore_trysignal(semaphore_t *s)
{
	if (unlikely(!mutex_try((mutex_t*) &s->enter)))
		return false;
	if (likely(s->cnt > 0))
	{
		s->cnt--;
		mutex_release((mutex_t*) &s->enter);
		return true;
	}
	mutex_release((mutex_t*) &s->enter);
	return false;
}
