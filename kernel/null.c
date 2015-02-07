/*
 * null.c
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
#include <vfs.h>
#include <kheap.h>
#include <string.h>
#include <devfs.h>

static vfs_node_t nulldev;

/*
 * read from the null device
 */
static uint32_t null_read(vfs_node_t *node, uint32_t offset, uint32_t len, uint8_t *buf)
{
	return len;
}

/*
 * writes to the null device
 */
static uint32_t null_write(vfs_node_t *node, uint32_t offset, unsigned int len, uint8_t *buf)
{
	return len;
}

/*
 * Initialize null device
 */
void null_init(void)
{
	int r;
	nulldev = vfs_node_init(FS_CHARDEVICE);
	strcpy(nulldev.name, "null");
	nulldev.major = 7;
	nulldev.minor = 1;
	nulldev.read = (vfs_read_fn_t) &null_read;
	nulldev.write = (vfs_write_fn_t) &null_write;
	nulldev.length = sizeof(vfs_node_t);

	r = devfs_mknod(&nulldev);
	if (r == -1)
		panic("null: could not create dev node!");

	return;
}
