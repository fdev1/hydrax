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

#ifndef __xxERRNO_H__
#define __xxERRNO_H__

extern int errno;

#define ESUCCESS		(0)
#define EACCES			(-2)
#define EBADF			(-3)
#define EFAULT			(-4)
#define ELOOP			(-5)	/* too many simbolic links */
#define ENAMETOOLONG		(-6)
#define ENOENT			(-7)
#define ENOMEM			(-8)
#define ENOTDIR			(-9)
#define EOVERFLOW		(-10)
#define ENOSYS			(-12)
#define ECHILD			(-13)
#define EINVAL			(-14)
#define EINTR			(-15)
#define EPERM			(-16)
#define ESRCH			(-17)
#define EAGAIN			(-18)
#define EDOM			( 33)
#define ERANGE			( 34)
#define EILSEQ			(-20)
#define EFTYPE			(-21)
#define ESPIPE			(-22)
#define EEXIST			(-23)
#define ENOSPC			(-24)
#define EFBIG			(-25)

#endif

