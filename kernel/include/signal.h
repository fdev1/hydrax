/*
 * include/signal.h
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

#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <arch/arch.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

/*
 * signals enumerator
 */
typedef enum
{
	SIGHUP = 1,
	SIGINT = 2,
	SIGQUIT = 3,
	SIGILL = 4,
	SIGBUS = 5,
	SIGABRT = 6,
	SIGSYS = 7,
	SIGFPE = 8,
	SIGKILL = 9,
	SIGTRAP = 10,
	SIGSEGV = 11,
	SIGTHREAD = 12,
	SIGPIPE = 13,
	SIGALRM = 14,
	SIGTERM = 15,
	SIGXCPU = 16,
	SIGSTOP = 17,
	SIGTSTP = 18,
	SIGCONT = 19,
	SIGCHLD = 20,
	SIGTTIN = 21,
	SIGTTOU = 22,
	SIGXFSZ = 23,
	SIGPOLL = 24,
	SIGPROF = 25,
	SIGVTALRM = 26,
	SIGURG = 27,
	SIGUSR1 = 30,
	SIGUSR2 = 31
}
signal_t;

typedef uint32_t sigset_t;

#define SIGEV_NONE		(0)
#define SIGEV_SIGNAL	(1)
#define SIGEV_THREAD	(2)

struct __ptread_attr
{
	int x;
};

union sigval
{
	int sival_int;
	void *sival_ptr;
};

struct sigevent
{
	int sigev_notify;
	int sigev_signo;
	union sigval sigev_value;
	void (*sigev_notify_function)(union sigval);
	struct __ptread_attr *sigev_notify_attributes;
	
};

typedef struct __siginfo
{
	int si_signo;
	int si_code;
	int si_errno;
	pid_t si_pid;
	uid_t si_uid;
	void *si_addr;
	int si_status;
	long si_band;
	union sigval si_value;
}
siginfo_t;

struct sigaction
{
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_handler)(int);
	void (*sa_sigaction)(int, siginfo_t*, void*);
};

/*
 * signal handler function typedef
 */
typedef void (*signal_handler_t)(int signum);
typedef void (*sighandler_t)(int);

#define SIG_DFL		((signal_handler_t)  0)
#define SIG_ERR		((signal_handler_t) -1)
#define SIG_HOLD		((signal_handler_t)  1)
#define SIG_IGN		((signal_handler_t)  2)


#define SA_NOCLDSTOP	(1)		/* Do not generate SIGCHLD when children stop
							   or stopped children continue. */
#define SIG_BLOCK		(2)		/* The resulting set is the union of the current set and 
							   the signal set pointed to by the argument set.*/
#define SIG_UNBLOCK		(3)		/* The resulting set is the intersection of the current set 
							   and the complement of the signal set pointed to by the argument 
							   set. */
#define SIG_SETMASK		(4)		/* The resulting set is the signal set pointed to by the argument set. */
#define SA_ONSTACK		(5)		/* Causes signal delivery to occur on an alternate stack. */
#define SA_RESETHAND	(6)		/* Causes signal dispositions to be set to SIG_DFL on entry 
							   to signal handlers. */
#define SA_RESTART		(7)		/* Causes certain functions to become restartable. */
#define SA_SIGINFO		(8)		/* Causes extra information to be passed to signal handlers 
							   at the time of receipt of a signal. */
#define SA_NOCLDWAIT	(9)		/* Causes implementations not to create zombie processes on 
							   child death. */
#define SA_NODEFER		(10)		/* Causes signal not to be automatically blocked on entry to 
							   signal handler. */
#define SS_ONSTACK		(11)		/* Process is executing on an alternate signal stack. */
#define SS_DISABLE		(12)		/* Alternate signal stack is disabled. */
#define MINSIGSTKSZ		(1024)	/* Minimum stack size for a signal handler. */
#define SIGSTKSZ		(1024)	/* Default size in bytes for the alternate signal stack. */

typedef struct __stack
{
	void *ss_sp;
	size_t ss_size;
	int ss_flags;
}
stack_t;

/*
 * SIGILL Error codes
 */
#define ILL_ILLOPC		(1)		/* Illegal opcode. */
#define ILL_ILLOPN		(2)		/* Illegal operand. */
#define ILL_ILLADR		(3)		/* Illegal addressing mode. */
#define ILL_ILLTRP		(4)		/* Illegal trap. */
#define ILL_PRVOPC		(5)		/* Privileged opcode. */
#define ILL_PRVREG		(6)		/* Privileged register. */
#define ILL_COPROC		(7)		/* Coprocessor error. */
#define ILL_BADSTK		(8)		/* Internal stack error. */


/* 
 * SIGFPE error codes
 */
#define FPE_INTDIV		(1)		/* Integer divide by zero. */
#define FPE_INTOVF		(2)		/* Integer overflow. */
#define FPE_FLTDIV		(3)		/* Floating-point divide by zero. */
#define FPE_FLTOVF		(4)		/* Floating-point overflow. */
#define FPE_FLTUND		(5)		/* Floating-point underflow. */
#define FPE_FLTRES		(6)		/* Floating-point inexact result. */
#define FPE_FLTINV		(7)		/* Invalid floating-point operation. */
#define FPE_FLTSUB		(8)		/* Subscript out of range. */

/*
 * SIGSEGV error codes
 */
