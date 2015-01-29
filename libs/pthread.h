#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <arch/platform.h>
#include <unistd.h>

typedef struct __pthread
{
	pid_t tid;
	void *stack;
}
pthread_t;

typedef struct __pthread_attr
{
	int x;
}
pthread_attr_t;

int pthread_create(pthread_t * thread, 
	const pthread_attr_t * attr,
	void *(*start_routine)(void*),
	void *arg);

#endif
