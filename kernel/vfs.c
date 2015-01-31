/*
 * vfs.c
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

#include <vfs.h>
#include <printk.h>
#include <unistd.h>
#include <string.h>
#include <kheap.h>
#include <scheduler.h>
#include <errno.h>

static vfs_node_t *vfs_root = NULL;
static vfs_node_t *vfs_root_mount = NULL;
static unsigned int vfs_root_nodes = 0;

/*
 * gets a component of a path
 */
static char* vfs_get_path_level(
	const char *filename, unsigned int level, char *buf)
{
	unsigned int i;
	size_t sz;
	const char *start, *end;
	char *s;

	assert(strlen(filename) <= MAX_PATH);

	if (*filename == '/')
	{
		filename++;
	}
	start = filename;
	while (level > 0)
	{
		while (*filename != '/' && *filename != NULL)
			filename++;

		if (*filename == NULL)
			return NULL;
		start = ++filename;
		level--;
	}
	while (*filename != '/' && *filename != NULL)
		filename++;
	
	sz = (intptr_t) filename - (intptr_t) start;
	if (sz == 0)
		return NULL;

	//s = (char*) malloc(sizeof(char) * sz);
	s = buf;
	if (s == NULL)
		return NULL;
	for (i = 0; i < sz; i++)
		s[i] = *start++;
	s[i] = NULL;	
	return s;		
}

const char *get_path_from_level(const char *path, int level)
{
	const char *p;
	p = path;
	if (*p == '/')
		p++;

	while (level > 0)
	{
		while (*p != NULL && *p != '/')
			p++;
		if (*p == NULL)
			break;
		p++;
		if (*p == NULL)
			break;
		level--;
	}

	return p;
}

/*
 * initialize the vfs
 */
void vfs_init(vfs_node_t *root)
{
	vfs_root = (vfs_node_t*) malloc(sizeof(vfs_node_t));
	if (root == NULL)
		panic("vfs: out of memory");
	strcpy(vfs_root->name, "root");
	*vfs_root = vfs_node_init(FS_DIRECTORY);
	assert(vfs_root->flags & FS_DIRECTORY);
	printk(7, "vfs: virtual file system ready.");
	vfs_switch_root(root);
}

/*
 * changes the root filesystem
 */
void vfs_switch_root(vfs_node_t *newroot)
{
	vfs_root_mount = newroot;
}

/*
 * attaches a node
 */
int vfs_mknod(vfs_node_t *parent, vfs_node_t *node)
{
	vfs_node_t *tmp;
	assert(node != NULL);
	assert(node->next == NULL);

	if (parent == NULL)
		parent = vfs_root;
	if (!(parent->flags & FS_DIRECTORY))
		return -1;

	mutex_busywait(&parent->lock);
	node->parent = parent;
	if (parent->ptr == NULL)
	{
		parent->ptr = node;
		if (parent == vfs_root)
			vfs_root_nodes++;
	}
	else
	{
		tmp = parent->ptr;
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = node;
		if (parent == vfs_root)
			vfs_root_nodes++;
	}
	mutex_release(&parent->lock);	
	return 0;
}

/*
 * Remove a node from the vfs.
 */
int vfs_rmnod(vfs_node_t *node)
{
	vfs_node_t *tmp, *sibling;
	assert(node != NULL);
	assert(node->parent != NULL);

	if (node->parent->ptr == NULL)
		return -1;

	mutex_busywait(&node->parent->lock);
	tmp = node->parent->ptr;
	sibling = NULL;
	while (tmp != NULL && tmp != node)
	{
		sibling = tmp;
		tmp = tmp->next;
	}

	if (tmp == NULL)
	{
		mutex_release(&node->parent->lock);
		return -1;
	}

	if (sibling == NULL)
		node->parent->ptr = node->next;
	else
		sibling->next = node->next;
	mutex_release(&node->parent->lock);
	node->next = NULL;
	return 0;
}

/*
 * reads from a file
 */
uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
	assert(node != NULL);
	assert(buffer != NULL);
	assert(size > 0);
	if (node == NULL)
		node = vfs_root;
	if (node->read != 0)
		return node->read(node, offset, size, buffer);
	else
		return 0;
}

/*
 * writes a character to a CHAR_DEVICE node
 */
int vfs_put(vfs_node_t *node, unsigned char c)
{
	assert(node != NULL);
	assert(node->flags & FS_CHARDEVICE);
	if (node->write != NULL)
		return node->write(node, NULL, 1, &c);
	else 
		return 0;
}

/*
 * writes a null-terminated string to a CHAR_DEVICE node.
 */
int vfs_put_s(vfs_node_t *node, char *s)
{
	assert(node != NULL);
	assert(node->flags & FS_CHARDEVICE);
	if (node->write != NULL)
		return node->write(node, NULL, strlen(s) + 1, (const uint8_t*) s);
	else
		return 0;
}

