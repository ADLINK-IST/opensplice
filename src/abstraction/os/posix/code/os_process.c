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
/** \file os/posix/code/os_process.c
 *  \brief Posix process management
 *
 * Implements process management for POSIX
 */

#include "os_process.h"
#include "../posix/code/os__process.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_init.h"
#include "os_time.h"

#include <sys/types.h>
#ifndef OSPL_NO_VMEM
#include <sys/mman.h>
#endif
#ifndef PIKEOS_POSIX
#include <sys/wait.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <errno.h>
#ifndef INTEGRITY
#include <signal.h>
#endif
#include <stdio.h>
#ifndef VXWORKS_RTP
#include <pthread.h>
#endif

static char* processName = NULL;
/** \brief pointer to environment variables */
#ifdef __APPLE__
#include <crt_externs.h>
#else
extern char **environ;
#endif
static os_procTerminationHandler _ospl_termHandler   = (os_procTerminationHandler)0;

#ifndef INTEGRITY
#ifndef VXWORKS_RTP
#define _SIGNALVECTOR_(sig) _ospl_oldSignalVector##sig
static struct sigaction _SIGNALVECTOR_(SIGINT);
static struct sigaction _SIGNALVECTOR_(SIGQUIT);
static struct sigaction _SIGNALVECTOR_(SIGHUP);
static struct sigaction _SIGNALVECTOR_(SIGTERM);

#ifdef __APPLE__
#define CATCH_EXCEPTIONS 0
#else
#define CATCH_EXCEPTIONS 1
#endif

#if CATCH_EXCEPTIONS
static struct sigaction _SIGNALVECTOR_(SIGILL);
static struct sigaction _SIGNALVECTOR_(SIGABRT);
static struct sigaction _SIGNALVECTOR_(SIGFPE);
static struct sigaction _SIGNALVECTOR_(SIGSEGV);
#endif

static struct sigaction _SIGNALVECTOR_(SIGPIPE);
static struct sigaction _SIGNALVECTOR_(SIGALRM);
static struct sigaction _SIGNALVECTOR_(SIGUSR1);
static struct sigaction _SIGNALVECTOR_(SIGUSR2);
/*static struct sigaction _SIGNALVECTOR_(SIGTSTOP); -- ignore for now */
static struct sigaction _SIGNALVECTOR_(SIGTTIN);
/*static struct sigaction _SIGNALVECTOR_(SIGTTOUT); -- ignore for now */


#define OSPL_SIGNALHANDLERTHREAD_TERMINATE 1 /* Instruct thread to terminate */
#define OSPL_SIGNALHANDLERTHREAD_EXIT      2 /* Instruct thread to call exit() */

static pthread_t _ospl_signalHandlerThreadId;
static int _ospl_signalHandlerThreadTerminate = 0;
#endif
static int installSignalHandler = 1;
static int _ospl_signalpipe[2] = { 0, 0};

/* private functions */
#if !defined VXWORKS_RTP && !defined PIKEOS_POSIX
static int
isSignallingSafe(
    int reportReason)
{

    if(!installSignalHandler && reportReason){
        OS_REPORT(OS_WARNING, "OS abstraction layer", 0,
                  "Did not install signal handlers to cleanup resources.\n"\
                  "              To ensure cleanup for Java applications, the path to the 'jsig' library\n"\
                  "              (libjsig.so) must be set in the LD_PRELOAD environment variable.\n"\
                  "              This library is part of your Java distribution.\n"\
                  "              To ensure proper cleanup set this before starting your application.");
    }
    return installSignalHandler;
}

static void
signalHandler(
    int sig,
    siginfo_t *info,
    void *arg)
{
    os_int32 terminate;
    os_terminationType reason;
    int result;
    (void)info;
    (void)arg;

    OS_UNUSED_ARG(info);
    OS_UNUSED_ARG(arg);

    if (_ospl_termHandler) {
        switch (sig) {
            case SIGINT:
            case SIGQUIT:
            case SIGHUP:
            case SIGTERM:
                reason = OS_TERMINATION_NORMAL;
            break;
            default:
                reason = OS_TERMINATION_ERROR;
            break;
        }
        terminate = _ospl_termHandler(reason);
    } else {
        terminate = 1;
    }

    if (terminate) {
        /* The signalHandlerThread will always perform the
         * exit() call.
         */
        _ospl_signalHandlerThreadTerminate = OSPL_SIGNALHANDLERTHREAD_EXIT;
        result = write(_ospl_signalpipe[1], &sig, sizeof(int));
    }
}

/* This thread is introduced to ensure there is always 1 thread
 * that is capable of handling signals
 */
