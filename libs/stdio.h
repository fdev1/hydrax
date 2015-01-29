#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <arch/platform.h>

#define STDIN		(0)
#define STDOUT 	(1)
#define STDERR 	(2)


int getchar(void);
int putchar(int);

void printf(const char *fmt, ...);


#endif

