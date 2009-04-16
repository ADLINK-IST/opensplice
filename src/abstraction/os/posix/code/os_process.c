/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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

#include <os_process.h>
#include <../posix/code/os__process.h>
#include <os_heap.h>
#include <os_report.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
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

/** \brief pointer to environment variables */
extern char **environ;
static os_procTerminationHandler _ospl_termHandler   = (os_procTerminationHandler)0;

#ifndef INTEGRITY
#define _SIGNALVECTOR_(sig) _ospl_oldSignalVector##sig
static struct sigaction _SIGNALVECTOR_(SIGINT);
static struct sigaction _SIGNALVECTOR_(SIGHUP);
static struct sigaction _SIGNALVECTOR_(SIGPIPE);
static struct sigaction _SIGNALVECTOR_(SIGALRM);
static struct sigaction _SIGNALVECTOR_(SIGTERM);
static struct sigaction _SIGNALVECTOR_(SIGUSR1);
static struct sigaction _SIGNALVECTOR_(SIGUSR2);
static struct sigaction _SIGNALVECTOR_(SIGPOLL);
static struct sigaction _SIGNALVECTOR_(SIGVTALRM);
static struct sigaction _SIGNALVECTOR_(SIGPROF);

#define OSPL_SIGNALHANDLERTHREAD_TERMINATE 1 /* Instruct thread to terminate */
#define OSPL_SIGNALHANDLERTHREAD_EXIT      2 /* Instruct thread to call exit() */

static pthread_t _ospl_signalHandlerThreadId;
static int _ospl_signalHandlerThreadTerminate = 0;
static pthread_mutex_t _ospl_signalHandlerThreadMutex;
static pthread_cond_t  _ospl_signalHandlerThreadCondition;

