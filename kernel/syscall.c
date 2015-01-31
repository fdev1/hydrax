/*
 * syscall.c
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
#include <arch/stdarg.h>
#include <syscall.h>
#include <isr.h>
#include <scheduler.h>
#include <printk.h>
#include <io.h>
#include <time.h>
#include <errno.h>

/*
 * test system call
 */
/*static*/ int syscall0(char *s)
{
	kprintf("%s\n", s);
	arch_dump_stack_trace();
	return 0;
}

typedef struct
{
	void *entry;
	unsigned int argc;
}
syscall_t;

/* 
 * system call table
 */
static syscall_t syscalls[] =
{
	{ &syscall0, 1 },
	{ &fork, 0 },
	{ &exit, 1 },
	{ &getpid, 0 },
	{ &getuid, 0 },
	{ &getpid, 0 },
	{ &open, 2 },
	{ &close, 1 },
	{ &write, 3 },
	{ &read, 3 },
	{ &kill, 2 },
	{ &readdir, 3 },
	{ &reboot, 0 },
	{ &stat, 2 },
	{ &time, 1 },
	{ &chdir, 1 },
	{ &getcwd, 2 },
	{ &execve, 3 },
	{ &waitpid, 3 },
	{ &setenv, 3 },
	{ &unsetenv, 1 },
	{ &setuid, 1 },
	{ &setgid, 1 },
	{ &geteuid, 0 },
	{ &getegid, 0 },
	{ &seteuid, 1 },
	{ &setegid, 1 },
	{ &pipe, 1 },
	{ &vfork, 0 },
	{ &ioctl, 4 },		/* long arg = 2 */
	{ &dup, 1 },
	{ &yield, 0 },
	{ &clone, 1 },
	{ &gettid, 0 },
};

/*
 * performs a system call.
 * This function is meant to be called from userland only
 * and will eventually be removed from the kernel.
 */
int syscall(unsigned int num, ...)
{
	int ret;
	va_list args;
	unsigned int arg1, arg2, arg3;
	
	switch (syscalls[num].argc)
	{
		case 0:
			asm volatile("int $0x30" : "=a" (ret) : "0" (num));
			break;
		case 1:
			va_start(args, num);
			arg1 = (unsigned int) va_arg(args, unsigned int);
			asm volatile("int $0x30" : "=a" (ret) : "0" (num), "b" (arg1));
			return ret;
		case 2:
			va_start(args, num);
			arg1 = (unsigned int) va_arg(args, unsigned int);
			arg2 = (unsigned int) va_arg(args, unsigned int);
			asm volatile("int $0x30" : "=a" (ret) : "0" (num), "b" (arg1), "c" (arg2));
			return ret;
		case 3:
			va_start(args, num);
			arg1 = (unsigned int) va_arg(args, unsigned int);
			arg2 = (unsigned int) va_arg(args, unsigned int);
			arg3 = (unsigned int) va_arg(args, unsigned int);
			asm volatile("int $0x30" : "=a" (ret) : "0" (num), "b" (arg1), "c" (arg2), "d" (arg3));
			return ret;
		default:
			assert(0);
			return 0;

	}
	return 0;	/* clang	 shows warning without this */
}

/*
 * syscall handler
 */
static void syscall_handler(registers_t *regs)
{
	int ret;

	if (unlikely(regs->eax >= SYSCALL_COUNT))
	{
		regs->eax = ENOSYS;
		return;
	}
	
	/*
	 * This allows easy access to the user-mode stack.
	 */
	current_task->registers = regs;

	asm __volatile__(
		"push %1;"
		"push %2;"
		"push %3;"
		"push %4;"
		"push %5;"
		"call *%6;"
		"add $0x14, %%esp"
		: "=a" (ret)
		: "r" (regs->edi), "r" (regs->esi), "r" (regs->edx),
			"r" (regs->ecx), "r" (regs->ebx), "r" (syscalls[regs->eax].entry));
	regs->eax = ret;
}

/*
 * initialize syscall interface
 */
void syscall_init(void)
{
	register_isr(0x30, (void*) &syscall_handler);
}

