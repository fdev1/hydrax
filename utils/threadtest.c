#include <assert.h>
#include <arch/stdarg.h>
#include <unistd.h>
#include <mutex.h>
#include <string.h>
#include <stdio.h>

mutex_t mutex = MUTEX_INITIALIZER;
unsigned int stack[1024];
void syscall_test();
int x;

void thread_entry(void *argument)
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
}

int main()
{
	int i = 3;
	
	if (clone() != 0)
	{
		while (i--)
		{
			mutex_wait(&mutex);
			printf("cloned pid=%i tid=%i.\n", getpid(), gettid());
			mutex_release(&mutex);
			yield();
		}
		exit(0);
	}
	else
	{
		asm __volatile__(
			"mov %1, %%esp;"
			"call *%0;" : : "r" (&thread_entry), "i" (&stack[255]));
		exit(0);
	}	
	return 0;
}
