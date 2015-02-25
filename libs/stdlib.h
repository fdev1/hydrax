#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <malloc.h>

int atoi(const char *str);

char *getenv(const char *name);

void abort(void);

int atexit(void (*function)(void));


#endif
