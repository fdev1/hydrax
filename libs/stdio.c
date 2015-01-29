#include <arch/platform.h>
#include <arch/stdarg.h>
#include "stdio.h"
#include "unistd.h"


void vprintf(const char *fmt, va_list *args);


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
			write(STDOUT, fmt++, 1);
		}
		else
		{
			switch (fmt[1])
			{
				case 'x' :
					u = (unsigned int) va_arg(*args, unsigned int);
					if (u)
						write(STDOUT, (void*)itox(u), strlen(itox(u)));
					else
						write(STDOUT, (void*)"0", 1);
					fmt += 2;
					break;
				case 'c' :
					c = (char) va_arg(*args, int);
					write(STDOUT, &c, 1);
					break;
					
				case 'i' :
					i = (int) va_arg(*args, int);
					if (i)
						write(STDOUT, (void*)itoa(i), strlen(itoa(i)));
					else
						write(STDOUT, "0", 1);
					fmt += 2;
					break;
				case 's' :
					s = (char*) va_arg(*args, char*);
					while (*s != NULL)
					{
						write(STDOUT, s, 1);
						s++;
					}
					fmt += 2;
					break;
				default:
					write(STDOUT, fmt++, 1);
			}

		}		
	}
}

/*
 * minimal printf
 */
void printf(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
	kvprintf(fmt, &args);
        va_end(args);
}

int getchar(void)
{
	char c;
	read(STDIN, &c, 1);
	return (int) c;
}

int putchar(int c)
{
	char cc = c;
	write(STDOUT, &cc, 1);
	return (int) cc;
}

