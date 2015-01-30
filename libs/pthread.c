#include <unistd.h>
#include <pthread.h>
#include <malloc.h>

#define INITIAL_STACK_SIZE	(1024)


/*
 * Create a new thread;
 */
int pthread_create(pthread_t *thread, 
	const pthread_attr_t *attr,
	void *(*start_routine)(void*),
	void * arg)
{
	pid_t pid;	
	unsigned int *stack;
	
	stack = (unsigned int*) malloc(INITIAL_STACK_SIZE * sizeof(unsigned int));
	if (stack == NULL)
		return ENOMEM;
	
	pid = clone(&stack[INITIAL_STACK_SIZE - 1]);
	if (pid < 0)
		return -1;
	
	if (pid == 0)
	{
		start_routine(arg);
		exit(0);
	}
	return thread->id = pid;
}
