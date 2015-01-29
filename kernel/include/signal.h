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

/*
 * signals enumerator
 */
typedef enum
{
	SIGHUP = 1,
	SIGINT = 2,
	SIGQUIT = 3,
	SIGILL = 4,
	SIGABRT	= 6,
	SIGFPE = 8,
	SIGKILL = 9,
	SIGSEGV = 11,
	SIGTHREAD = 12,
	SIGPIPE = 13,
	SIGALRM = 14,
	SIGTERM = 15,
	SIGUSR1 = 30,
	SIGUSR2 = 31,
	SIGCHLD = 20,
	SIGCONT = 19,
	SIGSTOP = 17,
	SIGTSTP = 18,
	SIGTTIN = 21,
	SIGTTOU = 22
}
signal_t;

/*
 * signal handler function typedef
 */
typedef void (*signal_handler_t)(int signum);

#endif