static void *
signalHandlerThread(
    void *arg)
{
    sigset_t sigset;
    int sig;
    int result;
    (void)arg;

    OS_UNUSED_ARG(arg);

    sigemptyset(&sigset);
    pthread_sigmask(SIG_SETMASK, &sigset, NULL);

    result = -1;
    while (result == -1) {
        result = read(_ospl_signalpipe[0], &sig, sizeof(int));
    }
    /* first call previous signal handler, if not default */
    switch (sig) {
    case -1: /* used for terminating this thread! */
        assert(_ospl_signalHandlerThreadTerminate != OSPL_SIGNALHANDLERTHREAD_EXIT);
    break;
    case SIGINT:
        if ((_SIGNALVECTOR_(SIGINT).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGINT).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGINT).sa_handler(SIGINT);
        }
    break;
    case SIGQUIT:
        if ((_SIGNALVECTOR_(SIGQUIT).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGQUIT).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGQUIT).sa_handler(SIGQUIT);
        }
    break;
    case SIGHUP:
        if ((_SIGNALVECTOR_(SIGHUP).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGHUP).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGHUP).sa_handler(SIGHUP);
        }
    break;
    case SIGTERM:
        if ((_SIGNALVECTOR_(SIGTERM).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGTERM).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGTERM).sa_handler(SIGTERM);
        }
    break;
#if CATCH_EXCEPTIONS
    case SIGILL:
        if ((_SIGNALVECTOR_(SIGILL).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGILL).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGILL).sa_handler(SIGILL);
        }
    break;
    case SIGABRT:
        if ((_SIGNALVECTOR_(SIGABRT).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGABRT).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGABRT).sa_handler(SIGABRT);
        }
    break;
    case SIGFPE:
        if ((_SIGNALVECTOR_(SIGFPE).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGFPE).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGFPE).sa_handler(SIGFPE);
        }
    break;
    case SIGSEGV:
        if ((_SIGNALVECTOR_(SIGSEGV).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGSEGV).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGSEGV).sa_handler(SIGSEGV);
        }
    break;
#endif
    case SIGPIPE:
        if ((_SIGNALVECTOR_(SIGPIPE).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGPIPE).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGPIPE).sa_handler(SIGPIPE);
        }
    break;
    case SIGALRM:
        if ((_SIGNALVECTOR_(SIGALRM).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGALRM).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGALRM).sa_handler(SIGALRM);
        }
    break;
    case SIGUSR1:
        if ((_SIGNALVECTOR_(SIGUSR1).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGUSR1).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGUSR1).sa_handler(SIGUSR1);
        }
    break;
    case SIGUSR2:
        if ((_SIGNALVECTOR_(SIGUSR2).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGUSR2).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGUSR2).sa_handler(SIGUSR2);
        }
    break;
/*  Only in newer POSIX versions, ignoring for now
    case SIGTSTOP:
        if ((_SIGNALVECTOR_(SIGTSTOP).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGTSTOP).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGTSTOP).sa_handler(SIGTSTOP);
        }
    break;
*/    case SIGTTIN:
        if ((_SIGNALVECTOR_(SIGTTIN).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGTTIN).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGTTIN).sa_handler(SIGTTIN);
        }
    break;
/*  Only in newer POSIX versions, ignoring for now
    case SIGTTOUT:
        if ((_SIGNALVECTOR_(SIGTTOUT).sa_handler != SIG_DFL) &&
            (_SIGNALVECTOR_(SIGTTOUT).sa_handler != SIG_IGN)) {
            _SIGNALVECTOR_(SIGTTOUT).sa_handler(SIGTTOUT);
        }
    break;
*/    default:
        assert(0);
    }
    if (_ospl_signalHandlerThreadTerminate == OSPL_SIGNALHANDLERTHREAD_EXIT) {
        if(!os_serviceGetSingleProcess())
        {
            exit(0);
        } else
        {
            _exit(0);
        }
    }
    return NULL;
}

#define _SIGACTION_(sig) sigaction(sig,&action,&_ospl_oldSignalVector##sig)
#define _SIGCURRENTACTION_(sig) sigaction(sig, NULL, &_ospl_oldSignalVector##sig)
#endif
#endif

