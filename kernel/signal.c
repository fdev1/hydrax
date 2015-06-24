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
#include <memory.h>

#define queue_signal(task, sig)							\
if (sig)												\
do													\
{													\
	const uint32_t sigbit = 1 << (sig - 1);					\
	if (likely((task->sig_pending[0] & sigbit) == 0))			\
	{												\
		task->sig_pending[0] |= sigbit;					\
		task->sig_info[sig - 1][0].si_value.sival_int = 0;			\
		task->sig_info[sig - 1][0].si_signo = sig;			\
		task->sig_info[sig - 1][0].si_uid = current_task->euid;\
		task->sig_info[sig - 1][0].si_pid = current_task->pid;	\
		task->sig_info[sig - 1][0].si_errno = 0;			\
		task->sig_info[sig - 1][0].si_code = SI_USER;		\
		task->sig_info[sig - 1][0].si_addr = NULL;			\
		task->sig_info[sig - 1][0].si_band = 0;				\
		task->sig_info[sig - 1][0].si_errno = 0;			\
		task->sig_info[sig - 1][0].si_status = 0;			\
	}												\
	else if (likely((task->sig_pending[1] & sigbit) == 1))		\
	{												\
		task->sig_pending[1] |= sigbit;					\
		task->sig_info[sig - 1][1].si_value.sival_int = 0;			\
		task->sig_info[sig - 1][1].si_signo = sig;			\
		task->sig_info[sig - 1][1].si_uid = current_task->euid;\
		task->sig_info[sig - 1][1].si_pid = current_task->pid;	\
		task->sig_info[sig - 1][1].si_errno = 0;			\
		task->sig_info[sig - 1][1].si_code = SI_USER;		\
		task->sig_info[sig - 1][1].si_addr = NULL;			\
		task->sig_info[sig - 1][1].si_band = 0;				\
		task->sig_info[sig - 1][1].si_errno = 0;			\
		task->sig_info[sig - 1][1].si_status = 0;			\
	}												\
	else if (likely((task->sig_pending[2] & sigbit) == 1))		\
	{												\
		task->sig_pending[2] |= sigbit;					\
		task->sig_info[sig - 1][2].si_value.sival_int = 0;			\
		task->sig_info[sig - 1][2].si_signo = sig;			\
		task->sig_info[sig - 1][2].si_uid = current_task->euid;\
		task->sig_info[sig - 1][2].si_pid = current_task->pid;	\
		task->sig_info[sig - 1][2].si_errno = 0;			\
		task->sig_info[sig - 1][2].si_code = SI_USER;		\
		task->sig_info[sig - 1][2].si_addr = NULL;			\
		task->sig_info[sig - 1][2].si_band = 0;				\
		task->sig_info[sig - 1][2].si_errno = 0;			\
		task->sig_info[sig - 1][2].si_status = 0;			\
	}												\
	else if (likely((task->sig_pending[3] & sigbit) == 1))		\
	{												\
		task->sig_pending[3] |= sigbit;					\
		task->sig_info[sig - 1][3].si_value.sival_int = 0;	\
		task->sig_info[sig - 1][3].si_signo = sig;			\
		task->sig_info[sig - 1][3].si_uid = current_task->euid;\
		task->sig_info[sig - 1][3].si_pid = current_task->pid;	\
		task->sig_info[sig - 1][3].si_errno = 0;			\
		task->sig_info[sig - 1][3].si_code = SI_USER;		\
		task->sig_info[sig - 1][3].si_addr = NULL;			\
		task->sig_info[sig - 1][3].si_band = 0;				\
		task->sig_info[sig - 1][3].si_errno = 0;			\
		task->sig_info[sig - 1][3].si_status = 0;			\
	}												\
}													\
while (0)
		

