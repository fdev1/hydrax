#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <arch/arch.h>
#include <unistd.h>
#include <signal.h>

typedef uint32_t pthread_attr_t;
typedef void* (*pthread_start_fn)(void*);

/*
 * Creates a new thread.
 * 
 * NOTE: The stack argument is not part of pthreads.
 * I added it temporarily until I get malloc working
 * on the C library.
 */
int pthread_create(pthread_t * thread, 
	const pthread_attr_t * attr,
	pthread_start_fn start_routine,
	void *arg);

/*
 *  * Send signal to a thread
 *   */
int pthread_kill(pthread_t, int);


int pthread_exit(int status_code);

#endif
