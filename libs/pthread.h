#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <arch/platform.h>
#include <unistd.h>

typedef struct __pthread
{
	pid_t id;
	void *stack;
}
pthread_t;

typedef struct __pthread_attr
{
	int x;
}
pthread_attr_t;

/*
 * Creates a new thread.
 * 
 * NOTE: The stack argument is not part of pthreads.
 * I added it temporarily until I get malloc working
 * on the C library.
 */
int pthread_create(pthread_t * thread, 
	void *stack,
	const pthread_attr_t * attr,
	void *(*start_routine)(void*),
	void *arg);

#endif
