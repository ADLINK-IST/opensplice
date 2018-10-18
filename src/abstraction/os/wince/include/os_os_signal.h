/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OS_WIN32_SIGNAL_H
#define OS_WIN32_SIGNAL_H

#if defined (__cplusplus)
extern "C" {
#endif

typedef int os_os_signal;

typedef int os_os_sigset;

typedef int os_os_sigaction;

typedef void (*os_os_actionHandler)();

/* Signal types */
#define SIGINT          2       /* interrupt */
#define SIGILL          4       /* illegal instruction - invalid function image */
#define SIGFPE          8       /* floating point exception */
#define SIGSEGV         11      /* segment violation */
#define SIGTERM         15      /* Software termination signal from kill */
#define SIGBREAK        21      /* Ctrl-Break sequence */
#define SIGABRT         22      /* abnormal termination triggered by abort call */

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
