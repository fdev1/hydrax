/*
 * signal.c
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
#include <signal.h>
#include <unistd.h>
#include <printk.h>
#include <scheduler.h>

/*
 * This is the default signal handler. It gets executed
 * in kernel mode when a signal is received. From here we must
 * call the task registered signal handler and we must switch
 * to user mode first.
 */
void signal_handler(int sigmask)
{
	uint32_t sigtmp, signum;
	
	sigmask = 0;
	
	while (current_task->signal)
	{
		/*
		* if there is a signal pending handle it
		*/
		signum = 0;
		if (likely(current_task->signal))
		{
			sigtmp = current_task->signal;
			while (1)
			{
				if (unlikely(sigtmp & 1))
				{
					current_task->signal &= ~(1 << signum);
					break;
				}
				sigtmp >>= 1;
				signum += 1;
			}
			signum++;
		}
		
		/*
		 * TODO: For now we just call the signal handler
		 * directly so it runs in kernel mode.
		 */
		if (current_task->sig_handler[signum] != NULL)
		{
			current_task->sig_handler[signum](signum);
			continue;
		}

		/*
		 * If the signal has no handler perform the
		 * default action
		 */
		switch (signum)
		{
			case SIGILL:
				printk(7, "SIGILL received by pid %i tid %i", getpid(), gettid());
				arch_dump_stack_trace();
				exit(-1);
				break;

			case SIGSEGV:
				printk(7, "SIGSEGV received by pid %i tid %i", getpid(), gettid());
				exit(-1);
				break;
				
			case SIGSTOP:
				current_task->status = TASK_STATE_WAITING;
				schedule();
				break;
				
			case SIGCONT:
				current_task->status = TASK_STATE_RUNNING;
				schedule();
				break;
				
			case SIGQUIT:
			case SIGTERM:
			case SIGKILL:
				printk(7, "SIGKILL received by pid %i tid %i", getpid(), gettid());
				exit(-1);
				break;
							
			case SIGINT:
				printk(7, "SIGINT received by pid %i", getpid());
				break;

			case SIGALRM:
			case SIGCHLD:
				break;

			default:
				if (signum != 30)
				{
					printk(7, "pid %i received signal %i", getpid(), signum);
				}
		}
	}
}

