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
#include "syscalls.h"

ssyscall0(SYSCALL_TEST, void, syscall_test);
ssyscall0(SYSCALL_SYNC, int, sync);
ssyscall0(SYSCALL_REBOOT, void, reboot);
ssyscall0(SYSCALL_GETTID, int, gettid);
ssyscall0(SYSCALL_GETEUID, uid_t, geteuid);
ssyscall0(SYSCALL_GETEGID, gid_t, getegid);
ssyscall0(SYSCALL_YIELD, void, yield);
ssyscall0(SYSCALL_VFORK, int, vfork);
ssyscall3(SYSCALL_CHOWN, int, chown, const char*, path, uid_t, uid, gid_t, gid);
ssyscall1(SYSCALL_CLONE, int, clone, void*, stack);
ssyscall1(SYSCALL_TIME, time_t, time, time_t*, t);
ssyscall3(SYSCALL_SETENV, int, setenv, const char*, name, const char*, value, int, overwrite);
ssyscall1(SYSCALL_UNSETENV, int, unsetenv, const char*, name);
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


ssyscall1(SYSCALL_ISATTY, int, isatty, int, fd);
ssyscall1(SYSCALL_WAIT, pid_t, wait, int*, status);

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


