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
#include <kheap.h>
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
#include <procfs.h>
#include <dirent.h>
#include <io.h>
#include <errno.h>
#include <signal.h>
#include <linkedlist.h>

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
int pthread_kill_nolock(pid_t tid, int signum);
void free_env(void);

/*
 * import symbols
 */
extern page_directory_t *current_directory;
extern void signal_handler(int signum);

/*
 * Global symmbols
 */
DEFINE_LINKED_LIST(task_t, tasks_list);
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
	idle_task.mmaps_lock = MUTEX_INITIALIZER;
	idle_task.mmaps = NULL;
	idle_task.cwd = NULL;
	idle_task.root = NULL;
	idle_task.argv = NULL;
	idle_task.envp = NULL;
	idle_task.buffers = NULL;
	idle_task.sigmask = 0;
	idle_task.thread_stack = NULL;
	idle_task.procfs_node = NULL;
	idle_task.parent = NULL;
	idle_task.children_list = LINKED_LIST_INIT(task_t);
	idle_task.threads_list = LINKED_LIST_INIT(task_t);
	
	clear_task_signals(&idle_task);
	
	arch_set_kernel_stack(idle_task.machine_state, NULL);
	arch_set_directory(idle_task.machine_state, current_directory);
	arch_set_directory_physical(idle_task.machine_state, (uint32_t) current_directory_physical);
	arch_set_memmap(idle_task.machine_state, current_memmap);

	for (i = 0; i < 32; i++)
	{
		idle_task.sig_action[i].sa_flags = 0;
		idle_task.sig_action[i].sa_mask = 0;
		idle_task.sig_action[i].sa_handler = SIG_DFL;
	}

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
		if (likely(tasks_list.count != 0))
		{
			tmp = LINKED_LIST_MOVE_FIRST(tasks_list);
		}
		else
		{
			mutex_release(&schedule_lock);
			return;
		}
	}
	else
	{
		if (unlikely(tasks_list.current->next == NULL && tasks_list.count == 0))
			tmp = &idle_task;
		else
			tmp = LINKED_LIST_MOVE_NEXT(tasks_list);
		
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
			tmp = LINKED_LIST_MOVE_FIRST(tasks_list);
		}

		/* wake up the task if it has a signal */
		if (unlikely(tmp->sig_pending[0] != 0 && tmp->status == TASK_STATE_WAITING))
			tmp->status = TASK_STATE_RUNNING;
	
		if (tmp == current_task && 
			current_task->status != TASK_STATE_RUNNING)
			tmp = &idle_task;

		if (tmp->status == TASK_STATE_RUNNING)
			break;
		tmp = LINKED_LIST_MOVE_NEXT(tasks_list);
	}
	
	assert(tmp != &idle_task || tmp->sig_pending[0] == 0);
	assert(arch_get_kernel_stack(tmp->machine_state) != NULL || tmp == &idle_task);

	/*
	 * switch to the new task
	 */
	if (likely(tmp->sig_pending[0] == 0))
		arch_scheduler_load_task_state(tmp);
	else
		arch_scheduler_load_task_state_and_signal(tmp, tmp->sig_pending[0]);
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
	return tasks_list.count;
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
void __attribute__((__noreturn__)) exit(int exit_code) 
{
	LINKED_LIST_ITER_STRUCT(task_t) iter;
	
	assert(current_task->id != 0);
	/* assert(current_task->parent != NULL); */
	assert(current_task->threads_list.count != 0);
	
	/*
	 * Signal all threads to exit and wait
	 */
	if (current_task == current_task->main_thread &&
				current_task->threads_list.count != 0)
	{
		/*
		 * TODO: I think we need locking here since a child
		 * thread can spawn a thread that will be added to the
		 * parent's thread list.
		 */
		iter = LINKED_LIST_ITER_INIT(task_t, current_task->threads_list);
		while (iter.current != NULL)
		{
			pthread_kill(iter.current->val->id, SIGKILL);
			schedule();
			LINKED_LIST_MOVE_NEXT(iter);
		}
	}

	/*
	 * If the current task has children make them
	 * all orphans
	 */
	if (unlikely(current_task->children_list.count != 0))
	{
		mutex_wait(&current_task->lock);
		while (current_task->children_list.first != NULL)
		{
			current_task->children_list.first->val->parent = NULL;
			LINKED_LIST_REMOVE_FIRST_FREE(task_t, current_task->children_list);
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
		iter = LINKED_LIST_ITER_INIT(task_t,
			current_task->main_thread->threads_list);
		LINKED_LIST_SEEK_UNTIL(iter, 
			iter.current->val == current_task);
		LINKED_LIST_REMOVE_CURRENT_FREE(
			current_task->main_thread->threads_list, iter);
	}
	
	/* send the SIGCHLD signal to parent */
	if (likely(current_task->parent != NULL))
		pthread_kill_nolock(current_task->parent->id, SIGCHLD);

	/*
	 * Invoke the schedule. The task should get switched out
	 * for good so this will never return
	 */
	schedule_prelocked();
	assert(0);
	while (1);
}

/*
 * Exit a thread
 */
int pthread_exit(int status_code) __attribute__((weak, alias("exit")));

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
 * This is the entry point into usermode.
 * It calls the program's entry point and calls
 * exit upon return.
 */
void __usermode user_entry(
	const int (*main_func)(int, char**), int argc, char **argv)
{
	/* exit(main_func(argc, argv)); */
	int ret;
	ret = main_func(argc, argv);
	exit(ret);
}

/*
 * Replace the current userspace with an executable
 * image
 */
int execve(const char *path, char *const argv[], char *const envp[])
{
	int i, j, l, fd;
	intptr_t vaddr;
	char *next_arg;

	if (unlikely(strlen(path) > MAX_PATH))
		return ENAMETOOLONG;

	if (unlikely(path == NULL || *path == NULL))
		return EBADF;
	
	fd = open(path, NULL);
	if (fd < 0)
		return -1;
		
	while (current_task->buffers != NULL)
		kfree((void*) current_task->buffers->address);
	
	current_task->sigmask = 0;
	clear_task_signals(current_task);
	
	vaddr = elf_load(fd);
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
	{
		current_task->sig_action[i].sa_flags = 0;
		current_task->sig_action[i].sa_mask = 0;
		current_task->sig_action[i].sa_handler = SIG_DFL;
	}

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
	l += (sizeof(char*) * (i + 2));
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
	arch_enter_user_mode(ARCH_STACK_START, vaddr, (i + 1), current_task->argv, l);
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
 * Remove a task from any lists that hold references
 * to it and from the task table
 */
static inline int free_task_entry(task_t *t)
{
	pid_t terminated_pid;
	LINKED_LIST_ITER_STRUCT(task_t) iter;

	assert(t != NULL);
	assert(t->main_thread != NULL);
	
	if (unlikely(tasks_list.count == 0))
		return -1;

	/*
	 * Remove the task from the parent's children
	 * or threads list
	 */
	if (t != t->main_thread)
	{
		/*
		 * This is done by exit().
		 */
		#if 0
		mutex_wait(&t->main_thread->lock);
		tmp = t->main_thread->threads;
		while (tmp->next_thread != t)
			tmp = tmp->next_thread;
		tmp->next_thread = t->next_thread;
		mutex_release(&t->main_thread->lock);
		#endif
	}
	else
	{
		if (likely(t->parent != NULL))
		{
			mutex_wait(&t->parent->lock);
			iter = LINKED_LIST_ITER_INIT(task_t, t->parent->children_list);
			LINKED_LIST_SEEK_UNTIL(iter, iter.current->val == t);
			LINKED_LIST_REMOVE_CURRENT_FREE(t->parent->children_list, iter);
			mutex_release(&t->parent->lock);
		}
	}

	/*
	 * Remove the entry from the task list 
	 */
	mutex_wait(&schedule_lock);
	iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
	LINKED_LIST_SEEK_UNTIL(iter, iter.current->val == t);
	LINKED_LIST_REMOVE_CURRENT_FREE(tasks_list, iter);
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

/*
 * Wait for a child process to exit.
 */
pid_t wait(int *status)
{
	/* FIXME: Why is this not locking? */
	assert(tasks_list.count != 0);
	LINKED_LIST_MOVE_FIRST(current_task->children_list);
	LINKED_LIST_SEEK_UNTIL(current_task->children_list,
		current_task->children_list.current->val->status != TASK_STATE_ZOMBIE);
	return free_task_entry(current_task->children_list.current->val);
}

/*
 * Wait for a process to exit, get it's status
 * code, clean up the task entry, and return.
 */
int waitpid(pid_t pid, int *status, int options)
{
	LINKED_LIST_ITER_STRUCT(task_t) tasks_iter;
	
	if (unlikely(pid == -1))
		return wait(status);
	
	/* find the task entry in the task list */
	mutex_wait(&schedule_lock);
	tasks_iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
	LINKED_LIST_SEEK_UNTIL(tasks_iter, 
		unlikely(tasks_iter.current->val->pid && 
		tasks_iter.current->val->main_thread !=  tasks_iter.current->val));
	mutex_release(&schedule_lock);

	if (tasks_iter.current->val == NULL || tasks_iter.current->val->parent != current_task)
		return ECHILD;

	while (tasks_iter.current != NULL && tasks_iter.current->val->status != TASK_STATE_ZOMBIE)
	{
		pause();
		mutex_wait(&schedule_lock);
		if (unlikely(tasks_list.count == 0))
		{
			mutex_release(&schedule_lock);
			return -1;
		}

		tasks_iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
		LINKED_LIST_SEEK_UNTIL(tasks_iter,
			unlikely(tasks_iter.current->val->pid &&
				tasks_iter.current->val->main_thread != tasks_iter.current->val));
		mutex_release(&schedule_lock);
	}

	assert(tasks_iter.current != NULL);
	*status = tasks_iter.current->val->exit_code;

	return free_task_entry(tasks_iter.current->val);
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


