/*
 * include/arch/x86/scheduler.h
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

#ifndef __ARCH_SCHEDULER_H__
#define __ARCH_SCHEDULER_H__

#include <arch/arch.h>
#include <mmu.h>

/*
 * task state structure
 */
typedef struct
{
	uint32_t eip;
	uint32_t esp;
	uint32_t ebp;
	uint32_t ebx;
	uint32_t esi;
	uint32_t kernel_stack;
	uint32_t directory_physical;
	memmap_t *memmap;
	page_directory_t *directory;
}
task_state_t;

/*
 * jump to a virtual address
 */
#define arch_jump(addr)		asm volatile("jmp *%0" : : "r" (addr))

/*
 * Call a procedure by it's virtual address
 */
#define arch_call(addr)		asm volatile("call *%0" : : "r" (addr))

/*
 * Macros to get/set machine state fields
 */
#define arch_get_kernel_stack(task)			(task.kernel_stack)
#define arch_set_kernel_stack(task, value)		task.kernel_stack = value
#define arch_get_directory(task)				(task.directory)
#define arch_set_directory(task, value)			task.directory = value
#define arch_get_directory_physical(task)		(task.directory_physical)
#define arch_set_directory_physical(task, value)	task.directory_physical = value
#define arch_get_memmap(task)					(task.memmap)
#define arch_set_memmap(task, value)			task.memmap = value

#define arch_get_user_stack()					(current_task->registers->useresp)
#define arch_set_user_stack(stack)				current_task->registers->useresp = (stack)

/*
 * Given an address for a usermode stack this macro returns what
 * the stack pointer would be after the stack is relocated to this
 * buffer. Currently it assumes that the stack grown downwards
 * so it's not very portable. TODO: We need to FIX that.
 */
#define arch_adj_user_stack_pointer(stack)		((stack) - (ARCH_STACK_START - current_task->registers->useresp))

/*
 * Copy the user-mode stack. This is a simple copy of the
 * stack used by clone() to fork new threads. Pointers to
 * stack values will still point to the old stack.
 */
#define arch_copy_user_stack(stack)		\
	do											\
	{											\
		uint32_t *old_stack, *new_stack;				\
		new_stack = (uint32_t*) (stack - (ARCH_STACK_START - current_task->registers->useresp)); \
		old_stack = (uint32_t*) current_task->registers->useresp;	\
		while ((uint32_t) old_stack <= ARCH_STACK_START)	\
			*new_stack++ = *old_stack++;				\
	}											\
	while (0)


/*
 * returns TRUE if the current task has just been
 * resumed by calling arch_scheduler_load_task_state()
 */
#define arch_task_resumed(task)		(task.eip == 0x12345)

/*
 * get the current value of the eip register
 */
#define arch_scheduler_get_ip()		(read_eip())

/*
 * save the current task's eip into a task_state_T
 * structure
 * 
 * NOTE: This needs to be done in two asm directives.
 */
#define arch_scheduler_save_task_ip(task)			\
	asm __volatile__("mov $1f, %%eax; 1:" : : : "eax");	\
	asm __volatile__("mov %%eax, %0;" : "=m" (task.eip) : : "eax")

/*
 * save the current task state (ebp and esp) to a
 * task_state_t structure
 */
#define arch_scheduler_save_task_state(task)		\
	asm __volatile__(							\
		"mov %0, %%eax;"					\
		"mov %%esp, 0x04(%%eax);"			\
		"mov %%ebp, 0x08(%%eax);"			\
		"mov %%ebx, 0x0C(%%eax);"			\
		"mov %%esi, 0x10(%%eax);"			\
		: : "r" (&task) : "eax", "memory" )

/*
 * load a task_state_t structure into the cpu and jump
 * to it's current eip
 */
#define arch_scheduler_load_task_state(task)			\
	asm __volatile__(							\
		"cli;"								\
		"mov %0, current_task;"					\
		"mov 0x04(%0), %%esp;"					\
		"mov 0x08(%0), %%ebp;"					\
		"mov 0x14(%0), %%ebx;"					\
		"mov 0x1C(%0), %%eax;"					\
		"mov %%eax, current_memmap;"				\
		"add %1, %%ebx;"						\
		"mov $tss_entry, %%eax;"					\
		"mov %%ebx, 0x4(%%eax);"					\
		"mov 0x20(%0), %%eax;"					\
		"mov %%eax, current_directory;"			\
		"mov 0x18(%0), %%eax;"					\
		"mov %%eax, %%cr3;"						\
		"mov $0x12345, %%eax;"					\
		"mov 0x0C(%0), %%ebx;"					\
		"mov 0x10(%0), %%esi;"					\
		"mov 0x00(%0), %0;"						\
		"sti;"								\
		"jmp *%0"	: :							\
		"r" (task), 							\
		"i" (SCHED_KERNEL_STACK_SIZE - 4) : 		\
		"memory", "ebx", "esi", "eax" )

/*
 * load a task_state_t structure into the cpu and jump
 * into the specified signal handler
 */