void schedule_prelocked(void);

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
	uint32_t sigtmp, sig;
	siginfo_t siginfo;
	
	sigmask = 0;
	sigtmp = current_task->sig_pending[0] & ~current_task->sigmask;
	while (sigtmp)
	{
		/*
		 * if there is a signal pending handle it
		 */
		sig = 0;
		while (1)
		{
			if (unlikely(sigtmp & 1))
			{
				const uint32_t sigbit = 1 << sig;
				
				if (unlikely(current_task->sig_pending[3] & sigbit))
					current_task->sig_pending[3] &= ~sigbit;
				else if (unlikely(current_task->sig_pending[2] & sigbit))
					current_task->sig_pending[2] &= ~sigbit;
				else if (unlikely(current_task->sig_pending[1] & sigbit))
					current_task->sig_pending[1] &= ~sigbit;
				else if (likely(current_task->sig_pending[0] & sigbit))
					current_task->sig_pending[0] &= ~sigbit;
				
				siginfo = current_task->sig_info[sig][0];
				current_task->sig_info[sig][0] = current_task->sig_info[sig][1];
				current_task->sig_info[sig][1] = current_task->sig_info[sig][2];
				current_task->sig_info[sig][2] = current_task->sig_info[sig][3];

				if (likely((current_task->sig_delivered[0] & sigbit) == 0))
					current_task->sig_delivered[0] |= sigbit;
				else if (likely((current_task->sig_delivered[1] & sigbit) == 0))
					current_task->sig_delivered[1] |= sigbit;
				else if (likely((current_task->sig_delivered[2] & sigbit) == 0))
					current_task->sig_delivered[2] |= sigbit;
				else if (likely((current_task->sig_delivered[3] & sigbit) == 0))
					current_task->sig_delivered[3] |= sigbit;
				
				break;
			}
			sigtmp >>= 1;
			sig++;
		}
		sig++;
		
		switch ((unsigned int) current_task->sig_action[sig - 1].sa_handler)
		{
			case (unsigned int) SIG_DFL:

				/*
				 * If the signal has no handler perform the
				 * default action
				 */
				switch (sig)
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
				assert(current_task->sig_action[sig - 1].sa_handler != SIG_ERR);
				break;
			case (unsigned int) SIG_HOLD:
				assert(current_task->sig_action[sig - 1].sa_handler != SIG_HOLD);
				break;

			/*
			 * TODO: For now we just call the signal handler
			 * directly so it runs in kernel mode.
			 */
			default:
				current_task->sig_action[sig - 1].sa_handler(sig);
				break;
				
		}
		sigtmp = current_task->sig_pending[0] & ~current_task->sigmask;
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
	old_disp = current_task->sig_action[sig - 1].sa_handler;
	current_task->sig_action[sig - 1].sa_handler = disp;
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
	current_task->sig_action[sig - 1].sa_flags = 0;
	current_task->sig_action[sig - 1].sa_mask = 0;
	current_task->sig_action[sig - 1].sa_handler = SIG_IGN;
	return ESUCCESS;
}

/*
 * Examine and change blocked signals.
 */
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	if (likely(oldset != NULL))
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
	if (unlikely(set == NULL))
		return EFAULT;
	*set = *current_task->sig_pending;
	return ESUCCESS;
}

/*
 * set a signal handler
 */
sighandler_t signal(int sig, sighandler_t disp)
{
	if (unlikely(sig < 1 || sig > 32 || sig == SIGKILL || sig == SIGSTOP))
		return (sighandler_t) EINVAL;
	current_task->sig_action[sig - 1].sa_flags = 0;
	current_task->sig_action[sig - 1].sa_mask = 0;
	current_task->sig_action[sig - 1].sa_handler = disp;
	return (sighandler_t) ESUCCESS;
}

/*
 * Queue a signal and data to a process.
 */