/* private functions */
static void
signalHandler(
    int sig,
    siginfo_t *info,
    void *arg)
{
    os_int32 terminate;
    os_terminationType reason;

    if (_ospl_termHandler) {
        switch (sig) {
        default:
            reason = OS_TERMINATION_NORMAL;
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
        /* Do not lock _ospl_signalHandlerThreadMutex when the
         * signalHandler is called from the _ospl_signalHandlerThread
         */
        if (pthread_equal(pthread_self(), _ospl_signalHandlerThreadId)) {
            _ospl_signalHandlerThreadTerminate = OSPL_SIGNALHANDLERTHREAD_EXIT;
            pthread_cond_broadcast(&_ospl_signalHandlerThreadCondition);
        } else {
            pthread_mutex_lock(&_ospl_signalHandlerThreadMutex);
            _ospl_signalHandlerThreadTerminate = OSPL_SIGNALHANDLERTHREAD_EXIT;
            pthread_cond_broadcast(&_ospl_signalHandlerThreadCondition);
            pthread_mutex_unlock(&_ospl_signalHandlerThreadMutex);
        } 
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

    sigemptyset(&sigset);
    pthread_sigmask(SIG_SETMASK, &sigset, NULL);

    pthread_mutex_lock(&_ospl_signalHandlerThreadMutex);
    while (!_ospl_signalHandlerThreadTerminate) {
        pthread_cond_wait(&_ospl_signalHandlerThreadCondition, &_ospl_signalHandlerThreadMutex);
    }
    pthread_mutex_unlock(&_ospl_signalHandlerThreadMutex);

    if (_ospl_signalHandlerThreadTerminate == OSPL_SIGNALHANDLERTHREAD_EXIT) {
        exit(0);
    }
    return NULL;
}

#define _SIGACTION_(sig) sigaction(sig,&action,&_ospl_oldSignalVector##sig)
#define _SIGCURRENTACTION_(sig) sigaction(sig, NULL, &_ospl_oldSignalVector##sig)
#define _SIGDEFAULT_(sig) (_ospl_oldSignalVector##sig.sa_handler == SIG_DFL)
#endif

/* protected functions */
void
os_processModuleInit(void)
{
#if !defined INTEGRITY && !defined VXWORKS_RTP
    struct sigaction action;
    pthread_attr_t      thrAttr;
    pthread_mutexattr_t mtxAttr;
    pthread_condattr_t  cvAttr;

    pthread_mutexattr_init(&mtxAttr);
    pthread_mutexattr_setpshared (&mtxAttr, PTHREAD_PROCESS_PRIVATE);
    pthread_mutex_init(&_ospl_signalHandlerThreadMutex, &mtxAttr);

    pthread_condattr_init(&cvAttr);
    pthread_condattr_setpshared (&cvAttr, PTHREAD_PROCESS_PRIVATE);
    pthread_cond_init(&_ospl_signalHandlerThreadCondition, &cvAttr);

    pthread_attr_init(&thrAttr);
    pthread_attr_setstacksize(&thrAttr, 1024); /* 1KB */
    pthread_create(&_ospl_signalHandlerThreadId, &thrAttr, signalHandlerThread, (void*)0);

    /* install signal handlers */
    action.sa_handler = 0;
    action.sa_sigaction = signalHandler;
    sigfillset(&action.sa_mask); /* block all signals during handling of a signal */
    action.sa_flags = SA_SIGINFO;

    _SIGCURRENTACTION_(SIGINT);
    if (_SIGDEFAULT_(SIGINT)) {
        _SIGACTION_(SIGINT);
    }

    _SIGCURRENTACTION_(SIGHUP);
    if (_SIGDEFAULT_(SIGHUP)) {
        _SIGACTION_(SIGHUP);
    }

    _SIGCURRENTACTION_(SIGPIPE);
    if (_SIGDEFAULT_(SIGPIPE)) {
        _SIGACTION_(SIGPIPE);
    }

    _SIGCURRENTACTION_(SIGALRM);
    if (_SIGDEFAULT_(SIGALRM)) {
        _SIGACTION_(SIGALRM);
    }

    _SIGCURRENTACTION_(SIGTERM);
    if (_SIGDEFAULT_(SIGTERM)) {
        _SIGACTION_(SIGTERM);
    }

    _SIGCURRENTACTION_(SIGUSR1);
    if (_SIGDEFAULT_(SIGUSR1)) {
        _SIGACTION_(SIGUSR1);
    }

    _SIGCURRENTACTION_(SIGUSR2);
    if (_SIGDEFAULT_(SIGUSR2)) {
        _SIGACTION_(SIGUSR2);
    }

    _SIGCURRENTACTION_(SIGPOLL);
    if (_SIGDEFAULT_(SIGPOLL)) {
        _SIGACTION_(SIGPOLL);
    }

    _SIGCURRENTACTION_(SIGVTALRM);
    if (_SIGDEFAULT_(SIGVTALRM)) {
        _SIGACTION_(SIGVTALRM);
    }

    _SIGCURRENTACTION_(SIGPROF);
    if (_SIGDEFAULT_(SIGPROF)) {
        _SIGACTION_(SIGPROF);
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
#if !defined INTEGRITY && !defined VXWORKS_RTP
    /* deinstall signal handlers */
    _SIGACTION_(SIGINT);
    _SIGACTION_(SIGHUP);
    _SIGACTION_(SIGPIPE);
    _SIGACTION_(SIGALRM);
    _SIGACTION_(SIGTERM);
    _SIGACTION_(SIGUSR1);
    _SIGACTION_(SIGUSR2);
    _SIGACTION_(SIGPOLL);
    _SIGACTION_(SIGVTALRM);
    _SIGACTION_(SIGPROF);

    _ospl_signalHandlerThreadTerminate = 1;
    if (_ospl_signalHandlerThreadId != pthread_self()) {
        pthread_mutex_lock(&_ospl_signalHandlerThreadMutex);
        _ospl_signalHandlerThreadTerminate = OSPL_SIGNALHANDLERTHREAD_TERMINATE;
        pthread_cond_broadcast(&_ospl_signalHandlerThreadCondition);
        pthread_mutex_unlock(&_ospl_signalHandlerThreadMutex);
    }
#endif
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
 * to be called when the process exists.
 * The standard POSIX implementation guarantees the
 * required order of execution of the exit handlers.
 */
void
os_procAtExit(
    void (*function)(void))
{
    assert (function != NULL);
    atexit (function);
    return;
}

/** \brief Terminate the process and return the status
 *         the the parent process
 *
 * \b os_procExit terminates the process by calling \b exit.
 */
void
os_procExit(
    os_exitStatus status)
{
    assert(status != OS_EXIT_SUCCESS || status != OS_EXIT_FAILURE);
    exit((signed int)status);
    return;
}

#ifndef VXWORKS_RTP
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
#ifndef INTEGRITY
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
    struct sched_param sched_param;
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
        strcpy(argin, arguments);
        argv[0] = os_malloc(strlen(name) + 1);
        argv[0] = strcpy(argv[0], name);
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
	        if (getuid() == 0 || geteuid() == 0) {
		    sched_param.sched_priority = procAttr->schedPriority;
		    if (sched_setscheduler(pid, SCHED_FIFO, &sched_param) == -1) {
			OS_REPORT_2(OS_WARNING, "os_procCreate", 1,
                                    "sched_setscheduler failed with error %d (%s)", errno, name);
		    }
	        } else {
		    OS_REPORT_1(OS_WARNING, "os_procCreate", 2,
                                "scheduling policy can not be set because of privilege problems (%s)", name);
	        }
	    } else {
	        if (getuid() == 0 || geteuid() == 0) {
	            sched_param.sched_priority = procAttr->schedPriority;
	            if (sched_setscheduler(pid, SCHED_OTHER, &sched_param) == -1) {
		        OS_REPORT_2(OS_WARNING, "os_procCreate", 1,
                                    "sched_setscheduler failed with error %d (%s)", errno, name);
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
#ifndef INTEGRITY
os_result
os_procCheckStatus(
    os_procId procId,
    os_int32 *status)
{
    os_procId result;
    os_result rv;
    int les;

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

/** \brief Figure out the identity of the current process
 *
 * \b os_procFigureIdentity determines the numeric identity
 * of a process. On UNIX the process name can only be determined
 * by access to argv[0]. This is not the case for this implementation.
 * If however started via \b os_procCreate the name of the process
 * can be determined via the environment variable \b SPLICE_PROCNAME
 */
os_int32
os_procFigureIdentity(
    char *procIdentity,
    os_uint32 procIdentitySize)
{
    os_int32 size;
    char *process_name;

    process_name = getenv("SPLICE_PROCNAME");
    if (process_name != NULL) {
	size = snprintf(procIdentity, procIdentitySize, "%s (%d)", process_name, (int)getpid());
    } else {
	size = snprintf(procIdentity, procIdentitySize, "%d", (int)getpid());
    }
    return size;
}

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

#ifndef _POSIX_MEMLOCK
#error "Error: the posix implementation on this platform does not support page locking!"
#endif

os_result
os_procMLockAll(
    os_uint flags)
{
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
}

#ifndef _POSIX_MEMLOCK_RANGE
#error "Error: the posix implementation on this platform does not support page range locking!"
#endif

os_result
os_procMLock(
    const void *addr,
    os_uint length)
{
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
}

os_result
os_procMUnlock(
    const void *addr,
    os_uint length)
{
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
}

/** \brief  re-enable paging for calling process.
 *
 *  Enables paging for all pages mapped into the address space ofi
 *  the calling process.
 */
os_result
os_procMUnlockAll(void)
{
    int r;
    os_result result;

    r = munlockall();
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
}

#endif
