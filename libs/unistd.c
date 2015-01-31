#include <arch/platform.h>
#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

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
		"int $0x30;" : : "a" (num), "b" (argb), "c" (argc), "d" (argd), "e" (arge))
		
		
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
rettype name(arg1type arg1, arg2type arg2, arg3type arg3, arg4type, arg4)					\
	__attribute__((weak, alias("__" #name)))


ssyscall0(SYSCALL_TEST, void, syscall_test);
ssyscall0(SYSCALL_SYNC, int, sync);
ssyscall0(SYSCALL_REBOOT, void, reboot);
ssyscall0(SYSCALL_GETPID, pid_t, getpid);
ssyscall0(SYSCALL_FORK, int, fork);
ssyscall0(SYSCALL_GETTID, int, gettid);
ssyscall0(SYSCALL_GETUID, uid_t, getuid);
ssyscall0(SYSCALL_GETGID, gid_t, getgid);
ssyscall0(SYSCALL_GETEUID, uid_t, geteuid);
ssyscall0(SYSCALL_GETEGID, gid_t, getegid);
ssyscall0(SYSCALL_YIELD, void, yield);
ssyscall0(SYSCALL_VFORK, int, vfork);
ssyscall3(SYSCALL_CHOWN, int, chown, const char*, path, uid_t, uid, gid_t, gid);
ssyscall3(SYSCALL_WRITE, size_t, write, int, fd, const void*, buf, size_t, count);
ssyscall3(SYSCALL_READ, size_t, read, int, fd, void*, buf, size_t, count);
ssyscall1(SYSCALL_CLOSE, int, close, int, fd);
ssyscall1(SYSCALL_EXIT, int, exit, int, status);
ssyscall1(SYSCALL_CLONE, int, clone, void*, stack);
ssyscall1(SYSCALL_DUP, int, dup, int, fd);
ssyscall1(SYSCALL_PIPE, int, pipe, int, pipefd[2]);
ssyscall2(SYSCALL_DUP2, int, dup2, int, oldfd, int, newfd);
ssyscall2(SYSCALL_OPEN, int, open, const char*, path, int, flags);
ssyscall2(SYSCALL_KILL, int, kill, pid_t, pid, int, signum);
ssyscall3(SYSCALL_READDIR, int, readdir, unsigned int, fd, struct dirent*, dirent, unsigned int, count);
ssyscall2(SYSCALL_STAT, int, stat, const char*, path, struct stat*, buf);
ssyscall1(SYSCALL_TIME, time_t, time, time_t*, t);
ssyscall1(SYSCALL_CHDIR, int, chdir, const char*, path);
ssyscall2(SYSCALL_GETCWD, char*, getcwd, char*, buf, size_t, sz);
ssyscall3(SYSCALL_EXECVE, int, execve, const char*, path, char *const*, argv, char *const*, envp);
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
ssyscall1(SYSCALL_WAIT, pid_t, wait, int*, status);
ssyscall3(SYSCALL_READLINK, ssize_t, readlink, const char*, path, char*, buf, size_t, bufsize);

int ioctl(int fd, unsigned int request, ...)
{
	void *arg;
	va_list args;
	va_start(args, request);
	arg = (void*) va_arg(args, unsigned int);
	syscall3(SYSCALL_IOCTL, fd, request, arg);
}

int fcntl(int fd, int cmd, ...)
{
	return ENOSYS;
}
