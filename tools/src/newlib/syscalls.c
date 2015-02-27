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
#include <dirent.h>

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

ssyscall0(SYSCALL_TEST, void, syscall_test);
ssyscall0(SYSCALL_SYNC, int, sync);
ssyscall0(SYSCALL_REBOOT, void, reboot);
ssyscall0(SYSCALL_GETTID, int, gettid);
ssyscall0(SYSCALL_GETUID, uid_t, getuid);
ssyscall0(SYSCALL_GETGID, gid_t, getgid);
ssyscall0(SYSCALL_GETEUID, uid_t, geteuid);
ssyscall0(SYSCALL_GETEGID, gid_t, getegid);
ssyscall0(SYSCALL_YIELD, void, yield);
ssyscall0(SYSCALL_VFORK, int, vfork);
ssyscall3(SYSCALL_CHOWN, int, chown, const char*, path, uid_t, uid, gid_t, gid);
ssyscall1(SYSCALL_CLONE, int, clone, void*, stack);
ssyscall1(SYSCALL_DUP, int, dup, int, fd);
ssyscall1(SYSCALL_PIPE, int, pipe, int*, pipefd);
ssyscall2(SYSCALL_DUP2, int, dup2, int, oldfd, int, newfd);
ssyscall3(SYSCALL_READDIR, int, readdir, unsigned int, fd, struct dirent*, dirent, unsigned int, count);
ssyscall1(SYSCALL_TIME, time_t, time, time_t*, t);
ssyscall1(SYSCALL_CHDIR, int, chdir, const char*, path);
ssyscall2(SYSCALL_GETCWD, char*, getcwd, char*, buf, size_t, sz);
ssyscall3(SYSCALL_WAITPID, int, waitpid, pid_t, pid, int*, status, int, options);
ssyscall3(SYSCALL_SETENV, int, setenv, const char*, name, const char*, value, int, overwrite);
ssyscall1(SYSCALL_UNSETENV, int, unsetenv, const char*, name);
ssyscall1(SYSCALL_SETUID, int, setuid, uid_t, uid);
ssyscall1(SYSCALL_SETGID, int, setgid, gid_t, gid);
ssyscall1(SYSCALL_SETEUID, int, seteuid, uid_t, uid);
ssyscall1(SYSCALL_SETEGID, int, setegid, gid_t, gid);
ssyscall1(SYSCALL_CHROOT, int, chroot, const char*, path);
ssyscall1(SYSCALL_FSYNC, int, fsync, int, fd);
ssyscall2(SYSCALL_SETHOSTNAME, int, sethostname, const char*, name, size_t, len);
ssyscall2(SYSCALL_GETHOSTNAME, int, gethostname, char*, name, size_t, len);
ssyscall3(SYSCALL_READLINK, ssize_t, readlink, const char*, path, char*, buf, size_t, bufsize);
ssyscall2(SYSCALL_SIGSET, sighandler_t, sigset, int, sig, sighandler_t, disp);
ssyscall1(SYSCALL_SIGHOLD, int, sighold, int, sig);
ssyscall1(SYSCALL_SIGRELSE, int, sigrelse, int, sig);
ssyscall1(SYSCALL_SIGIGNORE, int, sigignore, int, sig);
ssyscall3(SYSCALL_SIGPROCMASK, int, sigprocmask, int, how, const sigset_t*, set, sigset_t*, oldset);
ssyscall1(SYSCALL_SIGPAUSE, int, sigpause, int, sig);
ssyscall3(SYSCALL_SIGQUEUE, int, sigqueue, pid_t, id, int, sig, const union sigval, value);
ssyscall2(SYSCALL_SIGWAIT, int, sigwait, const sigset_t*, set, int*, sig);
#if 0
ssyscall4(SYSCALL_PTHREAD_CREATE, int, pthread_create, pthread_t*, thread, const pthread_attr_t*, attr, pthread_start_fn, start_routine, void*, arg);
ssyscall2(SYSCALL_PTHREAD_KILL, int, pthread_kill, pthread_t, thread, int, sig);
ssyscall1(SYSCALL_PTHREAD_EXIT, int, pthread_exit, int, status_code);
#endif
ssyscall1(SYSCALL_UNAME, int, uname, struct utsname*, buf);


ssyscall1(SYSCALL_EXIT, void, exit, int, status);
ssyscall1(SYSCALL_CLOSE, int, close, int, fd);
ssyscall3(SYSCALL_EXECVE, int, execve, const char*, path, char *const*, argv, char *const*, envp);
ssyscall0(SYSCALL_FORK, int, fork);
ssyscall0(SYSCALL_GETPID, pid_t, getpid);
ssyscall1(SYSCALL_ISATTY, int, isatty, int, fd);
ssyscall2(SYSCALL_KILL, int, kill, pid_t, pid, int, signum);
ssyscall3(SYSCALL_OPEN, int, sysopen, const char*, path, int, flags, int, mode);
ssyscall3(SYSCALL_READ, ssize_t, read, int, fd, void*, buf, size_t, count);
ssyscall2(SYSCALL_STAT, int, stat, const char*, path, struct stat*, buf);
ssyscall1(SYSCALL_WAIT, pid_t, wait, int*, status);
ssyscall3(SYSCALL_WRITE, ssize_t, write, int, fd, const void*, buf, size_t, count);
ssyscall2(SYSCALL_SIGNAL, sighandler_t, signal, int, sig, sighandler_t, disp);

int open(const char *path, int flags, ...)
{
	return sysopen(path, flags, 0);
}

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


