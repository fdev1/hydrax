#include <arch/platform.h>
#include <kheap.h>
#include <vfs.h>
#include <printk.h>
#include <string.h>

static vfs_node_t *dev;

/*
 * initialize the devfs "file system"
 */
void devfs_init(void)
{
	int r;
	dev = malloc(sizeof(vfs_node_t));
	if (dev == NULL)
		panic("devfs_init: out of memory");

	*dev = vfs_node_init(FS_DIRECTORY);
	strcpy(dev->name, "dev");
	dev->length = sizeof(vfs_node_t);
	r = vfs_mknod(NULL, dev);
	if (r)
		panic("devfs: could not create node!");

	printk(7, "devfs: devfs initialized");
}

/*
 * convenience routine to add a node to /dev
 */
int devfs_mknod(vfs_node_t *node)
{
	return vfs_mknod(dev, node);
}

