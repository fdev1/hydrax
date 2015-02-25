#ifndef __STDIO_H__
#define __STDIO_H__

#include <arch/arch.h>
#include <sys/types.h>
#include <stdarg.h>
#include <unistd.h>

#define STDIN			(0)
#define STDOUT 		(1)
#define STDERR 		(2)

/* #define SEEK_SET		(1) */

typedef struct __cfile
{
	void *ptr;
}
FILE;

extern FILE *stderr;

int fflush(FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int fclose(FILE *stream);
FILE *fopen(const char *filename, const char *mode);
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
int fskeek(FILE *stream, long int offset, int origin);
long int ftell(FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
void setbuf(FILE *stream, char *buffer);
int vfprintf ( FILE * stream, const char * format, va_list arg );

int getchar(void);
int putchar(int);


void printf(const char *fmt, ...);


#endif

