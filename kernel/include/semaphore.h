/*
 * include/semaphore.h
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

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <mutex.h>

/*
 * Semaphore structure
 */
typedef struct __semaphore
{
	unsigned int cnt;
	unsigned int max_cnt;
	light_mutex_t enter;
	mutex_t exclusive;
	light_mutex_t waiters_lock;
	mutex_waiter_t *waiters;
}
semaphore_t;

/*
 * Light semaphore structure - This version can be used
 * with all functions except the waitsleep() and releasesleep()
 * family of functions. It'll have to be casted to semaphore_t*
 * when passing it to the function
 */
typedef struct __light_semaphore
{
	unsigned int cnt;
	unsigned int max_cnt;
	light_mutex_t enter;
	light_mutex_t exclusive;
}
light_semaphore_t;

/*
 * Semaphore structure initializer
 */
#define SEMAPHORE_INITIALIZER(max, count)			\
	(semaphore_t) {							\
		.enter = LIGHT_MUTEX_INITIALIZER,			\
		.exclusive = MUTEX_INITIALIZER,			\
		.waiters_lock = LIGHT_MUTEX_INITIALIZER,	\
		.waiters = NULL,						\
		.max_cnt = max,						\
		.cnt = count							\
	}

/*
 * Light semaphore structure initializer
 */
#define LIGHT_SEMAPHORE_INITIALIZER(max, count)		\
	(light_semaphore_t) {						\
		.cnt = count,							\
		.max_cnt = max,						\
		.enter = LIGHT_MUTEX_INITIALIZER,			\
		.exclusive = LIGHT_MUTEX_INITIALIZER		\
	}
		
	

bool semaphore_try(semaphore_t *s);
bool semaphore_try_excl(semaphore_t *s);
void semaphore_wait(semaphore_t *s);
void semaphore_wait_excl(semaphore_t *s);
void semaphore_busywait(semaphore_t *s);
void semaphore_busywait_excl(semaphore_t *s);
void semaphore_waitsleep(semaphore_t *s);
void semaphore_waitsleep_excl(semaphore_t *s);
void semaphore_signal(semaphore_t *s);

/*
 * Signal a semaphore and wake up any sleeping waiters.
 */
void semaphore_signal_sleep(semaphore_t *s);

void semaphore_signal_excl(semaphore_t *s);
bool semaphore_trysignal(semaphore_t *s);
bool semaphore_trysignal_excl(semaphore_t *s);

#endif
