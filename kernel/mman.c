/*
 * kernel/mman.c
 * 
 * Author: Fernando Rodriguez
 * Email: frodriguez.developer@outlook.com
 * 
 * Copyright 2014-2015 Fernando Rodriguez
 * All rights reserved.
 * 
 * This code is distributed for educational purposes
 * only. It may not be distributed without written 
 * permission from the author.
 *
 */

#include <arch/arch.h>
#include <sys/mman.h>
#include <mmu.h>
#include <io.h>
#include <errno.h>

/*
 * Map a file to the current address space
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	file_t *filedesc;
	if (unlikely((flags & (MAP_SHARED | MAP_PRIVATE)) == 0))
		return EINVAL;
	filedesc = get_file_descriptor(fd);
	if (unlikely(filedesc == NULL))
		return EBADF;
	
	return NULL;
}
