#include <unistd.h>
#include <pthread.h>

/*
 * Create a new thread;
 */
int pthread_create(pthread_t *thread, 
	void *stack,
	const pthread_attr_t *attr,
	void *(*start_routine)(void*),
	void * arg)
{
	pid_t pid;	
	pid = clone(stack);
	if (pid < 0)
		return -1;
	
	if (pid == 0)
	{
		start_routine(arg);
		exit(0);
	}
	return thread->id = pid;
}
