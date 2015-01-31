/*
 * scheduler.c
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
#include <mutex.h>
#include <scheduler.h>
#include <mmu.h>
#include <memory.h>
#include <elfldr.h>
#include <vfs.h>
#include <syscall.h>
#include <timer.h>
#include <symbols.h>
#include <unistd.h>
#include <string.h>
#include <printk.h>
#include <kheap.h>
#include <procfs.h>
#include <dirent.h>
#include <io.h>
#include <errno.h>

/*
 * debug symbols
 */
#define SCHED_DEBUG_MOVE_STACK		(0)
#define SCHED_DEBUG_FORK				(0)

/*
 * static symbols
 */
static task_t idle_task;

int kill_nolock(pid_t pid, int sig);
int killtask_nolock(pid_t tid, int signum);
void free_env(void);

/*
 * import symbols
 */
extern page_directory_t *current_directory;
extern void signal_handler(int signum);

/*
 * Global symmbols
 */
task_t *tasks = NULL;
task_t *current_task = NULL;
mutex_t schedule_lock = MUTEX_INITIALIZER;

/*
 * initialize the scheduler
 */
void scheduler_init(void)
{
	int i;
	desc_entry_t *descriptors;
	
	
	arch_scheduler_init();
	/*
	 * move the stack to the end of the user space
	 */
	arch_move_stack((void*) ARCH_STACK_START, ARCH_STACK_INITIAL_SIZE);

	/*
	 * create a descriptor table
	 */
	descriptors = (desc_entry_t*) malloc(sizeof(desc_entry_t));
	if (descriptors == NULL)
		panic("scheduler: Out of kernel memory!");
	descriptors->fd_next = 0;
	descriptors->file_descriptors = NULL;
	descriptors->lock = MUTEX_INITIALIZER;
	descriptors->refs = 0;

	idle_task.id = 0;
	idle_task.pid = 0;
	idle_task.signal = 0;
	idle_task.uid = 0;
	idle_task.gid = 0;
	idle_task.euid = 0;
	idle_task.egid = 0;
	idle_task.saved_uid = 0;
	idle_task.saved_gid = 0;
	idle_task.exit_code = 0;
	idle_task.main_thread = &idle_task;
	idle_task.status = TASK_STATE_RUNNING;
	idle_task.descriptors_info = descriptors;
	idle_task.lock = MUTEX_INITIALIZER;
	idle_task.env_lock = MUTEX_INITIALIZER;
	idle_task.cwd = NULL;
	idle_task.argv = NULL;
	idle_task.envp = NULL;
	idle_task.procfs_node = NULL;
	idle_task.parent = NULL;
	idle_task.next_thread = NULL;
	idle_task.threads = &idle_task;
	idle_task.children = NULL;
	idle_task.next_child = NULL;
	idle_task.next = NULL;

	arch_set_kernel_stack(idle_task.machine_state, NULL);
	arch_set_directory(idle_task.machine_state, current_directory);
	arch_set_directory_physical(idle_task.machine_state, (uint32_t) current_directory_physical);
	arch_set_memmap(idle_task.machine_state, current_memmap);

	for (i = 0; i < 32; i++)
		idle_task.sig_handler[i] = NULL;

	current_task = &idle_task;
	schedule();
	arch_enable_interrupts();
	procfs_init();

	printk(7, "scheduler: promoting to pid %i", getpid());
}

/*
 * Perform a task switch.
 * The caller must acquire the schedule_lock semaphore
 * before calling this function.
 */
