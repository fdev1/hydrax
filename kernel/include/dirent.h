/*
 * include/dirent.h
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

#ifndef __DIRENT_H__
#define __DIRENT_H__

#include <stdint.h>

/*
 * TODO: It should be FILENAME_MAX and it goes
 * in stdio.h
 */
#ifndef MAX_FILENAME
#define MAX_FILENAME 255
#endif

typedef int DIR;

/*
 * dirent structure
 */
struct dirent
{
    char name[MAX_FILENAME + 1];
    uint32_t ino;     
    uint32_t index;
};


#endif
