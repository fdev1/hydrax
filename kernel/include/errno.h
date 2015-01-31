/*
 * include/errno.h
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

#ifndef __ERRNO_H__
#define __ERRNO_H__

#define ESUCCESS		(0)
#define EACCES			(-2)
#define EBADF			(-3)
#define EFAULT			(-4)
#define ELOOP			(-5)	/* too many simbolic links */
#define ENAMETOOLONG	(-6)
#define ENOENT			(-7)
#define ENOMEM			(-8)
#define ENOTDIR		(-9)
#define EOVERFLOW		(-10)
#define ENOSYS			(-12)
#define ECHILD			(-13)
#define EINVAL			(-14)
#define EINTR			(-15)
#define EPERM			(-16)

#endif