/* protected functions */
void
os_processModuleInit(void)
{
#if !defined INTEGRITY && !defined VXWORKS_RTP && !defined PIKEOS_POSIX
    struct sigaction action;
    pthread_attr_t      thrAttr;
    int result;

    result = pipe(_ospl_signalpipe);
    pthread_attr_init(&thrAttr);
    pthread_attr_setstacksize(&thrAttr, 4*1024*1024); /* 4MB */
    pthread_create(&_ospl_signalHandlerThreadId, &thrAttr, signalHandlerThread, (void*)0);


    /* install signal handlers */
    action.sa_handler = 0;
    action.sa_sigaction = signalHandler;
    sigfillset(&action.sa_mask); /* block all signals during handling of a signal */
    action.sa_flags = SA_SIGINFO;


    _SIGCURRENTACTION_(SIGINT);

    /* If the user has set a signal handler or explicitly told the system to
     * ignore the signal, we don't set a handler ourselves. It's the
     * responsibility of the user to make sure exit() is called to
     * terminate the application to make sure all shared memory resources
     * are properly cleaned up. This is on a per signal basis.
     */
    if ((_SIGNALVECTOR_(SIGINT).sa_handler == SIG_DFL) ||
        (_SIGNALVECTOR_(SIGINT).sa_handler == SIG_IGN)) {
        _SIGACTION_(SIGINT);
    }
    _SIGCURRENTACTION_(SIGQUIT);

    if ((_SIGNALVECTOR_(SIGQUIT).sa_handler == SIG_DFL) ||
        (_SIGNALVECTOR_(SIGQUIT).sa_handler == SIG_IGN)) {
        _SIGACTION_(SIGQUIT);
    }
    _SIGCURRENTACTION_(SIGHUP);

    if ((_SIGNALVECTOR_(SIGHUP).sa_handler == SIG_DFL) ||
        (_SIGNALVECTOR_(SIGHUP).sa_handler == SIG_IGN)) {
        _SIGACTION_(SIGHUP);
    }
    _SIGCURRENTACTION_(SIGTERM);

    if ((_SIGNALVECTOR_(SIGTERM).sa_handler == SIG_DFL) ||
        (_SIGNALVECTOR_(SIGTERM).sa_handler == SIG_IGN)) {
        _SIGACTION_(SIGTERM);
    }

    if(isSignallingSafe(1)){
#if CATCH_EXCEPTIONS
        _SIGCURRENTACTION_(SIGILL);

        if ((_SIGNALVECTOR_(SIGILL).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGILL).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGILL);
        }
        _SIGCURRENTACTION_(SIGABRT);

        if ((_SIGNALVECTOR_(SIGABRT).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGABRT).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGABRT);
        }
        _SIGCURRENTACTION_(SIGFPE);

        if ((_SIGNALVECTOR_(SIGFPE).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGFPE).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGFPE);
        }
        _SIGCURRENTACTION_(SIGSEGV);

        if ((_SIGNALVECTOR_(SIGSEGV).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGSEGV).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGSEGV);
        }
#endif
        _SIGCURRENTACTION_(SIGPIPE);

        if ((_SIGNALVECTOR_(SIGPIPE).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGPIPE).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGPIPE);
        }
        _SIGCURRENTACTION_(SIGALRM);

        if ((_SIGNALVECTOR_(SIGALRM).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGALRM).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGALRM);
        }
        _SIGCURRENTACTION_(SIGUSR1);

        if ((_SIGNALVECTOR_(SIGUSR1).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGUSR1).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGUSR1);
        }
        _SIGCURRENTACTION_(SIGUSR2);

        if ((_SIGNALVECTOR_(SIGUSR2).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGUSR2).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGUSR2);
        }
        /* Only in newer POSIX versions, ignoring for now
        _SIGCURRENTACTION_(SIGTSTOP);
        _SIGACTION_(SIGTSTOP);
        */
        _SIGCURRENTACTION_(SIGTTIN);

        if ((_SIGNALVECTOR_(SIGTTIN).sa_handler == SIG_DFL) ||
            (_SIGNALVECTOR_(SIGTTIN).sa_handler == SIG_IGN)) {
            _SIGACTION_(SIGTTIN);
        }
        /* Only in newer POSIX versions, ignoring for now
        _SIGCURRENTACTION_(SIGTTOUT);
        _SIGACTION_(SIGTTOUT);
        */
    }
#endif
}

#undef _SIGACTION_
#undef _SIGCURRENTACTION_
#undef _SIGDEFAULT_

#define _SIGACTION_(sig) sigaction(sig,&_ospl_oldSignalVector##sig, NULL)
void
os_processModuleExit(void)
{
#if !defined INTEGRITY && !defined VXWORKS_RTP && !defined PIKEOS_POSIX
    int sig = -1;
    void *thread_result;

    /* deinstall signal handlers */
    _SIGACTION_(SIGINT);
    _SIGACTION_(SIGQUIT);
    _SIGACTION_(SIGHUP);
    _SIGACTION_(SIGTERM);


    if(isSignallingSafe(0)){
#if CATCH_EXCEPTIONS
        _SIGACTION_(SIGILL);
        _SIGACTION_(SIGABRT);
        _SIGACTION_(SIGFPE);
        _SIGACTION_(SIGSEGV);
#endif
        _SIGACTION_(SIGPIPE);
        _SIGACTION_(SIGALRM);
        _SIGACTION_(SIGUSR1);
        _SIGACTION_(SIGUSR2);
        _SIGACTION_(SIGTTIN);
    }
    _ospl_signalHandlerThreadTerminate = OSPL_SIGNALHANDLERTHREAD_TERMINATE;
    if (pthread_self() != _ospl_signalHandlerThreadId) {
        write(_ospl_signalpipe[1], &sig, sizeof(sig));
        pthread_join(_ospl_signalHandlerThreadId, &thread_result);
    }
#endif
    if(processName) {
        os_free(processName);
    }
}
#undef _SIGACTION_

