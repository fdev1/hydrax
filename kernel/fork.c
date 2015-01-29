/*
 * fork.c
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

#include <arch/platform.h>
#include <mutex.h>
#include <scheduler.h>
#include <mmu.h>
#include <memory.h>
#include <procfs.h>
#include <io.h>
#include <kheap.h>
#include <string.h>

static pid_t next_pid = 1;
static pid_t next_tid = 1;
static light_mutex_t pid_lock = LIGHT_MUTEX_INITIALIZER;

void schedule_prelocked(void);

/*
 * This is where the actual forking happens.
 * This function is inlined by fork(), vfork(), and
 * clone().
 */
static inline __attribute__((always_inline)) 
int kfork(task_t *new_task, page_directory_t *directory, 
	intptr_t phys, memmap_t *memmap, intptr_t kernel_stack)
{
	int i;
	task_t *tmp_task;

	/* set default signal handlers */
	for (i = 0; i < 32; i++)
		new_task->sig_handler[i] = current_task->sig_handler[i];

	/*
	 * Save task context, move the stack, and save the
	 * instruction pointer.
	 */
	arch_set_directory(new_task->machine_state, directory);
	arch_set_directory_physical(new_task->machine_state, phys);
	arch_set_memmap(new_task->machine_state, memmap);
	arch_set_kernel_stack(new_task->machine_state, kernel_stack);
	arch_scheduler_save_task_state(new_task->machine_state);
	arch_scheduler_move_the_fucking_task_stack(new_task);
	arch_scheduler_save_task_ip(new_task->machine_state);

	if (current_task != new_task)
	{
		/*
		 * schedule task
		 */
		mutex_wait(&schedule_lock);
		if (tasks == NULL)
		{
			tasks = new_task;
		}
		else
		{
			tmp_task = (task_t*) tasks;
			while (tmp_task->next != NULL)
				tmp_task = tmp_task->next;
			tmp_task->next = new_task;
		}
		schedule_prelocked();
		return new_task->errno;
	}
	else
	{	
		/*
		 * The 1st time that a task is scheduled for execution
		 * the scheduler does not release the lock, so we release
		 * it here.
		 */
		mutex_release(&schedule_lock);
		return 0;
	}
}

/*
 * forks the current task
 */
int fork(void)
{
	desc_entry_t *descriptors;
	task_t *new_task;
	task_t **ptask;
	page_directory_t *directory;
	memmap_t *memmap, *parent_memmap;
	intptr_t phys, kernel_stack;
	vfs_node_t *procfs_node;
	pid_t new_pid, new_tid;
	
	mutex_wait((mutex_t*) &pid_lock);
	new_pid = next_pid++;
	new_tid = next_tid++;
	mutex_release((mutex_t*) &pid_lock);

	/*
	 * Clone the task's address space
	 */
	directory = mmu_clone_directory(current_directory, &phys, CLONE_COW);
	if (directory == NULL)
		return ENOMEM;

	/* allocate the task_t structure */
	new_task = (task_t*) malloc(sizeof(task_t));
	if (unlikely(new_task == NULL))
	{
		mmu_destroy_directory(directory); 
		return ENOMEM;
	}

	/* allocate space for memmap structure */
	memmap = (memmap_t*) malloc(sizeof(memmap_t));
	if (unlikely(memmap == NULL))
	{
		mmu_destroy_directory(directory);
		free(new_task);
		return ENOMEM;
	}

	/* clone the parent's task memmap */
	parent_memmap = arch_get_memmap(current_task->machine_state);
	if (unlikely(!memmap_clone(memmap, parent_memmap)))
	{
		mmu_destroy_directory(directory);
		free(memmap);
		free(new_task);
		return ENOMEM;
	}

	/* allocate a kernel stack */
	kernel_stack = kalloc(SCHED_KERNEL_STACK_SIZE,
		NULL, NULL, KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL);	
	if (unlikely(kernel_stack == NULL))
	{
		memmap_destroy(memmap);
		free(memmap);
		free(new_task);
		mmu_destroy_directory(directory);
		return ENOMEM;
	}

	/* allocate space for procfs node */
	procfs_node = (vfs_node_t*) malloc(sizeof(vfs_node_t));
	if (unlikely(procfs_node == NULL))
	{
		mmu_destroy_directory(directory);
		memmap_destroy(memmap);
		kfree((void*)kernel_stack);
		free(memmap);
		free(new_task);
		return ENOMEM;
	}
	
	/*
	 * Initialize the procfs node and add it to
	 * the procfs file system
	 */
	/* *procfs_node = vfs_node_init(FS_DIRECTORY); */
	strcpy(procfs_node->name, itoa(new_tid));
	procfs_node->parent = procfs;
	procfs_node->open = (open_type_t) &procfs_open;
	procfs_node->stat = (stat_type_t) &procfs_stat;
	procfs_node->readdir = (readdir_type_t) &procfs_readdir;
	procfs_node->flags = FS_DIRECTORY;
	procfs_node->next = NULL;
	vfs_mknod(procfs, procfs_node);

	/*
	 * Clone the parent task descriptor table.
	 */
	descriptors = clone_descriptor_table();
	if (unlikely(descriptors == NULL))
	{
		free(procfs_node);
		mmu_destroy_directory(directory);
		memmap_destroy(memmap);
		kfree((void*)kernel_stack);
		free(memmap);
		free(new_task);
	}
	
	/*
	 * Initialize the new task structure
	 */
	new_task->id = new_tid;
	new_task->pid = new_pid;
	new_task->main_thread = new_task;
	new_task->uid = current_task->uid;
	new_task->gid = current_task->gid;
	new_task->euid = current_task->euid;
	new_task->egid = current_task->egid;
	new_task->argv = NULL;
	new_task->envp = NULL;
	new_task->status = TASK_STATE_RUNNING;
	new_task->signal = 0;
	new_task->exit_code = 0;
	new_task->lock = MUTEX_INITIALIZER;
	new_task->descriptors_info = descriptors;
	new_task->env_lock = MUTEX_INITIALIZER;
	new_task->cwd = current_task->cwd;
	new_task->argv = NULL;
	new_task->procfs_node = procfs_node;
	new_task->parent = current_task;
	new_task->threads = new_task;
	new_task->errno = new_pid;
	new_task->children = NULL;
	new_task->next_child = NULL;
	new_task->next_thread = NULL;
	new_task->next = NULL;
	
	/*
	 * Add the task to the children list of the parent
	 */
	mutex_wait(&current_task->lock);
	ptask = &current_task->children;
	while (*ptask != NULL)
		ptask = &(*ptask)->next_child;
	*ptask = new_task;
	mutex_release(&current_task->lock);
	
	return kfork(new_task, directory, phys, memmap, kernel_stack);	
}

