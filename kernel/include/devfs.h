/*
 * include/devfs.h
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

#ifndef __DEVFS_H__
#define __DEVFS_H__

#include <vfs.h>

void devfs_init(void);

int devfs_mknod(vfs_node_t *node);


#endif

