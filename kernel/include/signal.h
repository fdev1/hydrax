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

/*
 * signal handler function typedef
 */
typedef void (*signal_handler_t)(int signum);

#define SIG_DFL		((signal_handler_t)  0)
#define SIG_ERR		((signal_handler_t) -1)
#define SIG_HOLD		((signal_handler_t)  1)
#define SIG_IGN		((signal_handler_t)  2)

#endif