void schedule_prelocked(void)
{
	task_t *tmp;

	/*
	 * Save the state of the current task
	 */
	arch_scheduler_save_task_state(current_task->machine_state);
	arch_scheduler_save_task_ip(current_task->machine_state);
	
	/*
	 * If the task was just resumed (by arch_scheduler_load_task_state
	 * or arch_scheduler_load_task_state_and_signal) we'll end up back here
	 * but with a magic number on the task state structure. Here we check
	 * that magic number and return if it's there.
	 */
	if (arch_task_resumed(current_task->machine_state))
	{
		mutex_release(&schedule_lock);
		return;
	}

	/*
	 * get the next task to run
	 *
	 * If the current task is the idle task we check if there are 
	 * other tasks running. If there are we start scheduling from
	 * the beggining of the list, otherwise we leave. 
	 * If there is a task running we start scheduling from the 
	 * process next to the current process on the list.
	 *
	 */
	if (unlikely(current_task == &idle_task))
	{
		if (likely(tasks != NULL))
		{
			tmp = tasks;
		}
		else
		{
			mutex_release(&schedule_lock);
			return;
		}
	}
	else
	{
		if (unlikely(current_task->next == NULL && tasks == NULL))
			tmp = &idle_task;
		else
			tmp = current_task->next;
	}

	/*
	 * go through the list of task until we find one that's
	 * ready to run.
	 */
	int x = 0;
	while (1)
	{
		if (unlikely(tmp == NULL))
		{
			if (x++ > 0) /* TODO: this is ugly. find a better way */
			{
				tmp = &idle_task;
				break;
			}
			tmp = tasks;
		}

		/* wake up the task if it has a signal */
		if (unlikely(tmp->signal != 0 && tmp->status == TASK_STATE_WAITING))
			tmp->status = TASK_STATE_RUNNING;
	
		if (tmp == current_task && 
			current_task->status != TASK_STATE_RUNNING)
			tmp = &idle_task;

		if (tmp->status == TASK_STATE_RUNNING)
			break;
		tmp = tmp->next;
	}
	
	assert(tmp != &idle_task || tmp->signal == 0);
	assert(arch_get_kernel_stack(tmp->machine_state) != NULL || tmp == &idle_task);

	/*
	 * switch to the new task
	 */
	if (likely(tmp->signal == 0))
		arch_scheduler_load_task_state(tmp);
	else
		arch_scheduler_load_task_state_and_signal(tmp, tmp->signal);
}

/*
 * Invoke the scheduler
 */
void schedule(void)
{
	if (!mutex_try(&schedule_lock))
		return;	
	schedule_prelocked();
}

/*
 * Yield the current timeslice.
 */
void yield() __attribute__((weak, alias("schedule")));

/*
 * gets the number of running tasks
 */
unsigned int gettaskcount(void)
{
	unsigned int i = 1;
	task_t *tmp;
	tmp = tasks;
	while (tmp->next != NULL)
	{
		tmp = tmp->next;
		i++;
	}
	return i;
}


/*
 * change the current working directory
 */
void scheduler_chdir(vfs_node_t *node)
{
	current_task->cwd = node;
}

/*
 * Get the current working directory
 */
vfs_node_t *scheduler_getcwd(void)
{
	assert(current_task != NULL);
	return current_task->cwd;
}

/*
 * exit the current task
 */
int exit(int exit_code)
{
	task_t *tmp, *parent;
	assert(current_task->id != 0);
	assert(current_task->parent != NULL);
	assert(current_task->threads != NULL);
	
	/*
	 * Signal all threads to exit and wait
	 */
	if (current_task == current_task->main_thread &&
				current_task->threads->next_thread != NULL)
	{
		while (current_task->next_thread != NULL)
		{
			killtask(current_task->threads->next_thread->id, SIGKILL);
			schedule();
		}
	}

	/*
	 * If the current task has children make them
	 * all orphans
	 */
	if (unlikely(current_task->children))
	{
		mutex_wait(&current_task->lock);
		tmp = current_task->children;
		while (tmp != NULL)
		{
			parent = tmp->next_child;
			tmp->parent = NULL;
			tmp->next_child = NULL;
			tmp = parent;
		}
		mutex_release(&current_task->lock);
	}
	
	/*
	 * 1. set task state to ZOMBIE
	 * 2. set the status code
	 * 3. close all file descriptors
	 * 4. free all process pages except for the kernel
	 *    stack as we need it to exit this routine
	 */
	mutex_busywait(&schedule_lock);
	current_task->status = TASK_STATE_ZOMBIE;
	current_task->exit_code = exit_code;
	
	if (current_task == current_task->main_thread)
	{
		/*
		 * If this is the main thread we free everything
		 */
		if (current_task->argv != NULL)
			free(current_task->argv);
		free_env();
		destroy_descriptor_table();
		memmap_destroy(arch_get_memmap(current_task->machine_state));
		free(arch_get_memmap(current_task->machine_state));
		mmu_switch_to_kernel_directory();
		mmu_destroy_directory(arch_get_directory(current_task->machine_state));
	}
	else
	{
		/*
		 * Remove the current thread from the list of
		 * threads
		 */
		parent = current_task->main_thread;
		tmp = current_task->threads->next_thread;
		while (tmp != NULL && tmp != current_task)
		{
			parent = tmp;
			tmp = tmp->next_thread;
		}
		assert(parent != NULL);
		assert(tmp == current_task);
		parent->next_thread = current_task->next_thread;
	}
	
	/* send the SIGCHLD signal to parent */
	if (likely(current_task->parent != NULL))
		killtask_nolock(current_task->parent->id, SIGCHLD);

	/*
	 * Invoke the schedule. The task should get switched out
	 * for good so this will never return
	 */
	schedule_prelocked();
	assert(0);
	return exit_code;
}

