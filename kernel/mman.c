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

mmap_info_t *mmaps = NULL;
light_mutex_t mmaps_lock = LIGHT_MUTEX_INITIALIZER;

/*
 * Map a file to the current address space
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	file_t *filedesc;
	mmap_info_t *mmap_info, **tmp;
	void *ap;

	if (unlikely((flags & (MAP_SHARED | MAP_PRIVATE)) == 0))
		return EINVAL;
	filedesc = get_file_descriptor(fd);
	if (unlikely(filedesc == NULL))
		return EBADF;
	
	mmap_info = (mmap_info_t*) malloc(sizeof(mmap_info_t));
	if (unlikely(mmap_info == NULL))
		return ENOMEM;
	
	mmap_info->addr = addr;
	mmap_info->len = len;
	mmap_info->offset = off;
	mmap_info->node = filedesc->node;
	
	mutex_wait(&filedesc->node->lock);
	filedesc->node->refs++;
	mutex_release(&filedesc->node->lock);
	
	mutex_wait(&mmaps_lock);
	tmp = &mmaps;
	while (*tmp != NULL)
		tmp = &(*tmp)->next;
	*tmp = mmap_info;
	mutex_release(&mmaps_lock);
	
	ap = kalloc(len, (intptr_t) addr, NULL, KALLOC_OPTN_MMAP);
	if (unlikely(ap == NULL))
	{
		mutex_wait(&filedesc->node->lock);
		filedesc->node->refs++;
		mutex_release(&filedesc->node->lock);
	
		mutex_wait(&mmaps_lock);
		tmp = &mmaps;
		while (*tmp != mmap_info)
			tmp = &(*tmp)->next;
		*tmp = (*tmp)->next;
		mutex_release(&mmaps_lock);
		return ENOMEM;
	}
	
	return ap;
}
