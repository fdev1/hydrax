/*
 * include/initrd.h
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

#ifndef __INITRD_H__
#define __INITRD_H__

#include "arch/platform.h"
#include "vfs.h"

#if 0
typedef struct
{
    uint32_t nfiles;
} 
initrd_header_t;

typedef struct
{
	uint8_t magic;
	char name[64];
	uint32_t offset;
	uint32_t length;
} 
initrd_file_header_t;
#endif

/*
 * initialize the initrd and return it's node
 */
vfs_node_t *initrd_init(intptr_t location);

#endif
