#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <unistd.h>

void malloc_init(void);

/*
 * Allocate heap memory.
 */
void *malloc(size_t sz);

/*
 * Free heap memory.
 */
void free(void);

#endif

