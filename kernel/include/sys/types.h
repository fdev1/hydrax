#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <stdint.h>

typedef unsigned long __size_t;
typedef long __ssize_t;

typedef __size_t size_t;
typedef __ssize_t ssize_t;
typedef long suseconds_t;

typedef int32_t pid_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef uint32_t time_t;
typedef uint32_t dev_t;
typedef uint32_t ino_t;
typedef uint32_t mode_t;
typedef uint32_t nlink_t;
typedef long off_t;	/* newlib defines it as long */
typedef uint32_t blksize_t;
typedef uint32_t blkcnt_t;
typedef unsigned long clock_t;
typedef pid_t pthread_t;

/*
 * This are not part of any standard but newlib
 * uses them so we define them here.
 */
typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned long u_long;

#endif
