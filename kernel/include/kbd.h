/*
 * include/kbd.h
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

#ifndef __KBD_H__
#define __KBD_H__

typedef struct
{
	unsigned int left_shift:1;
	unsigned int right_shift:1;
	unsigned int alt:1;
}
keymask_t;



typedef void (*kbd_handler_t)(char, keymask_t);


void kbd_init(void);
void* kbd_register_handler(kbd_handler_t handler);

#endif

