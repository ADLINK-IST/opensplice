/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OS_WIN32_SIGNAL_H
#define OS_WIN32_SIGNAL_H

#include <signal.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef int os_os_signal;

typedef int os_os_sigset;

typedef int os_os_sigaction;

#define OS_SIGTERM      SIGTERM
#define OS_SIGINT       SIGINT
#define OS_SIGHUP       0
#define OS_SIGCHLD	    0
#define OS_SIGKILL      1000
#define OS_SIGPIPE      0
#define OS_SIGALRM      0
#define OS_SIGUSR1      0
#define OS_SIGUSR2      0
#define OS_SIGPOLL      0
#define OS_SIGVTALRM    0
#define OS_SIGPROF      0

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_SIGNAL_H */
