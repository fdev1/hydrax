/*
 * include/elfldr.h
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

#ifndef __ELFLDR_H__
#define __ELFLDR_H__

#include <arch/platform.h>
#include <vfs.h>

intptr_t elf_load(vfs_node_t *node);

#endif