/*
 * writes to a file
 */
uint32_t vfs_write(vfs_node_t *node, 
	uint32_t offset, uint32_t size, const unsigned char *buffer)
{
	assert(node != NULL);
	assert(buffer != NULL);
	if (node->write != 0)
		return node->write(node, offset, size, buffer);
	else
		return 0;
}

/*
 * Open a device node by it's major and minor numbers.
 * If minor equals zero then the first device with that
 * major and minor number is returned.
 */
vfs_node_t *vfs_opendev(const uint32_t major, const uint32_t minor)
{
	current_task->errno = ENOSYS;
	return NULL;
}

/*
 * Open a VFS node.
 */
vfs_node_t *vfs_open(const char *filename, unsigned int flags)
{
	unsigned int i;
	char *name;
	vfs_node_t *tmp, *node;
	char *buf;
	assert(filename != NULL);
	assert(strlen(filename) <= MAX_PATH);
	i = 0;
	node = (*filename == '/') ? vfs_root : get_current_task()->cwd;
	if (node == NULL)
		node = vfs_root;
	buf = malloc(sizeof(char) * (MAX_PATH + 1));
	if (buf == NULL)
	{
		get_current_task()->errno = ENOMEM;
		return NULL;
	}
	
	name = vfs_get_path_level(filename, i++, buf);
	if (name == NULL || !strcmp("", name))
	{
		free(buf);
		mutex_wait(&vfs_root->lock);
		vfs_root->refs++;
		mutex_release(&vfs_root->lock);
		return vfs_root;
	}

	do
	{
		while (1)
		{
			#if 0 && (CHAR_BIT == 8)
			const uint16_t dotdot = (uint16_t) '.' || (uint16_t) '.' << 8;
			if (*((uint16_t*) name) == dotdot && name[2] == NULL)
			#else
			if (name[0] == '.' && name[1] == '.' && name[2] == NULL)
			#endif
			{
				if (node == vfs_root)
				{
					free(buf);
					current_task->errno = ENOENT;
					return NULL;
				}
				else
				{
					assert(node->parent != NULL);
					node = node->parent;
				}
			}
			else if (!(name[0] == '.' && name[1] == NULL))
			{
				break;
			}
			name = vfs_get_path_level(filename, i++, buf);
			if (name == NULL || !strcmp("", name))
			{
				free(buf);
				mutex_wait(&node->lock);
				node->refs++;
				mutex_release(&node->lock);
				return node;
			}
		}
		
		if (node->open != NULL)
		{
			tmp = node->open(node, name, NULL);
		}
		else
		{
			tmp = vfs_finddir(node, name);
		}
		if (tmp == NULL)
		{
			current_task->errno = ENOENT;
			free(buf);
			mutex_wait(&node->lock);
			node->refs++;
			mutex_release(&node->lock);
			return NULL;
		}
		node = tmp;
	}
	while ((name = vfs_get_path_level(filename, i++, buf)) != NULL);
	free(buf);
	mutex_wait(&node->lock);
	node->refs++;
	mutex_release(&node->lock);
	return node;
}

/*
 * Close a VFS node
 */
void vfs_close(vfs_node_t *node)
{
	assert(node != NULL);
	mutex_wait(&node->lock);
	if (--node->refs == 0 && node->flags & FS_AUTOFREE)
	{
		if (node->flags & FS_PIPE)
		{
			pipe_t *pipe;
			pipe = (pipe_t*) node->data;
			if (pipe != NULL)
			{
				assert(pipe->buf != NULL);
				free(pipe->buf);
				free(pipe);
			}
			
		}
		free(node);
		return;
	}	
	mutex_release(&node->lock);
	node = NULL;
}

struct dirent *vfs_readdir(vfs_node_t *node, uint32_t index, struct dirent *buf)
{
	assert(buf != NULL);

	if (node == NULL)
		node = vfs_root;

	/*
	 * for the root filesystem we look on the vfs first
	 * and then we look on the mounted volume
	 */
	if (node->readdir == NULL)
	{
		if (node != vfs_root || index < vfs_root_nodes)
		{
			unsigned int i;

			if (node->ptr == NULL)
				return NULL;

			vfs_node_t *tmp = node->ptr;
			for (i = 0; i < index && tmp->next != NULL; i++)
				tmp = (vfs_node_t*) tmp->next;

			if (tmp == NULL || i < index)
				return NULL;
		
			strcpy(buf->name, tmp->name);
			buf->ino = tmp->inode;
			return buf;
		}
		else
		{
			unsigned int i;
			node = vfs_root_mount;
			if (node->ptr == NULL)
				return NULL;

			vfs_node_t *tmp = node->ptr;
			for (i = vfs_root_nodes; i < index && tmp->next != NULL; i++)
				tmp = (vfs_node_t*) tmp->next;

			if (tmp == NULL || i < index)
				return NULL;
		
			strcpy(buf->name, tmp->name);
			buf->ino = tmp->inode;
			return buf;			
		}
	}
	else
	{
		return node->readdir(node, index, buf);
	}
}

