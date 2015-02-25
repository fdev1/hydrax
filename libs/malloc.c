#include <unistd.h>
#include <malloc.h>
#include <bget/bget.h>

unsigned char heap_mem[1024 * 8];


void malloc_init(void)
{
	bpool(heap_mem, 1024 * 8);
}


void *malloc(size_t sz)
{
	return bget(sz);
}

void free(void* ptr)
{
	return brel(ptr);
}

