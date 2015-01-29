/*
 * include/arch/x86/stdarg.h
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

#ifndef __STDARG_H__
#define __STDARG_H__

//struct va_list
//{
//	unsigned char *ptr;	
//};

//#define va_start(list, last_arg)	list.ptr = (unsigned char*) &last_arg + sizeof(last_arg)
//#define va_arg(list, type)		*(type*)((list.ptr += sizeof(type)) - sizeof(type))

//#define va_arg(list, type)    (*(type *)((list += sizeof(type)) - sizeof(type)))

typedef unsigned char *va_list;
#define va_start(list, param) (list = (((va_list)&param) + sizeof(param)))
#define va_arg(list, type)    (*(type *)((list += sizeof(type)) - sizeof(type)))
#define va_end(list)
#endif

