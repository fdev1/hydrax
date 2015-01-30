#include <assert.h>
#include <arch/stdarg.h>
#include <unistd.h>
#include <mutex.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>


mutex_t mutex = MUTEX_INITIALIZER;
unsigned int stack[1024];
void syscall_test();
int x;

void *thread_entry(void *argument)
{
	int i = 3;
	printf("Argument at 0x%x\n", (uint32_t) &argument);
	while(i--)
	{
		mutex_wait(&mutex);
		printf("cloned pid=%i tid=%i\n", getpid(), gettid());
		mutex_release(&mutex);
		yield();
	}
	exit(0);
	return (void*) NULL;
}

int main()
{
	int i;
	int tid;
	pthread_t thread;
	
	tid = pthread_create(&thread, &stack[1023], NULL, &thread_entry, (void*)&thread);
	if (tid < 0)
	{
		printf("pthread_create failed!\n");
		exit(-1);
	}
	
	i = 3;
	while (i--)
	{
		mutex_wait(&mutex);
		printf("cloned pid=%i tid=%i.\n", getpid(), gettid());
		mutex_release(&mutex);
		yield();
	}

	return 0;
}