#define arch_scheduler_load_task_state_and_signal(task, sig)	\
	asm __volatile__(					\
		"cli;"						\
		"mov %0, current_task;"			\
		"mov 0x04(%0), %%esp;"			\
		"mov 0x08(%0), %%ebp;"			\
		"mov 0x14(%0), %%ebx;"			\
		"mov 0x1C(%0), %%eax;"			\
		"mov %%eax, current_memmap;"		\
		"mov 0x20(%0), %%eax;"			\
		"mov %%eax, current_directory;"	\
		"add %2, %%ebx;"				\
		"mov $tss_entry, %%eax;"			\
		"mov %%ebx, 0x4(%%eax);"			\
		"mov 0x18(%0), %%eax;"			\
		"mov %%eax, %%cr3;"				\
		"mov %%esp, %%ebx;"				\
		"cmp %%ebp, %%ebx;"				\
		"jge 2f;"						\
		"1:"							\
		"mov (%%ebx), %%eax;"			\
		"mov %%eax, -0xc(%%ebx);"		\
		"add $0x4, %%ebx;"				\
		"cmp %%ebp, %%ebx;"				\
		"jl 1b;"						\
		"2:"							\
		"mov %1, %%eax;"				\
		"mov %%eax, -0x4(%%ebx);"		\
		"mov $3f, %%eax;"				\
		"mov %%eax, -0x8(%%ebx);"		\
		"mov %%ebp, -0xc(%%ebx);"		\
		"sub $0xc, %%esp;"				\
		"sub $0xc, %%ebp;"				\
		"mov 0x0c(%0), %%ebx;"			\
		"mov 0x10(%0), %%esi;"			\
		"mov 0x00(%0), %0;"				\
		"mov $0x12345, %%eax;"			\
 		"sti;"						\
		"jmp *%0;"					\
		"3:"							\
		"call signal_handler;"			\
		"add $0x4, %%esp;"				\
		"pop %%ebp;"					\
		"ret;" : :					\
		"r" (task), 					\
		"r" (sig), 					\
		"i" (SCHED_KERNEL_STACK_SIZE - 4) : \
		"memory", "cc", "esi", "ebx", "eax" )

/*
 * Switch to user mode and call a function.
 * 
 * Sets usermode stack as follows:
 * 
 * +-----------+
 * |    eip    |	--> kernel eip
 * +-----------+
 * |    esp    |	--> kernel stack
 * +-----------+
 * |    argv   |	--> arguments data
 * +-----------+
 * |   *argv   |	--> arguments string table
 * +-----------+
 * |    argc   | 	--> argument count
 * +-----------+
 * |   *main   |	--> program entry point
 * +-----------+
 * 
 * Then switches to usermode and transfers control 
 * to user_entry().
 * 
 */
#define arch_enter_user_mode(stack, vaddr, argc, argv, len)	\
	asm __volatile__(								\
		"mov %%esp, %%ebx;"							\
		"mov %0, %%esp;"							\
		"1:"										\
		"push $1b;"								\
		"push %%ebx;"								\
		"sub %4, %%esp;"							\
		"mov %%esp, %%ebp;"							\
		"push %3;"								\
		"test %4, %4;"								\
		"jz 3f;"									\
		"2:"										\
		"mov (%3), %%eax;"							\
		"mov %%eax, 0x4(%%ebp);"						\
		"add $0x4, %%ebp;"							\
		"add $0x4, %3;"							\
		"sub $0x4, %4;"							\
		"jnz 2b;"									\
		"3:"										\
		"pop %3;"									\
		"lea 0x4(%%esp), %%eax;"						\
		"push %%eax;"								\
		"push %2;"								\
		"push %1;"								\
		"mov %%eax, %1;"							\
		"sub %3, %%eax;"							\
		"test %2, %2;"								\
		"jz 5f;"									\
		"4:"										\
		"mov (%1), %3;"							\
		"add %%eax, %3;"							\
		"mov %3, (%1);"							\
		"add $0x4, %1;"							\
		"dec %2;"									\
		"jnz 4b;"									\
		"5:"										\
		"mov %%esp, %2;"							\
		"sub $0x4, %2;"							\
		"mov %%ebx, %%esp;"							\
												\
		"pushf;"			/* eflags */				\
		"pop %%eax;"		/* read eflags */			\
		"or $0x200, %%eax;"	/* sti on iret */			\
												\
		"pushl $0x23;" 	/* ss */					\
		"pushl %2;"		/* esp */					\
		"push %%eax;"		/* eflags */				\
		"pushl $0x1B;"		/* cs */					\
		"push $user_entry;"	/* eip */					\
		"cli;"									\
		"mov $0x23, %%ax;"							\
		"mov %%ax, %%ds;"							\
		"mov %%ax, %%es;"							\
		"mov %%ax, %%fs;"							\
		"mov %%ax, %%gs;"							\
		"iret;" : :								\
		"i" (stack), 								\
		"r" (vaddr), 								\
		"r" (argc), 								\
		"r" (argv), 								\
		"r" (len) : 	 							\
		"cc", "memory", "eax", "ebx" )

/*
 * Perform arch-specific scheduler 
 * initialization.
 */
void arch_scheduler_init(void);

/*
 * Relocate the stack
 */
void arch_move_stack(void *new_stack_start, uint32_t size);

#if 1
struct __task;

void arch_scheduler_move_the_fucking_task_stack(struct __task *new_task);
#endif
                
#endif
