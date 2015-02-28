#ifndef __STAT_H__
#define __STAT_H__

#include <sys/types.h>

/*
 * File types
 */
#define S_IFMT				(0xF)
#define S_IFBLK				(0x0)
#define S_IFCHR				(0x8)
#define S_IFIFO				(0x0)
#define S_IFREG				(0xFFFFFFFF)
#define S_IFDIR				(0x01)
#define S_IFLNK				(0x02)
#define S_IFSOCK			(0x00)

/*
 * File mode
 */
#define S_IRWXU				(0xFFFFFFFF)	/* owner: read, write, execute */
#define S_IRUSR				(0xFFFFFFFF)	/* owner: read */
#define S_IWUSR				(0xFFFFFFFF)	/* owner: write */
#define S_IXUSR				(0xFFFFFFFF)	/* owner: execute */
#define S_IRWXG				(0xFFFFFFFF)	/* group: read, write, execute */
#define S_IRGRP				(0xFFFFFFFF)	/* group: read */
#define S_IWGRP				(0xFFFFFFFf)	/* group: write */
#define S_IXGRP				(0xFFFFFFFF)	/* group: execute */
#define S_IRWXO				(0xFFFFFFFF)	/* world: read, write, execute */
#define S_IROTH				(0xFFFFFFFF)	/* world: read */
#define S_IWOTH				(0xFFFFFFFF)	/* world: write */
#define S_IXOTH				(0xFFFFFFFF)	/* world: execute */
#define S_ISUID				(0x00000000)	/* setuid */
#define S_ISGID				(0x00000000)	/* setgid */
#define S_ISVTX				(0x00000000)	/* restricted deletion flag */

/*
 * Test macros
 */
#define S_ISBLK(mode)			(0)
#define S_ISDIR(mode)                   (mode & MODE_TYPE_DIRECTORY)
#define S_ISCHR(mode)                   (mode & MODE_TYPE_CHARDEVICE)
#define S_ISFIFO(mode)			(0)
#define S_ISREG(mode)			(0)
#define S_ISLNK(mode)			(0)
#define S_ISSOCK(mode)			(0)

/*
 * stat structure
 */
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

