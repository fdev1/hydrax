/*
 * include/console.h
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

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <arch/platform.h>


/*
 *
 */
void console_init(void);
void console_early_init(void);
/*
 *
 */
void video_put(char c);

/*
 *
 */
void video_clear();

/*
 *
 */
void video_put_s(const char *c);

#endif
