/*
 * include/scheduler.h
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

#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <arch/platform.h>
#include <arch/scheduler.h>
#include <mutex.h>
#include <signal.h>
#include <mmu.h>
#include <vfs.h>
#include <memmap.h>

#define SCHED_KERNEL_STACK_SIZE		(4096 * 0x10)
#define CONFIG_ENV_INITIAL_SIZE		(1024)


/*
 * task states
 */
#define TASK_STATE_RUNNING		(0)
#define TASK_STATE_WAITING		(1)
#define TASK_STATE_STOPPED		(2)
#define TASK_STATE_ZOMBIE		(3)

struct __descriptor_table;

/*
 * task state structure
 */
typedef struct __task
{
	task_state_t machine_state;	/* DO NOT MOVE THIS */
	pid_t id;
	pid_t pid;
	uid_t uid;
	gid_t gid;
	uid_t euid;
	gid_t egid;
	uid_t saved_uid;
	gid_t saved_gid;
	int32_t errno;
	unsigned int status;
	uint32_t signal;
	int exit_code;
	char **argv;
	char **envp;
	signal_handler_t sig_handler[32];
	vfs_node_t *procfs_node;
	vfs_node_t *cwd;
	mutex_t env_lock;
	mutex_t lock;
	struct __descriptor_table *descriptors_info;
	struct __task *parent;
	struct __task *main_thread;
	struct __task *children;
	struct __task *threads;
	struct __task *next_child;
	struct __task *next_thread;
	struct __task *next;
} 
task_t;

/*
 * imports
 */
extern task_t *current_task;			/* widely used */
extern task_t *tasks;			/* used by procfs */
extern mutex_t schedule_lock;		/* used by procfs */

/*
 * Gets the current task.
 */
#define get_current_task()	(current_task)

/*
 * initialize scheduler
 */
void scheduler_init(void);

/*
 * invoke the scheduler
 */
void schedule(void);

/*
 * Send a signal to all processes.
 */
int killall(int signum);

/*
 * wait for a signal
 */
void wait_signal(void);

/*
 * gets the count of running tasks
 */
unsigned int gettaskcount(void);

/*
 * executes a file with context and environment
 */
int execve(const char *filename, 
	char *const argv[], char *const envp[]);

/*
 * executes a file
 */
int exec(char *path, ...);

/*
 * Change the working directory for the current task.
 */
void scheduler_chdir(vfs_node_t *node);

/*
 * Get the current directory node for the current task.
 */
vfs_node_t *scheduler_getcwd(void);

#endif
