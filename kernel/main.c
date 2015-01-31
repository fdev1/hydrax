/*
 * main.c
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
#include <arch/cpu.h>
#include <console.h>
#include <printk.h>
#include <isr.h>
#include <mmu.h>
#include <vfs.h>
#include <initrd.h>
#include <multiboot.h>
#include <timer.h>
#include <kheap.h>
#include <scheduler.h>
#include <symbols.h>
#include <kbd.h>
#include <idle.h>
#include <syscall.h>
#include <elfldr.h>
#include <devfs.h>
#include <tty.h>
#include <arch/rtc.h>
#include <unistd.h>

uint32_t initial_esp;

/*
 * kernel entry point
 */
int hydrax_init(multiboot_header_t *p_mboot, uint32_t initial_stack)
{
	initial_esp = initial_stack;
	
	/*
	 * initialize critical systems
	 */
	console_early_init();
	cpu_init();
	mmu_init(p_mboot);
	kheap_init();
	vfs_init(initrd_init(*((uint32_t*) p_mboot->mods_addr)));
	devfs_init();
	kbd_init();
	console_init();
	tty_init();
	symbols_init();
	scheduler_init();
	timer_init(50);
	syscall_init();
	rtc_init();

	/*
	 * debug tests
	 */
	printk(8, "symbol test: %s", getsym((uint32_t) &initial_esp));

	/*
	 * exec pid 1
	 */
	if (!fork())
	{
		/*
		 * Open standard file descriptors
		 */
		if (unlikely(open("/dev/tty", NULL) != 0))
			panic("Could not open fd 0.");
		if (unlikely(open("/dev/tty", NULL) != 1))
			panic("Could not open fd 1.");
		if (unlikely(open("/dev/tty", NULL) != 2))
			panic("Could not open fd 2.");
		
		/*
		 * Execute /sbin/init
		 */
		execve("/sbin/init", NULL, NULL);
		panic("Could not execute init.");
	}

	/*
	 * Run idle loop
	 */
	idle();

	return 0;
}
