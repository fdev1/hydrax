/*
 * include/unistd.h
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

#ifndef __UNISTD_H__
#define __UNISTD_H__

#define MODE_TYPE_DIRECTORY		(0x1)
#define MODE_TYPE_LINK			(0x2)
#define MODE_TYPE_MOUNT			(0x4)
#define MODE_TYPE_CHARDEVICE		(0x8)

#define S_ISDIR(mode)			(mode & MODE_TYPE_DIRECTORY)
#define S_ISCHR(mode)			(mode & MODE_TYPE_CHARDEVICE)

#include <vfs.h>
#include <sys/stat.h>

/*
 * Opens a file descriptor
 */
int open(const char *pathname, int flags);

/*
 * Closes a file descriptor
 */
int close(int fd);

/*
 * read the next entry in a directory
 */
int readdir(unsigned int fd, struct dirent *dirent, unsigned int count);

/*
 * Writes to a file descriptor
 */
size_t write(int fd, const void* buf, size_t count);

/*
 * Reads from a file descriptor
 */
size_t read(int fd, void* buf, size_t count);

/*
 * Get information about a file.
 */
int stat(const char *path, struct stat *buf);


/*
 * forks the current task
 */
int fork(void);

/*
 * Forks a new task that shares the entire
 * parent process userspace.
 */
int vfork(void);

/*
 * Send a signal to a process.
 */
int kill(pid_t pid, int signum);

/*
 * clone the current task
 */
int clone(void *stack);

/*
 * exit the current task
 */
int exit(int exit_code);

/*
 * gets the PID of the running process
 */
pid_t getpid(void);

/*
 * gets the UID of the current task.
 */
uid_t getuid(void);

/*
 * gets the GID of the current task.
 */
uid_t getgid(void);

/*
 * Creates a pipe.
 */
int pipe(int pipefd[2]);

/*
 * executes a file with context and environment
 */
int execve(const char *filename, 
	char *const argv[], char *const envp[]);

/*
 * Change the working directory.
 */
int chdir(const char *path);

/*
 * Get the current working directory
 */
char *getcwd(char *buf, size_t size);

/*
 * Wait for a process to exit
 */
int waitpid(pid_t pid, int *status, int options);

/*
 * Send a signal to the current process.
 */
int raise(int sig);

/*
 * Sets and environment variable
 */
int setenv(const char *name, const char *value, int overwrite);

/*
 * Unsets an environment variable
 */
int unsetenv(const char *name);

/*
 * Set the UID
 */
int setuid(uid_t uid);

/*
 * Sets the GID.
 */
int setgid(gid_t gid);

/*
 * Get the current UID
 */
uid_t geteuid(void);

/*
 * Get the current GID
 */
gid_t getegid(void);

/*
 * Set the effective UID
 */
int seteuid(uid_t uid);

/*
 * Set the effective GID
 */
int setegid(gid_t gid);

/*
 * Send IOCTL request to device driver.
 */
int ioctl(int fd, unsigned long request, ...);

/*
 * Duplicate a file descriptor.
 */
int dup(int oldfd);

/*
 * Duplicate a file descriptor and assign newfd
 * to the it. If newfd is already open it is closed.
 */
int dup2(int oldfd, int newfd);

/*
 * Yield the CPU
 */
void yield(void);

/*
 * Get the thread id of the current task.
 */
pid_t gettid(void);

/*
 * Send a signal to the thread specified by
 * task_id.
 */
int killtask(pid_t tid, int signum);

#endif

