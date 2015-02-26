#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

/*
 * System call asm macros
 */
#define syscall0(num)			\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num))
#define syscall1(num, argb)		\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num), "b" (argb))
#define syscall2(num, argb, argc)	\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num), "b" (argb), "c" (argc))
#define syscall3(num, argb, argc, argd)	\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num), "b" (argb), "c" (argc), "d" (argd))
#define syscall4(num, argb, argc, argd, arge)	\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num), "b" (argb), "c" (argc), "d" (argd), "S" (arge))
		
		
#define ssyscall0(num, rettype, name)		\
rettype __##name(void)					\
{									\
	syscall0(num);						\
}									\
rettype name(void) __attribute__((weak, alias("__" #name)))

#define ssyscall1(num, rettype, name, arg1type, arg1)		\
rettype __##name(arg1type arg1)						\
{												\
	syscall1(num, arg1);							\
}												\
rettype name(arg1type arg1) __attribute__((weak, alias("__" #name)))

#define ssyscall2(num, rettype, name, arg1type, arg1, arg2type, arg2)	\
rettype __##name(arg1type arg1, arg2type arg2)				\
{														\
	syscall2(num, arg1, arg2);								\
}														\
rettype name(arg1type arg1, arg2type arg2) __attribute__((weak, alias("__" #name)))

#define ssyscall3(num, rettype, name, arg1type, arg1, arg2type, arg2, arg3type, arg3)	\
rettype __##name(arg1type arg1, arg2type arg2, arg3type arg3)				\
{																\
	syscall3(num, arg1, arg2, arg3);									\
}																\
rettype name(arg1type arg1, arg2type arg2, arg3type arg3)					\
	__attribute__((weak, alias("__" #name)))

#define ssyscall4(num, rettype, name, arg1type, arg1, arg2type, arg2, arg3type, arg3, arg4type, arg4)	\
rettype __##name(arg1type arg1, arg2type arg2, arg3type arg3, arg4type arg4)				\
{																\
	syscall4(num, arg1, arg2, arg3, arg4);									\
}																\
rettype name(arg1type arg1, arg2type arg2, arg3type arg3, arg4type arg4)					\
	__attribute__((weak, alias("__" #name)))

char **environ;

ssyscall1(SYSCALL_EXIT, int, exit, int, status);
ssyscall1(SYSCALL_CLOSE, int, close, int, fd);
ssyscall3(SYSCALL_EXECVE, int, execve, const char*, path, char *const*, argv, char *const*, envp);
ssyscall0(SYSCALL_FORK, int, fork);
ssyscall0(SYSCALL_GETPID, pid_t, getpid);
ssyscall1(SYSCALL_ISATTY, int, isatty, int, fd);
ssyscall2(SYSCALL_KILL, int, kill, pid_t, pid, int, signum);
ssyscall2(SYSCALL_OPEN, int, open, const char*, path, int, flags);
ssyscall3(SYSCALL_READ, ssize_t, read, int, fd, void*, buf, size_t, count);
ssyscall2(SYSCALL_STAT, int, stat, const char*, path, struct stat*, buf);
ssyscall1(SYSCALL_WAIT, pid_t, wait, int*, status);
ssyscall3(SYSCALL_WRITE, ssize_t, write, int, fd, const void*, buf, size_t, count);
ssyscall2(SYSCALL_SIGNAL, sighandler_t, signal, int, sig, sighandler_t, disp);

int gettimeofday(struct timeval *p, void *z)
{
	return -1;
}

clock_t times(struct tms *buf)
{
	return -1;
}

void* sbrk(ptrdiff_t incr)
{
	return (void*) -1;
}

off_t lseek(int fd, off_t offset, int whence)
{
	return -1;
}

int link(const char *old, const char *new)
{
	return -1;
}

int unlink(const char *name)
{
	return -1;
}

int fstat(int fd, struct stat *st)
{
	return -1;
}

void _exit(int status)
{
	exit(0);
}


