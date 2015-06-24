/*
 * procfs.c
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
 */

#include <arch/arch.h>
#include <scheduler.h>
#include <vfs.h>
#include <procfs.h>
#include <unistd.h>
#include <kheap.h>
#include <string.h>
#include <printk.h>
#include <memmap.h>

/*
 * Global variables
 */
vfs_node_t *procfs = NULL;

/*
 * Implements the procfs readdir() function.
 */
struct dirent *procfs_readdir(vfs_node_t *node, uint32_t index, struct dirent *buf)
{
	LINKED_LIST_ITER_STRUCT(task_t) iter;
	
	assert(node != NULL);
	assert(buf != NULL);
	assert((node->flags & 0x7) == FS_DIRECTORY);

	if (node == procfs)
	{
		assert(0);
		int i;
		if (tasks_list.count == 0)
			return NULL;
		#if 0
		tmp = tasks;
		for (i = 0; i < index && tmp->next != NULL; i++)
			tmp = tmp->next;
		if (i < index)
			return NULL;
		strcpy(buf->name, itoa(tmp->id));
		#else
		iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
		for (i = 0; i < index && i < tasks_list.count; i++)
			LINKED_LIST_MOVE_NEXT(iter);
		strcpy(buf->name, itoa(iter.current->val->id));
		#endif
		return buf;
	}
	else
	{
		switch (index)
		{
			case 0:
				strcpy(buf->name, "cmdline");
				return buf;
			case 1:
				strcpy(buf->name, "environ");
				return buf;
			case 2: 
				strcpy(buf->name, "memmap");
				return buf;
			default:
				return NULL;
		}
	}
}

uint32_t procfs_readstrings(char **strings, size_t sz, unsigned char *buf)
{
	int i;
	if (strings == NULL)
		return 0;
	i = 0;
	while (*strings != NULL && sz)
	{
		char *s;
		s = *strings++;
		if (s != NULL)
		{
			while (*s != NULL && sz != 0)
			{
				*buf++ = *s++;
				i++;
				sz--;
			}
		}
		
		if (sz != 0)
		{
			*buf++ = ' ';
			i++;
			sz--;
		}
	}
	return i;
}

/*
 * Implements the procfs read() function.
 */
uint32_t procfs_read(vfs_node_t *node, uint32_t offset, uint32_t sz, uint8_t *buf)
{
	task_t *task;
	LINKED_LIST_ITER_STRUCT(task_t) iter;

	assert(node != NULL);
	
	if (sz == 0)
		return 0;
	
	#if 0
	mutex_busywait(&schedule_lock);
	task = tasks;
	while (task != NULL && task->id != node->inode)
		task = task->next;
	mutex_release(&schedule_lock);
	#else
	mutex_busywait(&schedule_lock);
	iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
	LINKED_LIST_SEEK_UNTIL(iter, iter.current->val->id != node->inode);
	task = iter.current->val;
	mutex_release(&schedule_lock);
	#endif
	
	if (task == NULL || task->argv == NULL)
		return 0;
	if (!strcmp(node->name, "cmdline"))
	{
		return procfs_readstrings(task->argv, sz, buf);
	}
	else if (!strcmp(node->name, "environ"))
	{
		return procfs_readstrings(task->envp, sz, buf);
	}
	else if (!strcmp(node->name, "memmap"))
	{
		memmap_t *memmap;
		unsigned int i, end_byte, bytes_written;
		memmap = arch_get_memmap(task->machine_state);
		end_byte = sz + offset;
		bytes_written = 0;
		
		if (end_byte > (memmap->length / 32) * 4)
			end_byte = (memmap->length / 32) * 4;
		
		for (i = offset; i < end_byte; i++)
			buf[bytes_written++] = ((uint8_t*)memmap->pagemap)[i];
		return bytes_written;		
	}
	else
	{
		return 0;
	}
}

/*
 * Implements procfs open() function.
 */
vfs_node_t *procfs_open(vfs_node_t *node, const char *path, uint32_t flags)
{
	task_t *tmp = NULL;
	vfs_node_t *open_node;
	LINKED_LIST_ITER_STRUCT(task_t) iter;

	mutex_busywait(&schedule_lock);
	
	#if 0
	tmp = tasks;
	while (tmp != NULL && tmp->procfs_node != node)
		tmp = tmp->next;
	#else
	iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
	LINKED_LIST_SEEK_UNTIL(tasks_list, iter.current->val->procfs_node == node);
	if (iter.current != NULL)
		tmp = iter.current->val;
	#endif
	
	if (tmp == NULL)
	{
		mutex_release(&schedule_lock);
		return NULL;
	}

	open_node = NULL;

	if (!strcmp(path, "cmdline"))
	{
		open_node = (vfs_node_t*) malloc(sizeof(vfs_node_t));
		if (open_node == NULL)
		{
			mutex_release(&schedule_lock);
			return NULL;	/* we need a way to return error code */
		}
		strcpy(open_node->name, "cmdline");
		//open_node->open = (stat_type_t) &procfs_stat;
		open_node->uid = 1;
		open_node->gid = 1;
		open_node->inode = tmp->id;
		open_node->length = 43;
		open_node->read = (vfs_read_fn_t) &procfs_read;
		open_node->write = NULL;
	}
	else if (!strcmp(path, "environ"))
	{
		open_node = (vfs_node_t*) malloc(sizeof(vfs_node_t));
		if (open_node == NULL)
		{
			mutex_release(&schedule_lock);
			return NULL;	/* we need a way to return error code */
		}
		strcpy(open_node->name, "environ");
		//open_node->open = (stat_type_t) &procfs_stat;
		open_node->uid = 1;
		open_node->gid = 1;
		open_node->inode = tmp->id;
		open_node->length = 43;
		open_node->read = (vfs_read_fn_t) &procfs_read;
		open_node->write = NULL;		
	}
	else if (!strcmp(path, "memmap"))
	{
		memmap_t *memmap;
		memmap = arch_get_memmap(tmp->machine_state);
		open_node = (vfs_node_t*) malloc(sizeof(vfs_node_t));
		if (open_node == NULL)
		{
			mutex_release(&schedule_lock);
			return NULL;
		}
		strcpy(open_node->name, "memmap");
		open_node->uid = 0;
		open_node->gid = 0;
		open_node->inode = tmp->id;
		open_node->length = (memmap->length / 32) * 4 ;
		open_node->read = (vfs_read_fn_t) &procfs_read;
		open_node->write = NULL;				
	}

	mutex_release(&schedule_lock);
	return open_node;
}

/*
 * procfs stat handler
 */
struct stat *procfs_stat(vfs_node_t *node, const char *path, struct stat *buf)
{
	struct stat *s;
	vfs_node_t *stat_node;
	stat_node = procfs_open(node, path, NULL);
	s = vfs_stat(stat_node, NULL, buf);
	procfs_close(stat_node);
	return s;
}

/*
 * 
 */
void procfs_close(vfs_node_t *node)
{
	assert(node != NULL);
	free(node);
}

/*
 * Initialize /proc device node.
 */
void procfs_init(void)
{
	int r;
	procfs = (vfs_node_t*) malloc(sizeof(vfs_node_t));
	if (procfs == NULL)
		panic("procfs_init: out of kernel memory");

	*procfs = vfs_node_init(FS_DIRECTORY);
	strcpy(procfs->name, "proc");
	procfs->length = sizeof(vfs_node_t);
	r = vfs_mknod(NULL, procfs);
	if (r)
		panic("procfs: could not create node!");
	printk(7, "procfs: procfs ready");
}

