/*
 * printk.c
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
 */

#include <arch/arch.h>
#include <arch/stdarg.h>
#include <mutex.h>
#include <console.h>
#include <printk.h>
#include <string.h>
#include <timer.h>
#include <vfs.h>

extern vfs_node_t console;

void kvprintf(const char *fmt, va_list *args);

/*
 * printk - write a string to the screen.
 *
 */
void printk(const unsigned int level, const char *fmt, ...)
{
	static mutex_t s = MUTEX_INITIALIZER;
	va_list args;
	
	va_start(args, fmt);	
	mutex_busywait(&s);
	kprintf("\r");
	kprintf("[%i] ", timer_getticks());
	kvprintf(fmt, &args);
	kprintf("\n");
	mutex_release(&s);
	va_end(args);
}

/*
 * deprecated.
 */ 
void printk_int(const char* str, int arg)
{
	kprintf(str, arg);
}

static uint32_t printk_delay_spin = 0;

/*
 * slow down printk
 */
void printk_delay(const uint32_t delay)
{
	printk_delay_spin = delay;
}

/*
 * minimal printf
 */
void kvprintf(const char *fmt, va_list *args)
{
	int i;
	unsigned int u;
	const char* s;
	char c;

	while (*fmt != 0)
	{
		if (*fmt != '%')
		{
			vfs_write(&console, 0, 1, fmt++);
		}
		else
		{
			switch (fmt[1])
			{
				case 'x' :
					u = (unsigned int) va_arg(*args, unsigned int);
					if (u)
					{
						/* TODO: Use vfs_write_s instead */
						char *s;
						s = itox(u);
						vfs_write(&console, 0, strlen(s), s);
					}
					else
					{
						c = '0';
						vfs_write(&console, 0, 1, &c);
					}
					fmt += 2;
					break;
				case 'c' :
					c = (char) va_arg(*args, int);
					vfs_write(&console, 0, 1, &c);
					break;
					
				case 'i' :
					i = (int) va_arg(*args, int);
					if (i)
					{
						char *s;
						s = itoa(i);
						vfs_write(&console, 0, strlen(s), s);
					}
					else
					{
						c = '0';
						vfs_write(&console, 0, 1, &c);
					}
					fmt += 2;
					break;
				case 's' :
					s = (char*) va_arg(*args, char*);
					vfs_write(&console, 0, strlen(s), s);
					fmt += 2;
					break;
				default:
					vfs_write(&console, 0, 1, fmt++);
			}

		}		
	}

	/*
	 * spin the cpu a little to slow tings down
	 */
	for (i = 0; i < printk_delay_spin; i++);
}

/*
 * minimal printf
 */
void kprintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	kvprintf(fmt, &args);
	va_end(args);
}