/* public functions */



os_procTerminationHandler
os_procSetTerminationHandler(
    os_procTerminationHandler handler)
{
    os_procTerminationHandler oldHandler;

    oldHandler = _ospl_termHandler;
    _ospl_termHandler = handler;
    return oldHandler;
}

/** \brief Register an process exit handler
 *
 * \b os_procAtExit registers an process exit
 * handler by calling \b atexit passing the \b function
 * to be called when the process exits.
 * The standard POSIX implementation guarantees the
 * required order of execution of the exit handlers.
 */

#ifndef PIKEOS_POSIX
os_result
os_procAtExit(
    void (*function)(void))
{
    int result;
    os_result osResult;

    assert (function != NULL);

    result = atexit (function);
    if(!result)
    {
        osResult = os_resultSuccess;
    } else
    {
        osResult = os_resultFail;
    }
    return osResult;
}
#endif

void
os_procSetSignalHandlingEnabled(
    os_uint enabled)
{
#ifndef INTEGRITY
    if(enabled == 0){
        installSignalHandler = 0;
    } else {
        installSignalHandler = 1;
    }
#endif
}

/** \brief Terminate the process and return the status
 *         the the parent process
 *
 * \b os_procExit terminates the process by calling \b exit.
 */
#ifndef PIKEOS_POSIX
void
os_procExit(
    os_exitStatus status)
{
    assert(status != OS_EXIT_SUCCESS || status != OS_EXIT_FAILURE);
    exit((signed int)status);
    return;
}
#endif

#if !defined VXWORKS_RTP
/** \brief Create a process that is an instantiation of a program
 *
 * First an argument list is built from \b arguments.
 * Then \b os_procCreate creates a process by forking the current
 * process.
 *
 * The child process processes the lock policy attribute from
 * \b procAttr and sets the lock policy accordingly by calling
 * \b mlockall if required. If the process has root privileges
 * it processes the user credentials from \b procAttr and sets
 * the user credentials of the child process accordingly.
 * The child process then replaces the running program with the
 * program provided by the \b executable_file by calling \b execve.
 *
 * The parent process processes the scheduling class and
 * scheduling priority attributes from \b procAttr and
 * sets the scheduling properties of the child process
 * accordingly by calling \b sched_setscheduler.
 */