/*
 * get the pid of the current task
 */
pid_t getpid(void)
{
	assert(current_task != NULL);
	return current_task->pid;
}

/*
 * gets the thread id of the current task.
 */
pid_t gettid(void)
{
	assert(current_task != NULL);
	return current_task->id;
}

/*
 * gets the UID of the current task
 */
uid_t getuid(void)
{
	assert(current_task != NULL);
	return current_task->uid;
}

/*
 * gets the GID of the current task.
 */
gid_t getgid(void)
{
	assert(current_task != NULL);
	return current_task->gid;
}

/*
 * execute a file
 */
int exec(char *path, ...)
{
	return execve(path, NULL, NULL);
}

/*
 * Replace the current userspace with an executable
 * image
 */
int execve(const char *path, char *const argv[], char *const envp[])
{
	int i, j, l;
	intptr_t vaddr;
	char *next_arg;
	vfs_node_t *node;

	if (unlikely(strlen(path) > MAX_PATH))
		return ENAMETOOLONG;

	if (unlikely(path == NULL || *path == NULL))
		return EBADF;
	
	node = vfs_open(path, NULL);
	if (node == NULL)
		return ENOENT;
	
	vaddr = elf_load(node);
	if (vaddr == NULL)
	{
		printk(6, "exec: could not load elf!");
		return -1;
	}
	
	/*
 	 * Reset all signal handlers to default
	 * disposition
	 */
	for (i = 0; i < 32; i++)
		current_task->sig_handler[i] = NULL;

	/* free previously allocated args */
	if (current_task->argv != NULL)
		free(current_task->argv);
	
	/* 
	 * walk the arguments list to figure out
	 * the number of arguments.
	 */
	i = l = 0;
	if (argv != NULL)
	{
		while (argv[i] != NULL)
		{
			l += strlen(argv[i]) + 1;
			i++;
		}
	}
	
	/*
	 * allocate heap space to store a string pointer
	 * for each argument plus a NULL pointer.
	 */
	l += (sizeof(char*) * (i + 1));
	l += strlen(path) + 1;
	
	/* make sure sz is word aligned */
	if (l & 0x3)
	{
		l &= ~0x3;
		l += 0x4;
	}
	
	void *tmp = malloc(l);
	current_task->argv = tmp;
	if (current_task->argv == NULL)
		panic("What do we do here?");
	
	/*
	 * copy all arguments to the heap
	 */
	next_arg = (char*) ((intptr_t) 
		current_task->argv + (sizeof(char*) * (i + 2)));
	current_task->argv[0] = next_arg;
	strcpy(current_task->argv[0], path);
	next_arg += strlen(path) + 1;
	for (j = 0; j < i; j++)
	{
		current_task->argv[j + 1] = next_arg;
		strcpy(current_task->argv[j + 1], argv[j]);
		next_arg +=  strlen(argv[j + 1]) + 1;
	}
	current_task->argv[j + 1] = NULL;
	
	/*
	 * Set the task's environment variables
	 */
	free_env();
	if (envp != NULL)
	{
		char buf[255];
		char *name;
		char *value;
		char *envs;
							
		while(*envp != NULL)
		{
			name = buf;
			envs = *envp;
			while (*envs != NULL && *envs != '=')
				*name++ = *envs++;
			if (*envs == '=')
				envs++;
			*name++ = NULL;
			value = envs;
			setenv(buf, value, 0);
			envp++;
		}
	}

	/*
	 * jump into user-mode and to the program's
	 * entry point. Then call exit() from usermode
	 * to make sure we don't return to kernel mode
	 */
	arch_enter_user_mode(ARCH_STACK_START, vaddr, i, current_task->argv, l);
	assert(0);
	return ENOENT;
}

