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

static void arch_core_dump(void)
{
	return;
}

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
		
		switch ((unsigned int) current_task->sig_handler[signum])
		{
			case (unsigned int) SIG_DFL:

				/*
				* If the signal has no handler perform the
				* default action
				*/
				switch (signum)
				{
					/*
					* A - Abnormal termination and possibly core dump
					*/
					case SIGABRT:
					case SIGBUS:
					case SIGFPE:
					case SIGILL:
					case SIGQUIT:
					case SIGSEGV:
					case SIGSYS:
					case SIGTRAP:
					case SIGXCPU:
					case SIGXFSZ:
						arch_core_dump();
					/*
					* T - Abnormal Termination
					*/
					case SIGALRM:
					case SIGHUP:
					case SIGINT:
					case SIGKILL:
					case SIGPIPE:
					case SIGTERM:
					case SIGUSR1:
					case SIGUSR2:
					case SIGPOLL:
					case SIGPROF:
					case SIGVTALRM:
						exit(-1);
						break;

					/*
					* S - Stop the process
					*/
					case SIGSTOP:
					case SIGTSTP:
					case SIGTTIN:
					case SIGTTOU:
						current_task->status = TASK_STATE_WAITING;
						schedule();
						break;
						
					/*
					* C - Continue the process
					*/
					case SIGCONT:
						current_task->status = TASK_STATE_RUNNING;
						schedule();
						break;
						
					/*
					* I - Ignore the signal
					*/
					case SIGCHLD:
					case SIGURG:
						break;
				}
				break;
			case (unsigned int) SIG_IGN:
				break;
			case (unsigned int) SIG_ERR:
				assert(current_task->sig_handler[signum] != SIG_ERR);
				break;
			case (unsigned int) SIG_HOLD:
				assert(current_task->sig_handler[signum] != SIG_HOLD);
				break;

			/*
			 * TODO: For now we just call the signal handler
			 * directly so it runs in kernel mode.
			 */
			default:
				current_task->sig_handler[signum](signum);
				break;
				
		}
	}
}

