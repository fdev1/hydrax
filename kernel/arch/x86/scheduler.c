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
 * 
 */

#include <arch/arch.h>
#include <arch/scheduler.h>
#include <mmu.h>
#include <printk.h>
#include <memory.h>
#include <unistd.h>
#include <scheduler.h>
#include <signal.h>

/*
 * debug symbols
 */
#define SCHED_DEBUG_MOVE_STACK		(0)
#define SCHED_DEBUG_MOVE_TASK_STACK 	(0)

extern uint32_t initial_esp;

/*
 * general protection exception handler
 */
static void general_protection_exception(registers_t *regs)
{
	raise(SIGSEGV);
	schedule();
}

/*
 * Divide error handler
 */
void divide_error(registers_t *regs)
{
	raise(SIGILL);
	schedule();
}

void breakpoint(registers_t *regs)
{
	raise(SIGINT);
	schedule();
}

void overflow(registers_t *regs)
{
	raise(SIGILL);
	schedule();
}

void bound_range_exceeded(registers_t *regs)
{
	raise(SIGILL);
	schedule();
}

void invalid_opcode(registers_t *regs)
{
	raise(SIGILL);
	schedule();
}

void double_fault(registers_t *regs)
{
	printk(1, "double fault!");
	raise(SIGILL);
	schedule();
}

void stack_exception(registers_t *regs)
{
	raise(SIGSEGV);
	schedule();
}


/*
 * move the stack
 */
void arch_move_stack(void *new_stack_start, uint32_t size)
{
	if (SCHED_DEBUG_MOVE_STACK)
		printk(8, "sched_move_stack: mvoing stack to 0x%x (size: %i)", 
			(intptr_t) new_stack_start, size);

	uint32_t i, phys, virt;

	/* make sure new stack address is word aligned */
	assert(((uint32_t) new_stack_start % 4) == 0);

	/*
	 * just a hack for now
	 */
	if (SCHED_DEBUG_MOVE_STACK)
		printk(8, "sched_move_stack: pre-making stack pages.");
	for (i = (uint32_t) new_stack_start & 0xFFFFF000; i >= ((uint32_t)new_stack_start - size); i -= 0x1000)
	{
		if (SCHED_DEBUG_MOVE_STACK)
			printk(8, "sched_move_stack: making 0x%x", i);
		mmu_make_page(i);
	}
	
	
	/* allocate stack in user memory */
	virt = kalloc(size, i + 0x1000, &phys, 
		KALLOC_OPTN_ALIGN | KALLOC_OPTN_NOCOW | KALLOC_OPTN_NOSHARE | KALLOC_OPTN_NOFREE);
	assert(virt != NULL);

	if (SCHED_DEBUG_MOVE_STACK)
		printk(8, "sched_move_stack: allocated stack at 0x%x-0x%x (phys 0x%x)", 
			i+0x1000, i + 0x1000 + size - 1, phys);

	//mmu_dump_page(virt);
	//mmu_dump_page((uint32_t)new_stack_start);

	// Flush the TLB by reading and writing the page directory address again.
	#if 1
	uint32_t pd_addr;
	asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
	asm volatile("mov %0, %%cr3" : : "r" (pd_addr));
	#else
	asm volatile(
		"mov %cr3, %eax;"
		"mov %eax, %cr3;" : : : "eax");
	#endif

	//printk(8, "sched_move_stack: TLB flushed");

	// Old ESP and EBP, read from registers.
	uint32_t old_stack_pointer; asm volatile("mov %%esp, %0" : "=r" (old_stack_pointer));
	uint32_t old_base_pointer;  asm volatile("mov %%ebp, %0" : "=r" (old_base_pointer));
	uint32_t offset            = (uint32_t) new_stack_start - initial_esp;
	uint32_t new_stack_pointer = old_stack_pointer + offset;
	uint32_t new_base_pointer  = old_base_pointer  + offset;

	// Copy the stack.
	memcpy((void*) new_stack_start - size + 4, (void*)initial_esp - size + 4, size);

	if (SCHED_DEBUG_MOVE_STACK)
	{
		printk(8, "sched_move_stack: src: 0x%x-0x%x",
			new_stack_start - size + 4, new_stack_start + 4);
		printk(8, "sched_move_stack: dst: 0x%x-0x%x",
			initial_esp, initial_esp);
		printk(8, "sched_move_stack: correcting 0x%x-0x%x", 
			new_stack_start - size + 4, new_stack_start);
		printk(8, "sched_move_stack: values: 0x%x-0x%x", 
			old_stack_pointer, initial_esp);
	}

	/*
	 * walk the new stack and corrent any pointers to the old stack
	 */
	for (i = (uint32_t) new_stack_start - size + 4; i < (uint32_t) new_stack_start; i += 4)
	{
		uint32_t tmp = *(uint32_t*) i;
		if ((old_stack_pointer < tmp) && (tmp < initial_esp))
		{
			uint32_t *tmp2 = (uint32_t*) i;
			*tmp2 = tmp + offset;

			if (SCHED_DEBUG_MOVE_STACK)
				printk(8, "sched_move_stack: changing 0x%x to 0x%x",
					tmp, tmp + offset);
		}
	}

	if (SCHED_DEBUG_MOVE_STACK)
		printk(8, "sched_move_stack: switching stack.");

	/* switch stack */
	asm volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
	asm volatile("mov %0, %%ebp" : : "r" (new_base_pointer));
}

