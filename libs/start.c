#include <arch/stdarg.h>
#include "stdio.h"
#include <unistd.h>
#include <malloc.h>

extern int main(int argc, char **argv);

int _start(int argc, char **argv)
{
	malloc_init();
	exit(main(argc, argv));
}