int sigqueue(pid_t tid, int sig, const union sigval value)
{
	task_t *tmp = NULL;
	const uint32_t sigbit = 1 << (sig - 1);
	LINKED_LIST_ITER_STRUCT(task_t) iter;

	if (unlikely(sig < 1 || sig > 32))
		return EINVAL;
	
	iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
	LINKED_LIST_SEEK_UNTIL(iter, iter.current->val->id == tid);
	if (unlikely(iter.current == NULL))
		return ESRCH;
	tmp = iter.current->val;
	
	if (likely((tmp->sig_pending[0] & sigbit) == 0))
	{
		tmp->sig_pending[0] |= sigbit;
		tmp->sig_info[sig - 1][0].si_value = value;
		tmp->sig_info[sig - 1][0].si_signo = sig;
		tmp->sig_info[sig - 1][0].si_uid = current_task->euid;
		tmp->sig_info[sig - 1][0].si_pid = current_task->pid;
		tmp->sig_info[sig - 1][0].si_errno = 0;
		tmp->sig_info[sig - 1][0].si_code = SI_QUEUE;
		tmp->sig_info[sig - 1][0].si_addr = NULL;
		tmp->sig_info[sig - 1][0].si_band = 0;
		tmp->sig_info[sig - 1][0].si_errno = 0;
		tmp->sig_info[sig - 1][0].si_status = 0;
	}
	else if (likely((tmp->sig_pending[1] & sigbit) == 0))
	{
		tmp->sig_pending[1] |= sigbit;
		tmp->sig_info[sig - 1][1].si_value = value;
		tmp->sig_info[sig - 1][1].si_signo = sig;
		tmp->sig_info[sig - 1][1].si_uid = current_task->euid;
		tmp->sig_info[sig - 1][1].si_pid = current_task->pid;
		tmp->sig_info[sig - 1][1].si_errno = 0;
		tmp->sig_info[sig - 1][1].si_code = SI_QUEUE;
		tmp->sig_info[sig - 1][1].si_addr = NULL;
		tmp->sig_info[sig - 1][1].si_band = 0;
		tmp->sig_info[sig - 1][1].si_errno = 0;
		tmp->sig_info[sig - 1][1].si_status = 0;
	}
	else if (likely((tmp->sig_pending[2] & sigbit) == 0))
	{
		tmp->sig_pending[2] |= sigbit;
		tmp->sig_info[sig - 1][2].si_value = value;
		tmp->sig_info[sig - 1][2].si_signo = sig;
		tmp->sig_info[sig - 1][2].si_uid = current_task->euid;
		tmp->sig_info[sig - 1][2].si_pid = current_task->pid;
		tmp->sig_info[sig - 1][2].si_errno = 0;
		tmp->sig_info[sig - 1][2].si_code = SI_QUEUE;
		tmp->sig_info[sig - 1][2].si_addr = NULL;
		tmp->sig_info[sig - 1][2].si_band = 0;
		tmp->sig_info[sig - 1][2].si_errno = 0;
		tmp->sig_info[sig - 1][2].si_status = 0;
	}
	else if (likely((tmp->sig_pending[3] & sigbit) == 0))
	{
		tmp->sig_pending[3] |= sigbit;
		tmp->sig_info[sig - 1][3].si_value = value;
		tmp->sig_info[sig - 1][3].si_signo = sig;
		tmp->sig_info[sig - 1][3].si_uid = current_task->euid;
		tmp->sig_info[sig - 1][3].si_pid = current_task->pid;
		tmp->sig_info[sig - 1][3].si_errno = 0;
		tmp->sig_info[sig - 1][3].si_code = SI_QUEUE;
		tmp->sig_info[sig - 1][3].si_addr = NULL;
		tmp->sig_info[sig - 1][3].si_band = 0;
		tmp->sig_info[sig - 1][3].si_errno = 0;
		tmp->sig_info[sig - 1][3].si_status = 0;
	}
	else
	{
		return EAGAIN;
	}
	schedule();
	return ESUCCESS;
}

/*
 * 
 */
int pthread_kill(pthread_t t, int signum)
{
	int r;
	mutex_wait(&schedule_lock);
	r = pthread_kill_nolock(t, signum);
	mutex_release(&schedule_lock);
	return r;
}

/*
 * Send a signal to a specific thread.
 * If tid is 0 the signal is sent to allocate
 * threads.
 */
int pthread_kill_nolock(pthread_t tid, int sig)
{
	LINKED_LIST_ITER_STRUCT(task_t) iter;
	iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
	
	if (tid == 0)
	{
		while (iter.current != NULL)
		{
			queue_signal(iter.current->val, sig);
			LINKED_LIST_MOVE_NEXT(iter);
		}
	}
	else
	{
		LINKED_LIST_SEEK_UNTIL(iter, 
			iter.current->val->id == tid);
		if (iter.current == NULL)
			return -1;
		queue_signal(iter.current->val, sig);
	}
	return 0;
}

/*
 * Send a signal to a process
 */
int kill(pid_t pid, int signum)
{
	int r;
	mutex_wait(&schedule_lock);
	r = kill_nolock(pid, signum);
	mutex_release(&schedule_lock);
	return r;
}

/*
 * sends a signal to a task
 */
int kill_nolock(pid_t pid, int sig)
{
	LINKED_LIST_ITER_STRUCT(task_t) iter;
	iter = LINKED_LIST_ITER_INIT(task_t, tasks_list);
		
	if (pid == 0)
	{
		while (iter.current != NULL)
		{
			if (iter.current->val->pid == pid && 
				iter.current->val->main_thread == iter.current->val)
				queue_signal(iter.current->val, sig);
			LINKED_LIST_MOVE_NEXT(iter);
		}
	}
	else
	{
		LINKED_LIST_SEEK_UNTIL(iter, iter.current->val->pid == pid && 
			iter.current->val->main_thread == iter.current->val);
		if (iter.current == NULL)
			return -1;
		queue_signal(iter.current->val, sig);
	}
	return 0;
}

/*
 * raise a signal
 */
int raise(int signal)
{
	return pthread_kill(current_task->id, signal);
}

/*
 * Puts the current task to sleep
 */
void pause(void)
{
	assert(current_task != NULL);
	assert(current_task->id != 0);
	mutex_busywait(&schedule_lock);
	current_task->status = TASK_STATE_WAITING;
	schedule_prelocked();
	return;
}