void sleep_ticks(tticks_t ticks)
{
	tticks_t t;
	t = timer_getticks() + ticks;
	assert(t > ticks); /* TODO: handle overflow */
	while (timer_getticks() < t)
		schedule();
}

/*
 * set a signal handler
 */
int signal(int signum, void* handler)
{
	return -1;
}

/*
 * 
 */
int killtask(pid_t pid, int signum)
{
	int r;
	mutex_wait(&schedule_lock);
	r = killtask_nolock(pid, signum);
	mutex_release(&schedule_lock);
	return r;
}

/*
 * Send a signal to a specific thread.
 * If tid is 0 the signal is sent to allocate
 * threads.
 */
int killtask_nolock(pid_t tid, int signum)
{
	task_t *tmp;
	if (tid == 0)
	{
		tmp = tasks;
		while (tmp != NULL)
		{
			tmp->signal |= (1 << (signum - 1));
			tmp = tmp->next;
		}
	}
	else
	{
		tmp = tasks;
		while (tmp != NULL && tmp->id != tid)
			tmp = tmp->next;
		if (tmp == NULL)
			return -1;
		tmp->signal |= (1 << (signum - 1));
	}
	return 0;
}

/*
 * Send a signal to a process
 */
int kill(pid_t pid, int signum)
{
	int r;
	mutex_wait(&schedule_lock);
	r = kill_nolock(pid, signum);
	mutex_release(&schedule_lock);
	return r;
}

/*
 * sends a signal to a task
 */
int kill_nolock(pid_t pid, int signum)
{
	task_t *tmp;
	if (pid == 0)
	{
		tmp = tasks;
		while (tmp != NULL)
		{
			if (tmp->pid == pid && tmp->main_thread == tmp)
				tmp->signal |= (1 << (signum - 1));
			tmp = tmp->next;
		}
	}
	else
	{
		tmp = tasks;
		while (tmp != NULL && tmp->pid != pid && tmp->main_thread != tmp)
			tmp = tmp->next;
		if (tmp == NULL)
			return -1;
		tmp->signal |= (1 << (signum - 1));
	}
	return 0;
}

/*
 * raise a signal
 */
int raise(int signal)
{
	return killtask(current_task->id, signal);
}

/*
 * Puts the current task to sleep
 */
void wait_signal(void)
{
	assert(current_task != NULL);
	assert(current_task != &idle_task);
	mutex_busywait(&schedule_lock);
	current_task->status = TASK_STATE_WAITING;
	schedule_prelocked();
	return;
}

/*
 * Remove a task from any lists that hold references
 * to it and from the task table
 */
static inline int free_task_entry(task_t *t)
{
	task_t *tmp, **ptmp;
	pid_t terminated_pid;
	
	assert(t != NULL);
	assert(t->main_thread != NULL);
	
	if (unlikely(tasks == NULL))
		return -1;

	/*
	 * Remove the task from the parent's children
	 * or threads list
	 */
	if (t != t->main_thread)
	{
		mutex_wait(&t->main_thread->lock);
		tmp = t->main_thread->threads;
		while (tmp->next_thread != t)
			tmp = tmp->next_thread;
		tmp->next_thread = t->next_thread;
		mutex_release(&t->main_thread->lock);
	}
	else
	{
		if (likely(t->parent != NULL))
		{
			mutex_wait(&t->parent->lock);
			ptmp = &t->parent->children;
			while (*ptmp != NULL)
				ptmp = &(*ptmp)->next_child;
			*ptmp = t->next_child;
			mutex_release(&t->parent->lock);
		}
	}

	/*
	 * Remove the entry from the task list 
	 */
	mutex_wait(&schedule_lock);
	ptmp = &tasks;
	while (*ptmp != t)
	{
		ptmp = &(*ptmp)->next;
		assert(*ptmp != NULL);
	}
	*ptmp = t->next;
	mutex_release(&schedule_lock);

	/* free the task table entry and the procfs node */
	terminated_pid = t->id;
	kfree((void*) t->machine_state.kernel_stack);
	vfs_rmnod(t->procfs_node);
	free(t->procfs_node);
	free(t);
	return terminated_pid;
}

