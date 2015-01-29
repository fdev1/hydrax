/*
 * include/vfs.h
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

#ifndef __VFS_H__
#define __VFS_H__

#include <arch/platform.h>
#include <mutex.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/stat.h>

#define FS_FILE        		(0x00000001)
#define FS_DIRECTORY   		(0x00000002)
#define FS_CHARDEVICE  		(0x00000003)
#define FS_BLOCKDEVICE 		(0x00000004)
#define FS_PIPE        		(0x00000005)
#define FS_SYMLINK     		(0x00000006)
#define FS_MOUNTPOINT  		(0x00000008)
#define FS_USR_EXECUTE		(0x00000100)
#define FS_USR_WRITE		(0x00000200)
#define FS_USR_READ			(0x00000400)
#define FS_GRP_EXECUTE		(0x00000800)
#define FS_GRP_WRITE		(0x00001000)
#define FS_GRP_READ			(0x00002000)
#define FS_ANY_EXECUTE		(0x00004000)
#define FS_ANY_WRITE		(0x00008000)
#define FS_ANY_READ			(0x00010000)
#define FS_AUTOFREE			(0x00020000)


struct vfs_node;

/*
 *
 *
 */
typedef uint32_t (*read_type_t)(struct vfs_node *node, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct vfs_node *node, uint32_t, uint32_t, const uint8_t*);
typedef void *(*open_type_t)(struct vfs_node *node, const char *path, uint32_t flags);
typedef void (*close_type_t)(struct vfs_node *node);
typedef struct dirent * (*readdir_type_t)(struct vfs_node *node, uint32_t index, struct dirent *buf);
typedef void* (*finddir_type_t)(struct vfs_node *node, char *name);
typedef struct stat* (*stat_type_t)(struct vfs_node *node, const char *path, struct stat* buf);
typedef int (*ioctl_type_t)(struct vfs_node *node, unsigned long request, void *last_arg);


/*
 * vfs node structure
 */
typedef struct vfs_node
{
	char name[MAX_FILENAME + 1];
	uid_t uid;
	gid_t gid;
	uint32_t mask;
	uint32_t flags;
	uint32_t inode;
	uint32_t major;
	uint32_t minor;
	uint32_t length;
	uint32_t impl;
	uint32_t refs;
	void *data;
	mutex_t lock;
	semaphore_t semaphore;
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
	stat_type_t stat;
	ioctl_type_t ioctl;
	struct vfs_node *parent;
	struct vfs_node *ptr;
	struct vfs_node *next;		/* next sibling */
} 
vfs_node_t;

typedef struct __pipe
{
	uint8_t *buf;
	uint8_t *buf_end;
	uint8_t *read_ptr;
	uint8_t *write_ptr;
	semaphore_t semaphore;
}
pipe_t;


/*
 * initialize a file node
 *
 */
#define vfs_node_init(f)	VFS_NODE_INITIALIZER(f)

#define VFS_NODE_INITIALIZER(f)			\
	(vfs_node_t) {					\
		.name = { NULL },			\
		.mask = 777, 				\
		.uid = 0,					\
		.gid = 0,					\
		.flags = f,				\
		.inode = 0, 				\
		.major = 0,				\
		.minor = 0,				\
		.length = 0,				\
		.impl = 0,				\
		.refs = 0,				\
		.data = 0,				\
		.lock = MUTEX_INITIALIZER,	\
		.read = NULL,				\
		.write = NULL,				\
		.open = NULL,				\
		.close = NULL, 			\
		.readdir = NULL, 			\
		.finddir = NULL,			\
		.stat = NULL,				\
		.ioctl = NULL,				\
		.parent = NULL,			\
		.ptr = NULL,				\
		.next = NULL				\
	}

/*
 * initialize vfs 
 */
void vfs_init(vfs_node_t *root);

/*
 * switch root file system
 */
void vfs_switch_root(vfs_node_t*);

/*
 * Append a node to the vfs.
 */
int vfs_mknod(vfs_node_t *parent, vfs_node_t *node);

/*
 * Remove a node from the VFS.
 */
int vfs_rmnod(vfs_node_t *node);

/*
 * opens a vfs node
 */
vfs_node_t *vfs_open(const char* filename, unsigned int flags);

/*
 * Reads from a file node
 */
uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);

/*
 * Writes a single byte to a char device.
 */
int vfs_put(vfs_node_t *node, unsigned char c);

/*
 * Writes a null terminated string to a character device.
 */
int vfs_put_s(vfs_node_t *node, char *s);

/*
 * Write to a VFS node.
 */
uint32_t vfs_write(vfs_node_t *node, 
	uint32_t offset, uint32_t size, const unsigned char *buffer);

/*
 * Read a directory node.
 */
struct dirent *vfs_readdir(vfs_node_t *node, uint32_t index, struct dirent *buf);

/*
 * Find a directory in a vfs node.
 */
vfs_node_t *vfs_finddir(vfs_node_t *node, char *name);

/*
 * Get file node information
 */
struct stat *vfs_stat(vfs_node_t *node, const char *path, struct stat *buf);

/*
 * Get the path of the current node.
 */
char *vfs_get_path(vfs_node_t *node, char *buf);

/*
 * Send ioctl request to device.
 */
int vfs_ioctl(vfs_node_t *node, unsigned long request, void *last_arg);

/*
 * Closes a node.
 */
void vfs_close(vfs_node_t *node);

#endif

