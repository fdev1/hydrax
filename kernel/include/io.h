/*
 * include/io.h
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

#ifndef __IO_H__
#define __IO_H__

#include <vfs.h>
#include <sys/stat.h>
#include <mutex.h>
#include <unistd.h>

#define FD_MODE_READ		(0x01)
#define FD_MODE_WRITE		(0x02)

typedef struct __file_descriptor
{
	int fd;
	unsigned int offset;
	unsigned int mode;
	vfs_node_t *node;
	struct __file_descriptor *next;
}
file_t;

/*
 * Descriptor table entry structure
 */
typedef struct  __descriptor_table
{
	unsigned int fd_next;
	unsigned int refs;
	mutex_t lock;
	file_t *file_descriptors;
}
desc_info_t, desc_entry_t;

/*
 * Clones the current task file descriptors list.
 */
desc_info_t *clone_descriptor_table(void);

/*
 * Destroy the task's descriptor table.
 */
void destroy_descriptor_table(void);


#endif

