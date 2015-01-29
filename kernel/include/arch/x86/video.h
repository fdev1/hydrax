/*
 * include/arch/x86/video.h
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

#include "platform.h"


/*
 *
 */
void arch_video_init(void);

/*
 *
 */
void arch_video_put(char c);

/*
 *
 */
void arch_video_clear();

/*
 *
 */
void arch_video_put_s(const char *c);

#endif
