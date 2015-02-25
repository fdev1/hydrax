#ifndef ITOA_H
#define ITOA_H

#include <arch/arch.h>
#include <memory.h>

/*
 * itoa
 */
char *itoa(int);

char *itox(unsigned int);

int strcmp(const char *str1, const char *str2);

char *strcpy(char* dest, const char *src);
char *strcat(char *dest, const char *src);

#endif