/*
 * find a node in a directory
 */
vfs_node_t *vfs_finddir(vfs_node_t *node, char *name)
{
	if (node == NULL)
		node = vfs_root;

	if ((node->flags & 0x7) == FS_DIRECTORY && node->finddir == NULL)
	{
		/*
		 * we look on the vfs first and then we look
		 * on the mounted filesystem
		 */
		vfs_node_t *tmp;
		if (node->ptr != NULL)
		{
		
			tmp = node->ptr;
			while (tmp != NULL && strcmp(tmp->name, name))
				tmp = tmp->next;

			if (tmp != NULL)
				return tmp;
		}

		if (node == vfs_root)
		{
			tmp = vfs_root_mount->ptr;
			while (tmp != NULL && strcmp(tmp->name, name))
				tmp = tmp->next;

			if (tmp != NULL)
				return tmp;
		}

		return NULL;
	}
	else
	{
		return node->finddir(node, name);
	}
}

/*
 * Get info about a file.
 *
 * If the path is NULL this function stats the node directly,
 * otherwise it stats the path in the node.
 */
struct stat *vfs_stat(vfs_node_t *node, const char *path, struct stat *buf)
{
	if (node != NULL && node->stat != NULL)
	{
		return node->stat(node, path, buf);
	}
	else
	{
		vfs_node_t *stat_node;

		if (path == NULL)
		{
			stat_node = node;
		}
		else
		{

			unsigned int i;
			char *name, *buff;
			vfs_node_t *tmp, *node;

			i = 0;
			node = vfs_root;
			
			buff = malloc(sizeof(char) * (MAX_PATH + 1));
			if (buff == NULL)
			{
				get_current_task()->errno = ENOMEM;
				return NULL;
			}
			
			name = vfs_get_path_level(path, i++, buff);
			if (name == NULL || !strcmp("", name))
				stat_node = vfs_root;
			do
			{				
				if (node->stat != NULL)
				{
					free(buff);
					return node->stat(node, name, buf);
				}
				else
				{
					tmp = vfs_finddir(node, name);
				}
				if (tmp == NULL)
				{
					free(buff);
					get_current_task()->errno = ENOENT;
					return NULL;
				}
				node = tmp;
			}
			while ((name = vfs_get_path_level(path, i++, buff)) != NULL);
			
			free(buff);			
			stat_node = node;
		}
		
        	if (stat_node == NULL)
		{
			get_current_task()->errno = ENOENT;
			return NULL;
		}

		buf->st_dev = 0;
		buf->st_ino = 0;
		buf->st_mode = 0;
		buf->st_nlink = 0;
		buf->st_uid = stat_node->uid;
		buf->st_gid = stat_node->gid;
		buf->st_rdev = 0;
		buf->st_size = stat_node->length;
		buf->st_atime = 0;
		buf->st_mtime = 0;
		buf->st_ctime = 0;
		buf->st_blksize = 0;
		buf->st_blocks = 0;

		if ((stat_node->flags & 0x7) == FS_DIRECTORY)
			buf->st_mode |= MODE_TYPE_DIRECTORY;
		if ((stat_node->flags & 0x7) == FS_CHARDEVICE)
			buf->st_mode |= MODE_TYPE_CHARDEVICE;
		return buf;
	}
}

/*
 * Gets the full path to a node.
 */
char *vfs_get_path(vfs_node_t *node, char *buf)
{
	vfs_node_t *nodes[MAX_PATH_NODES], **p_node;
	char *out, *name;
	assert(node != NULL);
	if (node == vfs_root)
	{
		buf[0] = '/';
		buf[1] = NULL;
		return buf;
	}
	out = buf;
	p_node = nodes;
	do
	{
		*p_node++ = node;
		node = node->parent;
		assert(node != NULL);
	}
	while (node != vfs_root);	
	p_node--;
	while (p_node >= nodes)
	{
		name = (*p_node--)->name;
		*out++ = '/';
		while (*name != NULL)
			*out++ = *name++;
	}
	*out = NULL;
	return buf;
}

/*
 * Send IOCTL request to device.
 */
int vfs_ioctl(vfs_node_t *node, unsigned int request, void *last_arg)
{
	assert(node != NULL);
	if (node->ioctl != NULL)
		return node->ioctl(node, request, last_arg);
	return ENOENT;
}

