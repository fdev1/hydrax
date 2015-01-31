#ifndef __STDIO_H__
#define __STDIO_H__

#include <arch/arch.h>

#define STDIN		(0)
#define STDOUT 		(1)
#define STDERR 		(2)


int getchar(void);
int putchar(int);

void printf(const char *fmt, ...);


#endif

