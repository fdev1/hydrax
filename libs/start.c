#include <arch/stdarg.h>
#include "stdio.h"
#include <unistd.h>

extern int main(int argc, char **argv);

int _start(int argc, char **argv)
{
	exit(main(argc, argv));
}
