/*
 * include/sys/mman.h
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

#ifndef __MMAN_H__
#define __MMAN_H__


#define PROT_NONE			(0x00000)	/* Page cannot be accessed. */
#define PROT_READ			(0x00001)	/* Page can be read. */
#define PROT_WRITE			(0x00002)	/* Page can be written. */
#define PROT_EXEC			(0x00004)	/* Page can be executed. */

#define MAP_SHARED			(0x00008) 	/* Share changes. */
#define MAP_PRIVATE			(0x00010)	/* Changes are private. */
#define MAP_FIXED			(0x00020)	/* Interpret addr exactly. */

#define MS_ASYNC			(0x00040)	/* Perform asynchronous writes. */
#define MS_SYNC			(0x00080)	/* Perform synchronous writes. */
#define MS_INVALIDATE		(0x00100)	/* Invalidate mappings. */

#define MCL_CURRENT			(0x00200)	/* Lock currently mapped pages. */
#define MCL_FUTURE			(0x00400)	/* Lock pages that become mapped. */

#define POSIX_MADV_NORMAL	(0x00800) 	/* The application has no advice to give on its 
								   behavior with respect to the specified range. 
								   It is the default characteristic if no advice is 
								   given for a range of memory. */
#define POSIX_MADV_SEQUENTIAL	(0x01000)	/* The application expects to access the specified range 
								   sequentially from lower addresses to higher addresses. */
#define POSIX_MADV_RANDOM	(0x02000)	/* The application expects to access the specified range in 
								   a random order. */
#define POSIX_MADV_WILLNEED	(0x04000)	/* The application expects to access the specified range in 
								   the near future. */
#define POSIX_MADV_DONTNEED	(0x08000)	/* The application expects that it will not access the specified 
								   range in the near future. */

/*
 * The following flags shall be defined for 
 * posix_typed_mem_open():
 */
#define POSIX_TYPED_MEM_ALLOCATE		(0x10000)	/* Allocate on mmap() */
#define POSIX_TYPED_MEM_ALLOCATE_CONTIG	(0x20000)	/* Allocate contiguously on mmap() */
#define POSIX_TYPED_MEM_MAP_ALLOCATABLE	(0x40000)	/* Map on mmap(), without affecting allocatability. */

int mlock(const void *, size_t);
int mlockall(int);
void *mmap(void *, size_t, int, int, int, off_t);
int mprotect(void *, size_t, int);
int msync(void *, size_t, int);
int munlock(const void *, size_t);
int munlockall(void);
int munmap(void *, size_t);
int posix_madvise(void *, size_t, int);
int posix_mem_offset(const void *, size_t, off_t *, size_t *, int *);
int posix_typed_mem_get_info(int, struct posix_typed_mem_info *);
int posix_typed_mem_open(const char *, int, int);
int shm_open(const char *, int, mode_t);
int shm_unlink(const char *);

#endif