#define SEGV_MAPERR		(1)		/* Address not mapped to object. */
#define SEGV_ACCERR		(2)		/* Invalid permissions for mapped object. */

/*
 * SIGBUS error codes
 */
#define BUS_ADRALN		(1)		/* Invalid address alignment. */
#define BUS_ADRERR		(2)		/* Nonexistent physical address. */
#define BUS_OBJERR		(3)		/* Object-specific hardware error. */

/*
 * SIGTRAP error codes 
 */
#define TRAP_BRKPT		(1)		/* Process breakpoint. */
#define TRAP_TRACE		(2)		/* Process trace trap. */

/*
 * SIGCHLD error codes
 */
#define CLD_EXITED		(1)		/* Child has exited. */
#define CLD_KILLED		(2)		/* Child has terminated abnormally and did not create a core file. */
#define CLD_DUMPED		(3)		/* Child has terminated abnormally and created a core file. */
#define CLD_TRAPPED		(4)		/* Traced child has trapped. */
#define CLD_STOPPED		(5)		/* Child has stopped. */
#define CLD_CONTINUED	(6)		/* Stopped child has continued. */

/*
 * SIGPOLL error codes
 */
#define POLL_IN		(1)		/* Data input available.*/
#define POLL_OUT		(2)		/* Output buffers available. */
#define POLL_MSG		(3)		/* Input message available. */
#define POLL_ERR		(4)		/* I/O error. */
#define POLL_PRI		(5)		/* High priority input available. */
#define POLL_HUP		(6)		/* Device disconnected. */

/*
 * Error codes for all signals.
 */
#define SI_USER		(1)		/* Signal sent by kill(). */
#define SI_QUEUE		(2)		/* Signal sent by the sigqueue(). */
#define SI_TIMER		(3)		/* Signal generated by expiration of a timer set by timer_settime(). */
#define SI_ASYNCIO		(4)		/* Signal generated by completion of an asynchronous I/O request. */
#define SI_MESGQ		(5)		/* Signal generated by arrival of a message on an empty message queue. */

#include <ucontext.h>

void (*bsd_signal(int, void (*)(int)))(int);

/*
 * Send a signal to a process
 */
int kill(pid_t, int);

int killpg(pid_t, int);

#if 0
int pthread_kill(pthread_t, int);
int pthread_sigmask(int, const sigset_t *, sigset_t *); 
#endif

/*
 * raise a signal
 */
int raise(int);

/*
 * Examine and change signal action.
 */
int sigaction(int, const struct sigaction *, struct sigaction *);

/*
 * Add signal to signal set.
 */
static inline int sigaddset(sigset_t *set, int sig)
{
	if (unlikely(sig < 1 || sig > 32))
		return EINVAL;
	*set |= 1 << (sig - 1);
	return ESUCCESS;
}

/*
 * Delete signal from signal set.
 */
static inline int sigdelset(sigset_t *set, int sig)
{
	if (unlikely(sig < 1 || sig > 32))
		return EINVAL;
	if (unlikely(set == NULL))
		return EFAULT;
	*set &= ~(1 << (sig - 1));
	return ESUCCESS;
}

/*
 * Initialize set as an empty set;
 */
static inline int sigemptyset(sigset_t *set)
{
	*set = 0;
	return ESUCCESS;
}

/*
 * Initialize a full set.
 */
static inline int sigfillset(sigset_t *set)
{
	if (set == NULL)
	{
		//set_err_no(EFAULT);
		return -1;
	}
	*set = 0xFFFFFFFF;
	return ESUCCESS;
}

int sigaltstack(const stack_t *, stack_t *);

/*
 * set a signal handler
 */
int signal(int sig, sighandler_t handler);

/*
 * Add a signal to the calling process' signal
 * mask.
 */
int sighold(int);

/*
 * Sets the disposition of a signal to SIG_IGN
 */
int sigignore(int);

int siginterrupt(int, int);
int sigismember(const sigset_t *, int);

/*
 * Remove signal from the process signal mask and wait
 * for a signal.
 */
int sigpause(int);

/*
 * Returns the set of signals that are pending for delivery
 * to the current thread.
 */
int sigpending(sigset_t *);

/*
 * Examine and change blocked signals.
 */
int sigprocmask(int, const sigset_t *, sigset_t *);

/*
 * Queue a signal and data to a process.
 */
int sigqueue(pid_t, int, const union sigval);

/*
 * Remove a signal from the calling process' signal
 * mask.
 */
int sigrelse(int);

/*
 * Modifies the disposition of a signal
 */
sighandler_t sigset(int, sighandler_t);

int sigsuspend(const sigset_t *);
int sigtimedwait(const sigset_t *, siginfo_t *, const struct timespec *);

/*
 * Wait for a signal.
 */
int sigwait(const sigset_t *, int *);

int sigwaitinfo(const sigset_t *, siginfo_t *);

#define clear_task_signals(task)	\
do {							\
	(task)->sig_pending[0] = 0;	\
	(task)->sig_pending[1] = 0;	\
	(task)->sig_pending[2] = 0;	\
	(task)->sig_pending[3] = 0;	\
	(task)->sig_delivered[0] = 0;	\
	(task)->sig_delivered[1] = 0;	\
	(task)->sig_delivered[2] = 0;	\
	(task)->sig_delivered[3] = 0;	\
	memset((task)->sig_info, 0, sizeof(siginfo_t) * 4);	\
}							\
while (0)

#endif

