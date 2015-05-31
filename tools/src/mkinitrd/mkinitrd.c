#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(__APPLE_CC__)
/*#	include <sys/malloc.h>*/
#else
#	include <malloc.h>
#endif
#include <assert.h>

#define MAX_FILENAME	(255)
#define MAX_PATH		(255)

typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned char uint8_t;
//typedef char uint8_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
/*typedef uint32_t intptr_t;*/

#define intptr_t uint32_t

#if defined(NULL)
#undef NULL
#endif
#define NULL		(0)

typedef struct __mutex_t
{
	int a;
	int b;
	void *c;
}
mutex_t;

#define MUTEX_INITIALIZER	{0,0}

/*
 * Semaphore structure
 */
typedef struct __semaphore
{
	unsigned int cnt;
	unsigned int max_cnt;
	unsigned int enter;
	mutex_t exclusive;
	unsigned int waiters_lock;
	void *waiters;
}
semaphore_t;

void exit(int);

/*
 * ====================================================
 */


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
		.chown = NULL,				\
		.parent = NULL,			\
		.ptr = NULL,				\
		.next = NULL				\
	}


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
typedef int (*vfs_chown_fn)(struct vfs_node *node, uid_t uid, gid_t gid);


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
	vfs_chown_fn chown;
	struct vfs_node *parent;
	struct vfs_node *ptr;
	struct vfs_node *next;		/* next sibling */
} 
vfs_node_t;

typedef struct __initrd_header
{
	uint32_t magic;
	uint32_t sz;
}
initrd_header_t;

vfs_node_t *root = NULL;
vfs_node_t *p_node = NULL;

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

	s = buf;
	if (s == NULL)
		return NULL;
	for (i = 0; i < sz; i++)
		s[i] = *start++;
	s[i] = NULL;	
	return s;		
}



int find_parent(char *path, vfs_node_t **node)
{
	int i, c, levels;
	char p[MAX_PATH + 1];
	char *p_path;
	vfs_node_t *tmp, **ptmp;
	
	levels = 0;
	do
		p_path = vfs_get_path_level(path, levels++, p);
	while (p_path != NULL);
	levels--;
	
	i = 0;
	c = 0;
	p_path = vfs_get_path_level(path, i++, p);
	*node = root;
	while (p_path != NULL && *p_path != NULL && i < levels)
	{		
		tmp = (*node)->ptr;
		while (tmp != NULL)
		{
			if (!strcmp(tmp->name, p_path))
				break;
			tmp = tmp->next;
		}
		if (tmp == NULL)
		{
			tmp = p_node++;
			*tmp = VFS_NODE_INITIALIZER(FS_DIRECTORY);
			strcpy(tmp->name, p_path);
			tmp->parent = *node;
			ptmp = &(*node)->ptr;
			while (*ptmp != NULL)
				ptmp = &(*ptmp)->next;
			*ptmp = tmp;
			c++;
		}
		
		*node = tmp;		
		p_path = vfs_get_path_level(path, i++, p);
	}
	return c;
}

char *filename(const char *path)
{
	char *p;
	p = (char*) path;
	while (*p != NULL)
		p++;
	while (p > path && *(p - 1) != '/')
		p--;
	return p;	
}

int main(int argc, char **argv)
{
	int i, j;
	int nodes_count;
	int offset;
	char **args;
	vfs_node_t *node, *new_node, **tmp;
	
	initrd_header_t header;
	header.magic = 0x42424242;
	header.sz = (argc - 1) / 2;
	
	/*
	 * TODO: Right now we're allocating enough memory 
	 * for 5 nodes for each entry. We need to calculate
	 * how many nodes we'll need before time.
	 */
	p_node = (vfs_node_t*) malloc(sizeof(vfs_node_t) * header.sz * 5);
	if (p_node == 0)
	{
		printf("mkinitrd: out of memory!!!\n");
		return -1;
	}
	
	root = p_node++;
	*root = VFS_NODE_INITIALIZER(FS_DIRECTORY);
	root->data = NULL;
	root->gid = 0;
	root->uid = 0;
	root->length = 0;
	
	offset = 0;
	nodes_count = 1;	
	args = &argv[1];
	for (i = 0; i < header.sz; i++)
	{
		char *path, *initrd_path;
		FILE *stream;
		path = *args++;
		initrd_path = *args++;
		nodes_count += find_parent(initrd_path, &node);
		assert(node != NULL);
		stream = fopen(path, "r");
		if(stream == 0)
		{
			printf("Error: file not found: %s\n", path);
			return -1;
		}
		
		fseek(stream, 0, SEEK_END);
		new_node = p_node++;
		*new_node = VFS_NODE_INITIALIZER(FS_FILE);
		strcpy(new_node->name, filename(initrd_path));
		new_node->length = ftell(stream);
		new_node->data = (void*) offset;
		new_node->parent = node;
		offset += new_node->length;
		fclose(stream);
		
		tmp = &node->ptr;
		while (*tmp != NULL)
			tmp = &(*tmp)->next;
		*tmp = new_node;
		nodes_count++;
		
		printf("Processing %s...\n", initrd_path);
		
	}
	
	new_node = root;
	offset = sizeof(initrd_header_t) + (sizeof(vfs_node_t) * nodes_count);
	for (i = 0; i < nodes_count; i++)
	{
		if (new_node->length != 0)
		{
			new_node->data = (void*) (((uint32_t) new_node->data) + offset);
		}
		if (new_node->ptr != NULL)
		{
			new_node->ptr = (vfs_node_t*) 
				((((uint32_t) new_node->ptr) - (uint32_t) root) + 
				sizeof(initrd_header_t));
		}
		if (new_node->parent != NULL)
		{
			new_node->parent = (vfs_node_t*) 
				((((uint32_t) new_node->parent) - (uint32_t) root) + 
				sizeof(initrd_header_t));
		}
		if (new_node->next != NULL)
		{
			new_node->next = (vfs_node_t*) 
				((((uint32_t) new_node->next) - (uint32_t) root) + 
				sizeof(initrd_header_t));			
		}
		new_node++;
	}

	j = header.sz;
	header.sz = nodes_count;
	FILE *wstream = fopen("./initrd.img", "wb");
	fwrite(&header, sizeof(initrd_header_t), 1, wstream);
	fwrite(root, sizeof(vfs_node_t), nodes_count, wstream);
	
	for(i = 0; i < j; i++)
	{
		FILE *stream = fopen(argv[i * 2 + 1], "rb");
		fseek(stream, 0, SEEK_END);
		offset = ftell(stream);
		rewind(stream);
		unsigned char *buf = (unsigned char *) malloc(offset);
		fread(buf, 1, offset, stream);
		fwrite(buf, 1, offset, wstream);
		fclose(stream);
		free(buf);
	}

	fclose(wstream);

}
