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
#include <scheduler.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>


static char hostname[HOST_NAME_MAX + 1] = "";


/*
 * Gets the hostname.
 */
int gethostname(char *name, size_t len)
{
	char *p_host;
	if (unlikely(name == NULL))
		return EFAULT;	
	p_host = hostname;
	while (--len > 0 && *p_host != NULL)
		*name++ = *p_host++;
	*name = NULL;
	return ESUCCESS;
}

/*
 * Set the hostname
 */
int sethostname(const char *name, size_t len)
{
	char *p_host;
	if (unlikely(name == NULL))
		return EFAULT;
	if (unlikely(len <= -1 || len > HOST_NAME_MAX))
		return ENAMETOOLONG;
	if (unlikely(current_task->euid != 0 && current_task->egid != 0))
		return EPERM;
	p_host = hostname;
	while (len-- && *name != NULL)
		*p_host++ = *name++;
	*p_host = NULL;
	return ESUCCESS;
}
