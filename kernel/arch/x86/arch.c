/*
 * platform.c
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

#include <arch/arch.h>
#include <printk.h>
#include <symbols.h>
#include <unistd.h>
#include <syscall.h>
#include <scheduler.h>

/*
 * Write a byte to the specified port
 */
void outb(uint16_t port, uint8_t value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

/*
 * Read 16 bits from the specified IO port.
 */
uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

/*
 * If a new thread exits it'll return to this
 * function. It calls the pthread_exit system call
 * from userland.
 */
int __usermode pthread_uexit(void)
{
	asm __volatile__(			\
		"int $0x30;" : : "a" (SYSCALL_PTHREAD_EXIT), "b" (-1));
	return -1;
}


/*
 * Dump the contents of the stack on screen.
 */
void arch_dump_stack(unsigned int len)
{
	uint32_t *stack;
	asm volatile("mov %%esp, %0" : "=r" (stack));


	//arch_disable_interrupts();
	printk(8, "stack dump:");
	printk(8, "===========");

	while (len--)
	{
		printk(8, "0x%x: 0x%x\t0x%x: 0x%x", (uint32_t) stack, *stack, (uint32_t) &stack[5], stack[5]);
		stack++;
	}
	//arch_enable_interrupts();
}

void arch_dump_stack_trace(void)
{
	uint32_t *ebp;
	uint32_t levels = 50;
	uint32_t eip;
	asm volatile("mov %%ebp, %0" : "=r" (ebp));

	printk(7, "stack trace: (pid=%i)", getpid());

	/*
	* note:
	* at the moment kernel space is the 4MB of the address
	* so we stop there.
	*/
	while (levels--)
	{
		eip = ebp[1];
		if (eip < 0x100000)
			break;
		printk(7, "ebp=0x%x eip=0x%x sym=%s", ebp, eip, getsym(eip));
		ebp = (uint32_t*) ebp[0];
	}
	/* printk(7, "ebp=0x%x eip=0x%x", (uint32_t) ebp, eip); */
}

/*
 * panic
 */
void arch_panic(const char *message, const char *file, uint32_t line)
{
	arch_disable_interrupts();
	printk_delay(1000000);
	kprintf("#######################################################\n", 0);
	kprintf("##                    P A N I C !                    ##\n", 0);
	kprintf("#######################################################\n", 0);
	kprintf("\n", 0);
	kprintf("%s at %s: %i\n", message, file, line);
	arch_dump_stack_trace();
	arch_halt();
}

void arch_queue_user_task(void *task_fn, void *ret_fn, uint32_t argc, uint32_t *argv)
{
	uint32_t *pstack;
	
	
	pstack = (uint32_t*) current_task->registers->useresp;
	
	if (ret_fn == NULL)
		ret_fn = (void*) current_task->registers->eip;
	
	*pstack-- = (uint32_t) pstack;
	
	while (argc--)
		*pstack-- = (uint32_t) argv;
	
	*pstack-- = (uint32_t) ret_fn;
	
	current_task->registers->useresp = (uint32_t) pstack;
	current_task->registers->eip = (uint32_t) task_fn;
	
}


/*
 * reboot the system.
 */
void reboot(void)
{
	uint8_t good = 0x02;
	while (good & 0x02)
		good = inb(0x64);
	outb(0x64, 0xFE);
	arch_halt();
}
