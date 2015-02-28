#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <syscall.h>

/*
 * 
 */
int main(int argc, char **argv)
{
	int i;
	
	printf("Hello World! %i\n", 1);
	
	for (i = 1; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	putchar('\n');
	return 0;
}