os_result
os_procCreate(
    const char *executable_file,
    const char *name,
    const char *arguments,
    os_procAttr * procAttr,
    os_procId *procId)
{
    os_result rv = os_resultSuccess;
#if !defined INTEGRITY && !defined PIKEOS_POSIX
    pid_t pid;
    char *argv[64];
    int argc = 1;
    int go_on = 1;
    int i = 0;
    int sq_open = 0;
    int sq_close = 0;
    int dq_open = 0;
    int dq_close = 0;
    char *argin;
    struct sched_param sched_param, sched_current;
    int sched_policy;
    char environment[512];

    assert(executable_file != NULL);
    assert(name != NULL);
    assert(arguments != NULL);
    assert(procAttr != NULL);
    assert(procId != NULL);
    if (procAttr->schedClass == OS_SCHED_REALTIME) {
        sched_policy = SCHED_FIFO;
    } else if (procAttr->schedClass == OS_SCHED_TIMESHARE) {
        sched_policy = SCHED_OTHER;
    } else if (procAttr->schedClass == OS_SCHED_DEFAULT) {
        sched_policy = SCHED_OTHER;
    } else {
        return os_resultInvalid;
    }
    if (procAttr->schedPriority < sched_get_priority_min (sched_policy) ||
        procAttr->schedPriority > sched_get_priority_max (sched_policy)) {

        procAttr->schedPriority = (sched_get_priority_max (sched_policy) +
                                   sched_get_priority_min(sched_policy)) / 2;
        OS_REPORT_1(OS_WARNING, "os_procCreate", 2,
            "scheduling priority outside valid range for the policy reverted to valid value (%s)",
            name);
    }
    if (access(executable_file, X_OK) != 0) {
        rv = os_resultInvalid;
    } else {
        /* first translate the input string into an argv structured list */
        argin = os_malloc(strlen(arguments) + 1);
        os_strcpy(argin, arguments);
        argv[0] = os_malloc(strlen(name) + 1);
        argv[0] = os_strcpy(argv[0], name);
        while (go_on && (unsigned int)argc <= (sizeof(argv)/(sizeof(char *)))) {
            while (argin[i] == ' ' || argin[i] == '\t' ) {
                i++;
            }
            if (argin[i] == '\0' ) {
                break;
            } else if (argin[i] == '\'') {
                if (sq_open == sq_close) {
                    sq_open++;
                    argv[argc] = &argin[i];
                } else {
                    sq_close++;
                }
                i++;
            } else if (argin[i] == '\"') {
                if (dq_open == dq_close) {
                    dq_open++;
                } else {
                    dq_close++;
                }
                i++;
            } else {
                argv[argc] = &argin[i];
                argc++;
                while ((argin[i] != ' ' && argin[i] != '\t') ||
                       (sq_open != sq_close) ||
                       (dq_open != dq_close)) {
                    if (argin[i] == '\0') {
                        go_on = 0;
                        break;
                    } else if (argin[i] == '\'') {
                        sq_close++;
                        if ((sq_open == sq_close) && (dq_open == dq_close)) {
                            argin[i] = '\0';
                        }
                        i++;
                    } else if (argin[i] == '\"') {
                        dq_close++;
                        if ((dq_open == dq_close) && (sq_open == sq_close)) {
                            argin[i] = '\0';
                        }
                        i++;
                    } else {
                        i++;
                    }
                }
                argin[i] = '\0';
                i++;
            }
        }
        argv [argc] = NULL;
        if ((pid = fork()) == -1) {
            OS_REPORT_3(OS_WARNING, "os_procCreate", 1,
                        "fork failed with error %d (%s, %s)", errno, executable_file, name);
            rv = os_resultFail;
        } else if (pid == 0) {
            /* child process */
            if (procAttr->schedClass == OS_SCHED_REALTIME) {
                sched_param.sched_priority = procAttr->schedPriority;

                if (sched_setscheduler(pid, SCHED_FIFO, &sched_param) == -1) {
                    OS_REPORT_3(OS_WARNING, "os_procCreate", 1,
                            "sched_setscheduler failed with error %d (%s) for process '%s'",
                            errno, strerror(errno), name);
                }
            } else {
                    sched_getparam (0, &sched_current);
                    if (sched_current.sched_priority != procAttr->schedPriority) {
                        sched_param.sched_priority = procAttr->schedPriority;
                        if (sched_setscheduler(pid, SCHED_OTHER, &sched_param) == -1) {
                            OS_REPORT_4(OS_WARNING, "os_procCreate", 1,
                                        "sched_setscheduler failed with error %d (%s). Requested priority was %d, current is %d",
                                        errno, name, procAttr->schedPriority,
                                        sched_current.sched_priority);
                    }
                }
            }
            if (getuid() == 0) {
                /* first set the gid */
                if (procAttr->userCred.gid) {
                    setgid(procAttr->userCred.gid);
                }
                /* then set the uid */
                if (procAttr->userCred.uid) {
                    setuid(procAttr->userCred.uid);
                }
            }
            /* Set the process name via environment variable SPLICE_PROCNAME */
            snprintf(environment, sizeof(environment), "SPLICE_PROCNAME=%s", name);
            putenv(environment);
            /* exec executable file */
#if __APPLE__
            char **environ = *_NSGetEnviron ();
#endif
            if (execve(executable_file, argv, environ) == -1) {
                OS_REPORT_2(OS_WARNING, "os_procCreate", 1, "execve failed with error %d (%s)", errno, executable_file);
            }
            /* if executing this, something has gone wrong */
            rv = os_resultFail; /* Just to fool QAC */
            os_procExit(OS_EXIT_FAILURE);
        } else {
            /* parent process */
            os_free(argv[0]);
            os_free(argin);
            *procId = pid;
            rv = os_resultSuccess;
        }
    }
#endif

    return rv;
}
#endif

/** \brief Check the child exit status of the identified process
 *
 * \b os_procCheckStatus calls \b waitpid with flag \b WNOHANG
 * to check the status of the child process.
 * - On return of \b waitpid with result \b procId, the process
 *   has terminated and its result status is in \b status.
 * - On return of \b waitpid with result 0, the child process is
 *   not yet terminated and \b os_resultBusy is returned.
 * - On return of \b waitpid with result -1 and \b errno is \b ECHILD
 *   the identified child process is not found and \b
 *   os_resultUnavailable is returned.
 * - On any other return from \b waitpid, \b os_resultFail is returned.
 */
#if !defined INTEGRITY && !defined PIKEOS_POSIX
os_result
os_procCheckStatus(
    os_procId procId,
    os_int32 *status)
{
    os_procId result;
    os_result rv;
    int les;

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }

    result = waitpid(procId, &les, WNOHANG);
    if (result == procId) {
        if (WIFEXITED(les)) {
            *status = WEXITSTATUS(les);
        } else {
            *status = OS_EXIT_FAILURE;
        }
        rv = os_resultSuccess;
    } else if (result == 0) {
        rv = os_resultBusy;
    } else if ((result == -1) && (errno == ECHILD)) {
        rv = os_resultUnavailable;
    } else {
        rv = os_resultFail;
    }
    return rv;
}
#endif

/** \brief Return the integer representation of the given process ID
 *
 * Possible Results:
 * - returns the integer representation of the given process ID
 */
os_int
os_procIdToInteger(os_procId id)
{
   return (os_int)id;
}

