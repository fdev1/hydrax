/*
 * include/time.h
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

#ifndef __TIME_H__
#define __TIME_H__

#include <sys/types.h>

struct tm
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;		
};

struct timespec
{
	time_t tv_sec;		/* seconds */
	long tv_nsec;		/* nanoseconds */
};

time_t time(time_t *t);

#endif

