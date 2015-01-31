/*
 * include/string.h
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

#ifndef __STRING_H__
#define __STRING_H__

#include <arch/arch.h>

/*
 * itoa
 */
char *itoa(int);

char *itox(int);

int strcmp(const char *str1, const char *str2);

char *strcpy(char* dest, const char *src);
char *strcat(char *dest, const char *src);
unsigned int strlen(const char *s);

int atoi(const char *s);

#endif