/** \brief Return the process ID of the calling process
 *
 * Possible Results:
 * - returns the process ID of the calling process
 */
os_procId
os_procIdSelf(void)
{
    return (os_procId)getpid();
}

/* _OS_PROCESS_DEFAULT_CMDLINE_LEN_ is defined as
 * strlen("/proc/<max_pid>/cmdline" + 1, max_pid = 32768 on Linux, so a reason-
 * able default is 20 */
#define _OS_PROCESS_DEFAULT_CMDLINE_LEN_ (20)
#define _OS_PROCESS_PROCFS_PATH_FMT_     "/proc/%d/cmdline"
#define _OS_PROCESS_DEFAULT_NAME_LEN_ (512)

/** \brief Figure out the identity of the current process
 *
 * os_procFigureIdentity determines the numeric, and if possible named
 * identity of a process. It will first check if the environment variable
 * SPLICE_PROCNAME is set (which is always the case if the process is started
 * via os_procCreate). If so, that value will be returned. Otherwise it will be
 * attempted to determine the commandline which started the process through the
 * procfs. If that fails, the PID will be returned.
 *
 * \param procIdentity  Pointer to a char-buffer to which the result can be
 *                      written. If a name could be resolved, the result will
 *                      have the format "name <PID>". Otherwise it will just
 *                      be "<PID>".
 * \param procIdentitySize Size of the buffer pointed to by procIdentitySize
 * \return same as snprintf returns
 */
os_int32
os_procFigureIdentity(
    char *procIdentity,
    os_uint32 procIdentitySize)
{
    int size = 0;
    char process_name[_OS_PROCESS_DEFAULT_NAME_LEN_];

    size = os_procGetProcessName(process_name,_OS_PROCESS_DEFAULT_NAME_LEN_);
    if (size > 0) {
        size = snprintf(procIdentity, procIdentitySize, "%s <%d>",
                        process_name, os_procIdToInteger(os_procIdSelf()));
    }
    else {
         /* No processname could be determined, so default to PID */
         size = snprintf(procIdentity, procIdentitySize, "<%d>",
                 os_procIdToInteger(os_procIdSelf()));
     }
    return (os_int32)size;
}

#undef _OS_PROCESS_DEFAULT_CMDLINE_LEN_
#undef _OS_PROCESS_PROCFS_PATH_FMT_
#undef _OS_PROCESS_DEFAULT_NAME_LEN_

#if !defined (VXWORKS_RTP) && !defined (PIKEOS_POSIX)

/* os_procServiceDestroy will need an alternative for VXWORKS when ospl
 * extended to include VXWORKS - function not called by 'old' (vxworks) ospl
 */

/** \brief Terminate process \b pid by progressive use of stronger kill
 * signals followed by \b checkcount cycles of status tests to confirm
 * action of kill signal. Checkcount cycles are 100ms long.
 * Blocking state is passed via \b isblocking
 *
 * \b returns appropriate \b os_resultSuccess or Fail following final
 * status check
 */

