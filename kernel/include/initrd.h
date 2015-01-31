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

#include <arch/arch.h>
#include <vfs.h>

/*
 * initialize the initrd and return it's node
 */
vfs_node_t *initrd_init(intptr_t location);

#endif
