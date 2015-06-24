#include <arch/arch.h>
#include <kheap.h>
#include <vfs.h>
#include <printk.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

typedef struct __device
{
	int major;
	int minor;
	vfs_node_t* node;
	struct __device *next;
}
device_t;

static vfs_node_t *dev = NULL;
/* static device_t *devices = NULL; */

/*
 * initialize the devfs "file system"
 */
void devfs_init(void)
{
	int r;
	dev = malloc(sizeof(vfs_node_t));
	if (dev == NULL)
		panic("devfs: out of memory");

	*dev = vfs_node_init(FS_DIRECTORY);
	strcpy(dev->name, "dev");
	dev->length = sizeof(vfs_node_t);
	r = vfs_mknod(NULL, dev);
	if (r)
		panic("devfs: could not create node!");

	printk(7, "devfs: devfs initialized");
}

int devfs_register_device(vfs_node_t* node)
{
	return ENOSYS;
}

/*
 * convenience routine to add a node to /dev
 * This is a temporary development function.
 */
int devfs_mknod(vfs_node_t *node)
{
	return vfs_mknod(dev, node);
}