os_result
os_procServiceDestroy(
    os_int32 pid,
    os_boolean isblocking,
    os_int32 checkcount)
{
    int killResult;
    os_result osr = os_resultFail;
    os_time sleepTime;
    int waitPid;
    int exitStatus;
    os_int32 sleepCount = 0;
    os_procId pidgrp;

    /* Send the TERM signal to the spliced, which should cause the spliced  and
     * its own services to terminate cleanly.  To check spliced has terminated
     * call the waitForProcess function (up to the serviceTerminatePeriod limit).
     * If it has not terminated, we take more evasive action by sending the
     * KILL  signal and then perform the same test.  The results of that are then
     * reported.
     */

    printf("Wait %.1f seconds for all processes to terminate\n", checkcount/10.0f);
    pidgrp = getpgid(pid); /* in case needed by killgrp and avoids reading from file */

    killResult = kill (pid, SIGTERM);

    /* wait for process */

    /* use 10th of a second sleep periods */
    sleepTime.tv_sec  = 0;
    sleepTime.tv_nsec = 100000000;
    sleepCount = checkcount;

    while (sleepCount > 0)
    {
        if (isblocking)
        {
            /* Unless it has been waited for with wait or waitpid, a child process
             * that terminates becomes a "zombie".   That is the case in blocking
             * mode : we cannot just do the kill (pid, 0) test, since the 0 signal
             * will always be delivered to the zombie.
             * Instead, we have to do the waitpid test, and if it returns successfully,
             * record its exit status.
             */
            waitPid = waitpid(pid, &exitStatus, WNOHANG);
            if(waitPid > 0)
            {
                /* the process has now terminated, obtain its exit status */

                OS_REPORT_3(OS_INFO, "setExitStatus", 0,
                    "Process spliced <%d> %s %d",
                    pid,
                    WIFEXITED(exitStatus) ? "exited normally, exit status:" : (
                        WIFSIGNALED(exitStatus) ? "terminated due to an unhandled signal:" : "stopped with an unknown status"),
                        WIFEXITED(exitStatus) ? WEXITSTATUS(exitStatus) : (
                        WIFSIGNALED(exitStatus) ? WTERMSIG(exitStatus) : -1));

                osr = os_resultSuccess;
                break;
            }
            else if (waitPid == -1 && errno == ECHILD)
            {
                /* already gone, no exit status information available */
                osr = os_resultSuccess;
                break;
            }
        }
        else if (kill(pid, 0) == -1)
        {
            /* unable to send signal to the process so it must have terminated */
            osr = os_resultSuccess;
            break;
        }

        if (sleepCount % 10 == 0)
        {
           /* Only print 1 . per second */
            printf (".");
            fflush(stdout);
        }
        os_nanoSleep(sleepTime);    /* shutdown attempt interval */
        sleepCount--;

    }

    /* since (unlike windows) associated processes are not killed explictly
     * need to attempt killgrp before escalating kill signal */

    if(killpg(pidgrp, 0) == 0)     /* is this ok for blocking? */
    {
        killResult = killpg (pidgrp, SIGKILL);
        OS_REPORT_2(OS_INFO, "removeProcesses", 0, "Sent KILL signal to pg %d - killpg returned %d", pidgrp, killResult);
        if (killResult == 0)
        {
            osr = os_resultSuccess;
        }
        else
        {
            osr = os_resultFail;
        }
    }

    if (osr != os_resultSuccess)
    {
        killResult = kill (pid, SIGKILL);   /* escalate kill signal */

        /* wait for process */

        sleepCount = checkcount;
        osr = os_resultFail;

        while (sleepCount > 0)
        {
            if (isblocking)
            {
                /* Unless it has been waited for with wait or waitpid, a child process
                 * that terminates becomes a "zombie".   That is the case in blocking
                 * mode : we cannot just do the kill (pid, 0) test, since the 0 signal
                 * will always be delivered to the zombie.
                 * Instead, we have to do the waitpid test, and if it returns successfully,
                 * record its exit status.
                 */
                waitPid = waitpid(pid, &exitStatus, WNOHANG);
                if(waitPid > 0)
                {
                    /* the process has now terminated, obtain its exit status */

                    OS_REPORT_3(OS_INFO, "setExitStatus", 0,
                        "Process spliced <%d> %s %d",
                        pid,
                        WIFEXITED(exitStatus) ? "exited normally, exit status:" : (
                            WIFSIGNALED(exitStatus) ? "terminated due to an unhandled signal:" : "stopped with an unknown status"),
                            WIFEXITED(exitStatus) ? WEXITSTATUS(exitStatus) : (
                            WIFSIGNALED(exitStatus) ? WTERMSIG(exitStatus) : -1));

                    osr = os_resultSuccess;
                    break;
                }
                else if (waitPid == -1 && errno == ECHILD)
                {
                    /* already gone, no exit status information available */
                    osr = os_resultSuccess;
                    break;
                }
            }
            else if (kill(pid, 0) == -1)
            {
                /* unable to send signal to the process so it must have terminated */
                osr = os_resultSuccess;
                break;
            }

            if (sleepCount % 10 == 0)
            {
               /* Only print 1 . per second */
                printf (".");
                fflush(stdout);
            }

            os_nanoSleep(sleepTime);  /* shutdown attempt interval */
            sleepCount--;

        }

        if(killpg(pidgrp, 0) == 0)     /* is this ok for blocking? */
        {
            killResult = killpg (pidgrp, SIGKILL);
            OS_REPORT_2(OS_INFO, "removeProcesses", 0, "Sent KILL signal to pg %d - killpg returned %d", pidgrp, killResult);
            if (killResult == 0)
            {
                osr = os_resultSuccess;
            }
            else
            {
                osr = os_resultFail;
            }
        }
    }

    printf ("\n");
    fflush(stdout);

    return osr;
}

#endif

/** \brief Send a signal to the identified process
 *
 * \b os_procDestroy sends the dignal \b signal
 * to process \b procId by calling \b kill with
 * the same parameters.
 *
 * - If \b kill returns a value != -1, \b os_resultSuccess is
 *   returned.
 * - If \b kill returns with -1 and \b errno is set to \b EINVAL,
 *   \b os_resultInvalid is returned to indicate an invalid
 *   parameter.
 * - If \b kill returns with -1 and \b errno is set to \b ESRCH,
 *   \b os_resultUnavailable is returned to indicate that
 *   the identified process is not found.
 * - On any other return from \b kill, \b os_resultFail is returned.
 */
#ifndef INTEGRITY
os_result
os_procDestroy(
    os_procId procId,
    os_int32 signal)
{
    os_result rv;

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }

    if (kill(procId, signal) == -1) {
        if (errno == EINVAL) {
            rv = os_resultInvalid;
        } else if (errno == ESRCH) {
            rv = os_resultUnavailable;
        } else {
            rv = os_resultFail;
        }
    } else {
        rv = os_resultSuccess;
    }
    return rv;
}


