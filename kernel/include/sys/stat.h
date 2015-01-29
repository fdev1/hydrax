#ifndef __STAT_H__
#define __STAT_H__

#include <arch/platform.h>

typedef uint32_t dev_t;
typedef uint32_t ino_t;
typedef uint32_t mode_t;
typedef uint32_t nlink_t;
typedef uint32_t off_t;
typedef uint32_t blksize_t;
typedef uint32_t blkcnt_t;

struct stat
{
	dev_t st_dev;		/* Device ID of device containing file. */
	ino_t st_ino;		/* File serial number. */
	mode_t st_mode;		/* Mode of file (see below). */
	nlink_t st_nlink;   	/* Number of hard links to the file. */
	uid_t st_uid;		/* User ID of file. */
	gid_t st_gid;		/* Group ID of file. */

	dev_t st_rdev;		/* Device ID (if file is character or block special). */

	off_t st_size;		/* For regular files, the file size in bytes. 
                     		   For symbolic links, the length in bytes of the 
                     		   pathname contained in the symbolic link. */

	time_t st_atime;	/* Time of last access. */
	time_t st_mtime;	/* Time of last data modification. */
	time_t st_ctime;   	/* Time of last status change. */

	blksize_t st_blksize;	/* A file system-specific preferred I/O block size for 
                     		   this object. In some file system types, this may 
                     		   vary from file to file. */
	blkcnt_t  st_blocks;	/* Number of blocks allocated for this object. */
};

#endif
