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
#include <kheap.h>
#include <mutex.h>
#include <scheduler.h>

/*
 * Open a shared memory file
 */
int shm_open(const char *a, int b, mode_t c)
{
	return ENOSYS;
}

/*
 * Map a file to the current address space
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	void *ap;
	file_t *filedesc;
	mmap_info_t *mmap_info, **tmp;
	unsigned int kflags;
	
	if (unlikely((flags & (MAP_SHARED | MAP_PRIVATE)) == 0))
	{
		current_task->errno = EINVAL;
		return NULL;
	}
	filedesc = get_file_descriptor(fd);
	if (unlikely(filedesc == NULL))
	{
		current_task->errno = EBADF;
		return NULL;
	}
	
	mmap_info = (mmap_info_t*) malloc(sizeof(mmap_info_t));
	if (unlikely(mmap_info == NULL))
	{
		current_task->errno = ENOMEM;
		return NULL;
	}
	
	mmap_info->addr = addr;
	mmap_info->len = len;
	mmap_info->offset = off;
	mmap_info->prot = prot;
	mmap_info->flags = flags;
	mmap_info->fd = fd;
	mmap_info->next = NULL;
	
	kflags = KALLOC_OPTN_MMAP;
	if ((prot & PROT_WRITE) != 0)
		kflags |= KALLOC_OPTN_WRITEABLE;
	/* if (prot & PROT_EXEC) */
	/* if (prot & PROT_READ) */
	/* if (prot & PROT_NONE) what?? */
	if (flags & MAP_PRIVATE)
		kflags |= KALLOC_OPTN_NOCOW;
		

	/*
	 * Allocate the pages on the current process address
	 * space but don't map them. They will be mapped on access
	 * by the MMU driver.
	 */
	ap = (void*) kalloc(len, (intptr_t) addr, NULL, kflags);
	if (unlikely(ap == NULL))
	{
		current_task->errno = ENOMEM;
		return NULL;
	}

	/*
	 * Add the mmap to the current task mmap table.
	 */
	mutex_wait(&current_task->mmaps_lock);
	tmp = &current_task->mmaps;
	while (*tmp != NULL)
		tmp = &(*tmp)->next;
	*tmp = mmap_info;
	mutex_release(&current_task->mmaps_lock);
	
	return ap;
}