/*
 * Examine and change signal action.
 */
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	if (unlikely(signum < 1 || signum > 32 || signum == SIGKILL || signum == SIGSTOP))
		return EINVAL;
	if (oldact == NULL || act == NULL)
		return EFAULT;
	*oldact = current_task->sig_action[signum - 1];
	current_task->sig_action[signum - 1] = *act;
	return ESUCCESS;
}

/*
 * Remove signal from the process signal mask and wait
 * for a signal.
 */
int sigpause(int sig)
{
	sigset_t oldmask;
	oldmask = current_task->sigmask;
	current_task->sigmask = ~(1 << (sig - 1));
	pause();
	current_task->sigmask = oldmask;
	return ESUCCESS;
}

/*
 * Wait for a signal.
 */
int sigwait(const sigset_t *set, int *sig)
{
	int signum;
	if (unlikely(set == NULL || sig == NULL))
		return EFAULT;
	
	while (1)
	{
		if (current_task->sig_delivered[0] & *set)
		{
			uint32_t sigbit;
			signum = 0;
			sigbit = 1;
			while (1)
			{
				if (unlikely(sigbit & *set & current_task->sig_delivered[0]))
				{
					if (current_task->sig_delivered[3] & sigbit)
						current_task->sig_delivered[3] &= ~sigbit;
					else if (current_task->sig_delivered[2] & sigbit)
						current_task->sig_delivered[2] &= ~sigbit;
					else if (current_task->sig_delivered[1] & sigbit)
						current_task->sig_delivered[1] &= ~sigbit;
					else if (current_task->sig_delivered[0] & sigbit)
						current_task->sig_delivered[0] &= ~sigbit;
					signum++;
					*sig = signum;
					return signum;
				}
				signum++;
				sigbit <<= 1;
			}			
		}
		pause();	
	}
	return -1;
}

/*
 * Set and get signal alternate stack.
 */ 
int sigaltstack(const stack_t *ss, stack_t *oss)
{
	if (oss != NULL)
		*oss = current_task->sig_altstack;
	if (ss != NULL)
	{
		if (ss->ss_flags & ~SS_DISABLE)
			return EINVAL;
		if (ss->ss_size > MINSIGSTKSZ)
			return ENOMEM;
		current_task->sig_altstack = *ss;
	}
	return ESUCCESS;
}

/*
 * Wait for a signal.
 */
int sigsuspend(const sigset_t *set)
{
	sigset_t oldmask;
	oldmask = current_task->sigmask;
	current_task->sigmask = *set;
	while (1)
	{
		if (current_task->sig_delivered[0] & *set)
		{
			uint32_t sigbit;
			int32_t signum;
			signum = 0;
			sigbit = 1;
			while (1)
			{
				if (unlikely(sigbit & *set & current_task->sig_delivered[0]))
				{
					if (current_task->sig_delivered[3] & sigbit)
						current_task->sig_delivered[3] &= ~sigbit;
					else if (current_task->sig_delivered[2] & sigbit)
						current_task->sig_delivered[2] &= ~sigbit;
					else if (current_task->sig_delivered[1] & sigbit)
						current_task->sig_delivered[1] &= ~sigbit;
					else if (current_task->sig_delivered[0] & sigbit)
						current_task->sig_delivered[0] &= ~sigbit;
					signum++;
					current_task->sigmask = oldmask;
					return signum;
				}
				signum++;
				sigbit <<= 1;
			}
		}
		pause();
	}
	current_task->sigmask = oldmask;
	return ESUCCESS;
}

int sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	if (set == NULL)
		return EFAULT;
	sigset_t oldmask;
	oldmask = current_task->sigmask;
	current_task->sigmask = *set;
	while (1)
	{
		if (current_task->sig_delivered[0] & *set)
		{
			uint32_t sigbit;
			int32_t signum;
			signum = 0;
			sigbit = 1;
			while (1)
			{
				if (unlikely(sigbit & *set & current_task->sig_delivered[0]))
				{
					if (current_task->sig_delivered[3] & sigbit)
						current_task->sig_delivered[3] &= ~sigbit;
					else if (current_task->sig_delivered[2] & sigbit)
						current_task->sig_delivered[2] &= ~sigbit;
					else if (current_task->sig_delivered[1] & sigbit)
						current_task->sig_delivered[1] &= ~sigbit;
					else if (current_task->sig_delivered[0] & sigbit)
						current_task->sig_delivered[0] &= ~sigbit;
					signum++;
					current_task->sigmask = oldmask;
					return signum;
				}
				signum++;
				sigbit <<= 1;
			}
		}
		pause();
	}
	current_task->sigmask = oldmask;
	return ESUCCESS;
}
