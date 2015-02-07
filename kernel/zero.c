/*
 * zero.c
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
#include <memory.h>

static vfs_node_t zerodev;

/*
 * read from the null device
 */
static uint32_t zero_read(vfs_node_t *node, uint32_t offset, uint32_t len, uint8_t *buf)
{
	assert(buf != NULL);
	memset(buf, 0, len);
	return len;
}

/*
 * Initialize null device
 */
void zero_init(void)
{
	int r;
	zerodev = vfs_node_init(FS_CHARDEVICE);
	strcpy(zerodev.name, "zero");
	zerodev.major = 8;
	zerodev.minor = 1;
	zerodev.read = (vfs_read_fn_t) &zero_read;
	zerodev.length = sizeof(vfs_node_t);

	r = devfs_mknod(&zerodev);
	if (r == -1)
		panic("null: could not create dev node!");

	return;
}
