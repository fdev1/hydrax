/*
 * include/procfs.h
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

#ifndef __PROCFS_H__
#define __PROCFS_H__

#include <sys/stat.h>
#include <dirent.h>

extern vfs_node_t *procfs;


/*
 * Implements procfs readdir function.
 */
struct dirent *procfs_readdir(vfs_node_t *node, uint32_t index, struct dirent *buf);

/*
 * procfs stat handler
 */
struct stat *procfs_stat(vfs_node_t *node, const char *path, struct stat *buf);

uint32_t procfs_readstrings(char **strings, size_t sz, unsigned char *buf);

/*
 * Implements the procfs read() function.
 */
uint32_t procfs_read(vfs_node_t *node, uint32_t offset, uint32_t sz, uint8_t *buf);

/*
 * Implements procfs open function.
 */
vfs_node_t *procfs_open(vfs_node_t *node, const char *path, uint32_t flags);

/*
 * 
 */
void procfs_close(vfs_node_t *node);

/*
 * Initialize procfs file system.
 */
void procfs_init(void);

#endif
