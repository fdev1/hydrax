#include <arch/platform.h>
#include <unistd.h>
#include <syscall.h>
#include "stdio.h"

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



void syscall_test(void)
{
	syscall0(SYSCALL_TEST);
}
		
/*
 *
 */
int __open(const char *pathname, int flags)
{
	syscall2(SYSCALL_OPEN, pathname, flags);
}
int open(const char *pathname, int flags) 
	__attribute__((weak, alias("__open")));

/*
 * Close a file handle
 */
int close(int fd)
{
	syscall1(SYSCALL_CLOSE, fd);
}

/*
 *
 */
size_t write(int fd, const void *buf, size_t count)
{
	syscall3(SYSCALL_WRITE, fd, buf, count);
}

size_t read(int fd, void *buf, size_t count)
{
	syscall3(SYSCALL_READ, fd, buf, count);
}

int kill(int pid, int signum)
{
	syscall2(SYSCALL_KILL, pid, signum);
}

int exit(int exit_code)
{
	syscall1(SYSCALL_EXIT, exit_code);
}

int readdir(unsigned int fd, struct dirent *dirent, unsigned int count)
{
	syscall3(SYSCALL_READDIR, fd, dirent, count);
}

pid_t getpid(void)
{
	syscall0(SYSCALL_GETPID);
}

void reboot(void)
{
	syscall0(SYSCALL_REBOOT);
}

int stat(const char *path, struct stat *buf)
{
	syscall2(SYSCALL_STAT, path, buf);
}

time_t time(time_t *t)
{
	syscall1(SYSCALL_TIME, t);
}

int chdir(const char *path)
{
	syscall1(SYSCALL_CHDIR, path);
}

char *getcwd(char *buf, size_t size)
{
	syscall2(SYSCALL_GETCWD, buf, size);
}

int fork(void)
{
	syscall0(SYSCALL_FORK);
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	syscall3(SYSCALL_EXECVE, path, argv, envp);
}

int waitpid(pid_t pid, int *status, int options)
{
	syscall3(SYSCALL_WAITPID, pid, status, options);
}

int setenv(const char *name, const char *value, int overwrite)
{
	syscall3(SYSCALL_SETENV, name, value, overwrite);
}

int unsetenv(const char *name)
{
	syscall1(SYSCALL_UNSETENV, name);
}

uid_t getuid(void)
{
	syscall0(SYSCALL_GETUID);
}

gid_t getgid(void)
{
	syscall0(SYSCALL_GETGID);
}

int setuid(uid_t uid)
{
	syscall1(SYSCALL_SETUID, uid);
}

int setgid(gid_t gid)
{
	syscall1(SYSCALL_SETGID, gid);
}

uid_t geteuid(void)
{
	syscall0(SYSCALL_GETEUID);
}

gid_t getegid(void)
{
	syscall0(SYSCALL_GETEGID);
}

int seteuid(uid_t uid)
{
	syscall1(SYSCALL_SETEUID, uid);
}

int setegid(gid_t gid)
{
	syscall1(SYSCALL_SETEGID, gid);
}

int pipe(int pipefd[2])
{
	syscall1(SYSCALL_PIPE, pipefd);
}

void yield(void)
{
	syscall0(SYSCALL_YIELD);
}

int vfork(void)
{
	syscall0(SYSCALL_VFORK);
}

int clone(void *stack)
{
	syscall0(SYSCALL_CLONE);
}

pid_t gettid(void)
{
	syscall0(SYSCALL_GETTID);
}

int dup(int oldfd)
{
	syscall1(SYSCALL_DUP, oldfd);
}

int __dup2(int oldfd, int newfd)
{
	syscall2(SYSCALL_DUP2, oldfd, newfd);
}
int dup2(int oldfd, int newfd) __attribute__((weak, alias("__dup2")));