int waitid(pid_t id, int type)
{
	return ENOSYS;
}

pid_t wait(int *status)
{
	task_t *tmp;
	
	assert(tasks != NULL);
	
	//tmp = pr
	
	return ENOSYS;
}

/*
 * Wait for a process to exit, get it's status
 * code, clean up the task entry, and return.
 */
int waitpid(pid_t pid, int *status, int options)
{
	task_t *tmp, *parent;
	
	if (unlikely(pid == -1))
		return wait(status);
	
	/* find the task entry in the task list */
	mutex_busywait(&schedule_lock);
	tmp = tasks;
	while (tmp != NULL)
	{
		if (unlikely(tmp->pid == pid && tmp->main_thread == tmp))
			break;
		tmp = tmp->next;
	}
	mutex_release(&schedule_lock);

	if (tmp == NULL || tmp->parent != current_task)
		return ECHILD;

	while (tmp != NULL && tmp->status != TASK_STATE_ZOMBIE)
	{
		wait_signal();
		mutex_busywait(&schedule_lock);
		if (unlikely(tasks == NULL))
		{
			mutex_release(&schedule_lock);
			return -1;
		}

		tmp = tasks;
		while (tmp != NULL)
		{
			if (unlikely(tmp->pid == pid && tmp->main_thread == tmp))
				break;
			tmp = tmp->next;
		}
		mutex_release(&schedule_lock);
	}

	assert(tmp != NULL); /* task should always exist when calling this right? */
	*status = tmp->exit_code;

	return free_task_entry(tmp);
}

/*
 * Set the UID
 */
int setuid(uid_t uid)
{
	/*
	 * http://pubs.opengroup.org/onlinepubs/009695399/functions/setuid.html
	 */
	if (likely(current_task->euid == 0))
	{
		current_task->uid = uid;
		current_task->euid = uid;
	}
	else
	{
		if (likely(uid == current_task->uid))
		{
			current_task->euid = uid;
		}
		else
		{
			current_task->errno = EPERM;
			return -1;
		}
	}
	return 0;
}

/*
 * Sets the GID.
 */
int setgid(gid_t gid)
{
	/*
	 * http://pubs.opengroup.org/onlinepubs/009695399/functions/setgid.html
	 */
	if (current_task->euid == 0)
	{
		current_task->gid = gid;
		current_task->egid = gid;
	}
	else
	{
		if (current_task->gid == gid)
		{
			current_task->egid = gid;
		}
		else
		{
			current_task->errno = EPERM;
			return -1;
		}
	}
	return 0;
}

/*
 * Get the current UID
 */
uid_t geteuid(void)
{
	return current_task->euid;
}

/*
 * Get the current GID.
 */
gid_t getegid(void)
{
	return current_task->egid;
}

/*
 * Set the effective UID
 */
int seteuid(uid_t uid)
{
	if (current_task->uid == uid || current_task->uid == 0)
	{
		current_task->euid = uid;
	}
	else
	{
		current_task->errno = EPERM;
		return -1;
	}
	return 0;
}

/*
 * Set the effective GID
 */
int setegid(gid_t gid)
{
	if (current_task->gid == gid || current_task->uid == 0)
	{
		current_task->egid = gid;
	}
	else
	{
		current_task->errno = EPERM;
		return -1;
	}
	return 0;
}

void dump_task_info(void)
{
	printk(7, "pid = %i", getpid());
}


