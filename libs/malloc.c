#include <unistd.h>
#include <malloc.h>
#include <bget/bget.h>

char heap_mem[1024 * 1024];


void malloc_init(void)
{
	bpool(heap_mem, 1024 * 1024);
}


void *malloc(size_t sz)
{
	return bget(sz);
}

void free(void)
{
	return brel();
}

