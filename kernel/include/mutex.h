/*
 * include/mutex.h
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

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <sys/types.h>
#include <assert.h>

/*
 * A light mutex that can be used with all
 * the mutex calls except mutex_waitsleep()
 */
typedef struct __light_mutex
{
	int s;
}
light_mutex_t;

/*
 * Mutex waiter structure.
 */
typedef struct __mutex_waiter
{
	pid_t pid;
	bool waiting;
	struct __mutex_waiter *next;
}
mutex_waiter_t;

/*
 * Mutex structure
 */
typedef struct __mutex
{
	int s;
	light_mutex_t waiters_lock;
	mutex_waiter_t *waiters;
}
mutex_t;

#define NULL_SEMAPHORE		({0,0})

/*
 * initialize a semaphore
 *
 */
#define MUTEX_INITIALIZER 			(mutex_t) {0,0,0}
#define MUTEX_INITIALIZER_AUTOSET		(mutex_t) {1,0,0}
#define LIGHT_MUTEX_INITIALIZER		(light_mutex_t) { 0 }

/*
 * Forward declaration for mutex_attach_waiter()
 */
int getpid(void);
int gettid(void);
void yield(void);

/*
 * try a mutex and grabs it if available
 */
static inline int mutex_try(mutex_t *mutex)
{
	register int i = 1;
	arch_exchange_rm(i, mutex->s);
	assert(i == 1 || i == 0);
	return !i;
}

/*
 * Spin the CPU until the mutex is acquired.
 */
static inline void mutex_busywait(mutex_t *mutex)
{
	while (!mutex_try(mutex));
}

/*
 * Yield the CPU until the mutex is acquired.
 */
static inline void mutex_wait(mutex_t *mutex)
{
	while (!mutex_try(mutex))
		yield();
}

/*
 * release mutex
 */
static inline void mutex_release(mutex_t *mutex)
{
	register int i = 0;
	arch_exchange_rm(i, mutex->s);
	assert(i == 1);
	/*yield();*/
}

#if defined(KERNEL_CODE)
/*
 * Attach a waiter from the waiters list.
 * TODO: Do it without the branch
 */
static inline void mutex_attach_waiter(
	mutex_waiter_t **waiters_list, light_mutex_t *lock, mutex_waiter_t *waiter)
{
	mutex_wait((mutex_t*) lock);
	waiter->waiting = 1;
	waiter->pid = gettid();
	waiter->next = NULL;
	if (*waiters_list == NULL)
	{
		*waiters_list = waiter;
	}
	else
	{
		mutex_waiter_t *waiters;
		waiters = *waiters_list;
		while (waiters->next != NULL)
			waiters = waiters->next;
		waiters->next = waiter;
	}
	mutex_release((mutex_t*) lock);
}

/*
 * Detach a waiter to a waiters list.
 * TODO: Do it without the branch
 */
static inline void mutex_detach_waiter(
	mutex_waiter_t **waiters_list, light_mutex_t *lock, mutex_waiter_t *waiter)
{
	assert(*waiters_list != NULL);
	mutex_wait((mutex_t*) lock);
	if (*waiters_list == waiter)
	{
		*waiters_list = waiter->next;
	}
	else
	{
		mutex_waiter_t *waiters, *parent;
		waiters = (*waiters_list)->next;
		parent = *waiters_list;
		while (waiters != waiter)
		{
			parent = waiters;
			waiters = waiters->next;
			assert(waiters != NULL);
		}
		parent->next = waiters->next;
	}
	mutex_release((mutex_t*) lock);
}

/*
 * Put the task to sleep until the mutex is acquired.
 */
void mutex_waitsleep(mutex_t*);

/*
 * Release a mutex and signal any sleeping waiters.
 */
void mutex_releasesleep(mutex_t *m);
#endif


#endif

