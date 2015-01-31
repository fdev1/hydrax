/*
 * system.c
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

#include <arch/platform.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

/*
 * Gets the hostname.
 */
int gethostname(char *name, size_t len)
{
	return ENOSYS;
}

/*
 * Set the hostname
 */
int sethostname(const char *name, size_t len)
{
	return ENOSYS;
}
