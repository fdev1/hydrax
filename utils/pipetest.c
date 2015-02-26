#include <stdint.h>
#include <sys/types.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main()
{
	pid_t pid;
	int pipefd[2];
	
	if (pipe(pipefd) < 0)
	{
		printf("Pipe error\n");
		return -1;
	}
	
	pid = fork();
	if (pid < 0)
	{
		printf("Fork error\n");
		return -1;
	}
	
	if (pid == 0)
	{
		close(pipefd[0]);
		while (1)
		{
			char c;
			c = (char) getchar();
			write(pipefd[1], (uint8_t*) &c, 1);
			
		}
	}
	else
	{
		char c;
		close(pipefd[1]);
		while (1)
		{
			read(pipefd[0], (uint8_t*) &c, 1);
			putchar(c);
		}
	}	
}
