#include <arch/stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <syscall.h>

#include "args.h"

/*
 * 
 */
int main(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	putchar('\n');
	return 0;
}

