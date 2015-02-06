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
#include <errno.h>

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
	
	while (current_task->sig_pending)
	{
		/*
		 * if there is a signal pending handle it
		 */
		signum = 0;
		if (likely(current_task->sig_pending))
		{
			sigtmp = current_task->sig_pending & ~current_task->sigmask;
			while (1)
			{
				if (unlikely(sigtmp & 1))
				{
					current_task->sig_pending &= ~(1 << signum);
					current_task->sig_delivered |= (1 << signum);
					break;
				}
				sigtmp >>= 1;
				signum += 1;
			}
			signum++;
		}
		
		switch ((unsigned int) current_task->sig_handler[signum - 1])
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

/*
 * Modifies the disposition of a signal
 */
sighandler_t sigset(int sig, sighandler_t disp)
{
	sighandler_t old_disp;
	if (unlikely(sig < 1 || sig > 32))
		return SIG_ERR;
	old_disp = current_task->sig_handler[sig];
	current_task->sig_handler[sig] = disp;
	return old_disp;
}

/*
 * Add a signal to the calling process' signal
 * mask.
 */
int sighold(int sig)
{
	if (unlikely(sig < 1 || sig > 32))
		return -1;
	current_task->sigmask |= (1 << (sig - 1));
	return ESUCCESS;
}

/*
 * Remove a signal from the calling process' signal
 * mask.
 */
int sigrelse(int sig)
{
	if (unlikely(sig < 1 || sig > 32))
		return -1;
	current_task->sigmask &= ~(1 << (sig - 1));
	return ESUCCESS;
}

/*
 * Sets the disposition of a signal to SIG_IGN
 */
int sigignore(int sig)
{
	if (unlikely(sig < 1 || sig > 32))
		return -1;
	current_task->sig_handler[sig] = SIG_IGN;
}

/*
 * Examine and change blocked signals.
 */
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	*oldset = current_task->sigmask;
	switch (how)
	{
		case SIG_BLOCK:
			current_task->sigmask |= *set;
			return ESUCCESS;
		case SIG_UNBLOCK:
			current_task->sigmask &= ~(*set);
			return ESUCCESS;
		case SIG_SETMASK:
			current_task->sigmask = *set;
			return ESUCCESS;
		default:
			return EINVAL;
	}
}

/*
 * Returns the set of signals that are pending for delivery
 * to the current thread.
 */
int sigpending(sigset_t *set)
{
	if (set == NULL)
		return EFAULT;
	*set = current_task->sig_pending;
	return ESUCCESS;
}