/*
 * Moves the kernel stack for the new task and
 * sets the registers pointer in the task structure
 * to point to the new stack.
 */
void arch_scheduler_move_the_fucking_task_stack(task_t *new_task)
{
	uint32_t* stack, *stack2;
	uint32_t stack_end, stack2_end;
	stack_end = new_task->machine_state.kernel_stack;
	stack = (uint32_t*) (stack_end + (SCHED_KERNEL_STACK_SIZE -4));
	
	if (SCHED_DEBUG_MOVE_TASK_STACK)
		printk(7, "fork: new stack at 0x%x-0x%x", stack_end, stack);
	
	if (current_task->id == 0)
	{
		stack2 = (uint32_t*) ARCH_STACK_START;
		stack2_end = (uint32_t) new_task->machine_state.esp;
	}
	else
	{
		stack2 = (uint32_t*) (current_task->machine_state.kernel_stack + (SCHED_KERNEL_STACK_SIZE - 4));
		stack2_end = (uint32_t) new_task->machine_state.esp;
	}

	if (SCHED_DEBUG_MOVE_TASK_STACK)
		printk(7, "fork: pid=%i original stack: 0x%x-0x%x",
			getpid(), (uint32_t) stack2_end, (uint32_t) stack2);

	int64_t tmpi, a = 0, b = 0;
	uint32_t *pa = (uint32_t*) &a, *pb = (uint32_t*) &b;
	*pa = (uint32_t) stack;
	*pb = (uint32_t) stack2;
	tmpi = a - b;
	
	uint32_t start;
	start = (uint32_t) stack2;
	
	new_task->registers = (registers_t*) (uint32_t) ((uint32_t) current_task->registers + tmpi);
	
	if (SCHED_DEBUG_MOVE_TASK_STACK)
		printk(7, "fork: moving 0x%x-0x%x to 0x%x-0x%x",
			start, stack2_end, (uint32_t) stack, stack_end);
	
	while ((uint32_t) stack != stack_end && (uint32_t) stack2 != stack2_end)
	{
		if (*stack2 <= start && 	*stack2 >= stack2_end)
		{
			*stack = *stack2 + tmpi;
			if (SCHED_DEBUG_MOVE_TASK_STACK)
				printk(7, "fork: moved %x to %x (0x%x->0x%x)", 
					(uint32_t) stack2, (uint32_t) stack,
					*stack2, *stack2 + tmpi);
		}
		else
			*stack = *stack2;
		stack--;
		stack2--;
	}
	new_task->machine_state.esp += tmpi;
	new_task->machine_state.ebp += tmpi;
	
}

/*
 * Perform arch-specific scheduler initialization.
 */
void arch_scheduler_init(void)
{
	register_isr(0x0, &divide_error);
	register_isr(0x1, &general_protection_exception);	
	register_isr(0x2, &general_protection_exception);	
	register_isr(0x3, &breakpoint);
	register_isr(0x4, &overflow);
	register_isr(0x5, &bound_range_exceeded);
	register_isr(0x6, &invalid_opcode);
	register_isr(0x7, &general_protection_exception);	
	register_isr(0x8, &double_fault);
	register_isr(0x9, &general_protection_exception);	
	register_isr(0xA, &general_protection_exception);	
	register_isr(0xB, &general_protection_exception);	
	register_isr(0xC, &stack_exception);
	register_isr(0xD, &general_protection_exception);	
}

