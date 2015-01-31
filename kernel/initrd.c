/*
 * initrd.c 
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

#if defined(KERNEL_CODE)
#undef KERNEL_CODE
#endif
#define KERNEL_CODE

#include <arch/arch.h>
#include <initrd.h>
#include <kheap.h>
#include <memory.h>
#include <string.h>
#include <printk.h>
#include <vfs.h>

/*
 * Initial ram disk header.
 */
typedef struct __initrd_header
{
	uint32_t magic;
	uint32_t sz;
}
initrd_header_t;

/*
 * Holds pointer to root node.
 */
static vfs_node_t *root = NULL;

/*
 * Write to the initial ram disk
 */
static uint32_t initrd_read(vfs_node_t *node, 
	uint32_t offset, uint32_t size, uint8_t *buffer)
{
	assert(node != NULL);
	if (offset > node->length)
		return 0;
	if (offset + size > node->length)
		size = node->length-offset;
	if (size < 0)
		return 0;
	memcpy(buffer, ((uint8_t*) node->data) + offset, size);
	return size;
}

/*
 * initialize initial ram disk
 */
vfs_node_t *initrd_init(uint32_t offset)
{
	int i;
	initrd_header_t *header;	
	vfs_node_t *p_node;
	header = (initrd_header_t*) offset;
	root = (vfs_node_t*) (header + 1);
	p_node = root;
	
	for (i = 0; i < header->sz; i++)
	{
		if (p_node->ptr != NULL)
			p_node->ptr = (vfs_node_t*) (((uint32_t) p_node->ptr) + offset);
		if (p_node->next != NULL)
			p_node->next = (vfs_node_t*) (((uint32_t) p_node->next) + offset);		
		if (p_node->parent != NULL)
			p_node->parent = (vfs_node_t*) (((uint32_t) p_node->parent) + offset);		
		if (p_node->length > 0)
		{
			p_node->data = (void*) (((unsigned int) p_node->data) + offset);
			p_node->read = (read_type_t) &initrd_read;
		}
		p_node++;
	}
	
	return root;
}