int vfork(void)
{
	return fork();
}

/*
 * Clone the current thread and it's address spacce.
 * This works exactly like fork except that the new
 * thread shares the entire userspace with the parent.
 * 
 * WARNING: Upon returning from this call the cloned
 * thread shares the entire userspace of the parent thread
 * INCLUDING THE STACK. It is the userspace responsibility
 * to switch the child thread's stack. This requires
 * making sure that the parent thread does not modify
 * the stack until the child thread has switched stack.
 * The child thread usually returns before the parent
 * thread but this behaviour cannot be relied on.
 */
int clone(void)
{
	task_t *new_task, *tmp_task;
	intptr_t kernel_stack;
	vfs_node_t *procfs_node;
	pid_t new_pid, new_tid;
	uint32_t old_stack;
	
	/*
	 * Get new TID, use parent's PID.
	 */
	new_pid = current_task->pid;
	mutex_wait((mutex_t*) &pid_lock);
	new_tid = next_tid++;
	mutex_release((mutex_t*) &pid_lock);

	/* allocate the task_t structure */
	new_task = (task_t*) malloc(sizeof(task_t));
	if (unlikely(new_task == NULL))
		return ENOMEM;

	/* allocate a kernel stack */
	kernel_stack = kalloc(SCHED_KERNEL_STACK_SIZE,
		NULL, NULL, KALLOC_OPTN_ALIGN | KALLOC_OPTN_KERNEL);	
	if (unlikely(kernel_stack == NULL))
	{
		free(new_task);
		return ENOMEM;
	}

	/* allocate space for procfs node */
	procfs_node = (vfs_node_t*) malloc(sizeof(vfs_node_t));
	if (unlikely(procfs_node == NULL))
	{
		kfree((void*)kernel_stack);
		free(new_task);
		return ENOMEM;
	}
	
	/*
	 * Initialize the procfs node and add it to
	 * the procfs file system
	 */
	/* *procfs_node = vfs_node_init(FS_DIRECTORY); */
	strcpy(procfs_node->name, itoa(new_tid));
	procfs_node->parent = procfs;
	procfs_node->open = (open_type_t) &procfs_open;
	procfs_node->stat = (stat_type_t) &procfs_stat;
	procfs_node->readdir = (readdir_type_t) &procfs_readdir;
	procfs_node->flags = FS_DIRECTORY;
	procfs_node->next = NULL;
	vfs_mknod(procfs, procfs_node);

	/*
	 * Initialize the new task structure
	 */
	new_task->id = new_tid;
	new_task->pid = new_pid;
	new_task->uid = current_task->uid;
	new_task->gid = current_task->gid;
	new_task->argv = current_task->argv;
	new_task->envp = current_task->envp;
	new_task->status = TASK_STATE_RUNNING;
	new_task->signal = 0;
	new_task->exit_code = 0;
	new_task->lock = MUTEX_INITIALIZER;
	new_task->env_lock = MUTEX_INITIALIZER;
	new_task->cwd = current_task->cwd;
	new_task->argv = current_task->argv;
	new_task->procfs_node = procfs_node;
	new_task->main_thread = current_task->main_thread;
	new_task->parent = current_task->main_thread;
	new_task->threads = current_task->threads;
	new_task->next_thread = NULL;
	new_task->next = NULL;
	new_task->errno = new_tid;
	new_task->next_child = NULL;
	new_task->children = NULL;
	
	#if 0
	mutex_wait(&current_task->descriptors_info->lock);
	current_task->descriptors_info->refs++;
	mutex_release(&current_task->descriptors_info->lock);
	#endif
	new_task->descriptors_info = current_task->descriptors_info;
	
	/*
	 * Add the task to the list of threads
	 */
	mutex_wait(&current_task->main_thread->lock);
	tmp_task = current_task->threads;
	while (tmp_task->next_thread != NULL)
		tmp_task = tmp_task->next_thread;
	tmp_task->next_thread = new_task;
	mutex_release(&current_task->main_thread->lock);
	
	#if 0
	asm __volatile__("mov %%esp, %0" : "=r" (old_stack));
	memcpy(new_stack, old_stack, ARCH_STACK_START - old_stack);
	#endif
	
	return kfork(new_task, current_directory, 
		arch_get_directory_physical(current_task->machine_state), 
		arch_get_memmap(current_task->machine_state), kernel_stack);	
}