#if !defined VXWORKS_RTP && !defined OS_RTEMS_DEFS_H  && !defined PIKEOS_POSIX
/** \brief Get the process effective scheduling class
 *
 * Possible Results:
 * - process scheduling class is OS_SCHED_REALTIME
 * - process scheduling class is OS_SCHED_TIMESHARE
 * - process scheduling class is OS_SCHED_DEFAULT if
 *   the class effective could not be determined
 */
os_schedClass
os_procAttrGetClass(void)
{
    os_schedClass class;
    int policy;

    policy = sched_getscheduler(getpid());
    switch (policy)
    {
       case SCHED_FIFO:
       case SCHED_RR:
          class = OS_SCHED_REALTIME;
          break;
       case SCHED_OTHER:
          class = OS_SCHED_TIMESHARE;
          break;
       case -1:
          OS_REPORT_1(OS_WARNING, "os_procAttrGetClass", 1,
                      "sched_getscheduler failed with error %d", errno);
          class = OS_SCHED_DEFAULT;
          break;
       default:
          OS_REPORT_1(OS_WARNING, "os_procAttrGetClass", 1,
                      "sched_getscheduler unexpected return value %d", policy);
          class = OS_SCHED_DEFAULT;
          break;
    }
    return class;
}

/** \brief Get the process effective scheduling priority
 *
 * Possible Results:
 * - any platform and scheduling class dependent valid priority
 */
os_int32
os_procAttrGetPriority(void)
{
    struct sched_param param;

    param.sched_priority = 0;
    if (sched_getparam(getpid(), &param) == -1)
    {
       OS_REPORT_1 (OS_WARNING, "os_procAttrGetPriority", 1,
                    "sched_getparam failed with error %d", errno);
    }
    return param.sched_priority;
}
#endif

#if !defined ( OSPL_NO_VMEM ) && !defined(_POSIX_MEMLOCK)
#error "Error: the posix implementation on this platform does not support page locking!"
#endif

os_result
os_procMLockAll(
    os_uint flags)
{
#ifndef OSPL_NO_VMEM
    int f;
    int r;
    os_result result;

    f = 0;
    if (flags & OS_MEMLOCK_CURRENT) {
        f |= MCL_CURRENT;
    }
    if (flags & OS_MEMLOCK_FUTURE) {
        f |= MCL_FUTURE;
    }

    r = mlockall(f);
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        if (errno == EPERM) {
            OS_REPORT(OS_ERROR, "os_procMLockAll",
                0, "Current process has insufficient privilege");
        } else {
            if (errno == ENOMEM) {
                OS_REPORT(OS_ERROR, "os_procMLockAll",
                    0, "Current process has non-zero RLIMIT_MEMLOCK");
            }
        }
        result = os_resultFail;
    }

    return result;
#else
    return os_resultSuccess;
#endif
}

#if !defined( OSPL_NO_VMEM ) && ! defined( _POSIX_MEMLOCK_RANGE )
#error "Error: the posix implementation on this platform does not support page range locking!"
#endif

os_result
os_procMLock(
    const void *addr,
    os_address length)
{
#ifndef OSPL_NO_VMEM
    int r;
    os_result result;

    r = (int) mlock(addr, (size_t)length);
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        if (errno == EPERM) {
            OS_REPORT(OS_ERROR, "os_procMLock",
                0, "Current process has insufficient privilege");
        } else {
            if (errno == ENOMEM) {
                OS_REPORT(OS_ERROR, "os_procMLock",
                    0, "Current process has non-zero RLIMIT_MEMLOCK");
            }
        }
        result = os_resultFail;
    }
    return result;
#else
    return os_resultSuccess;
#endif
}

os_result
os_procMUnlock(
    const void *addr,
    os_address length)
{
#ifndef OSPL_NO_VMEM
    int r;
    os_result result;

    r = (int)munlock(addr, (size_t)length);
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        if (errno == EPERM) {
            OS_REPORT(OS_ERROR, "os_procMLock",
                0, "Current process has insufficient privilege");
        } else {
            if (errno == ENOMEM) {
                OS_REPORT(OS_ERROR, "os_procMLock",
                    0, "Current process has non-zero RLIMIT_MEMLOCK");
            }
        }
        result = os_resultFail;
    }
    return result;
#else
    return os_resultSuccess;
#endif
}

/** \brief  re-enable paging for calling process.
 *
 *  Enables paging for all pages mapped into the address space ofi
 *  the calling process.
 */
os_result
os_procMUnlockAll(void)
{
#ifndef OSPL_NO_VMEM
    int r;
    os_result result;

    r = munlockall();
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
#else
    return os_resultSuccess;
#endif
}

#endif
