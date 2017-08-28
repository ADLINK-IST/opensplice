/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "os_signalHandler.h"
#include "../common/include/os_signalHandlerCallback.h"

#include "os_errno.h"
#include "os_heap.h"
#include "os_stdlib.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#ifndef __Lynx__
#include <ucontext.h>
#endif
#ifndef INTEGRITY
#include <signal.h>
#endif
#include <stdio.h>
#ifndef VXWORKS_RTP
#include <pthread.h>
#endif

#include "os_report.h"
#include "os_thread.h"
#include "os_abstract.h"
#include "os_init.h"
#include "os_mutex.h"



#define SIGNAL_THREAD_STOP (-1)
#define lengthof(a) ((int)(sizeof(a) / sizeof(*a)))

/* These are the signals for which the User shall detach from the Domain.
 * For these signals a handler is set to unregister the user from the domain.
 */
static const int exceptions[] = {
    SIGILL, SIGTRAP, SIGABRT, SIGFPE, SIGBUS, SIGSEGV, SIGSYS
};

static const int quits[] = {
    SIGQUIT, SIGPIPE, SIGINT, SIGTERM, SIGHUP
};

static unsigned quits_len = lengthof(quits);

/* If any of the SIGFPE, SIGILL, SIGSEGV, or SIGBUS signals are generated while
 * they are blocked, the result is undefined, unless the signal was generated
 * by the kill() function, the sigqueue() function, or the raise() function. */
static const int excludes[] = {
    SIGFPE, SIGILL, SIGSEGV, SIGBUS
};

static sigset_t exceptionsMask;
static sigset_t quitsMask;


struct os_signalHandler_s {
    os_threadId threadId;   /* The thread id of the signalHandlerThread. */
    int pipeIn[2]; /* a pipe to send a signal to the signal handler thread. */
    int pipeOut[2]; /* a pipe to receive a result from the signal handler thread. */
    os_signalHandlerCallbackInfo callbackInfo;
    os_sigaction action;
    os_boolean handleExceptions;
};

#ifdef NSIG
#define OS_NSIG (NSIG)
#elif defined(_NSIG)
#define OS_NSIG (_NSIG)
#elif defined(SIGRTMAX)
#define OS_NSIG (SIGRTMAX)
#else
#define OS_NSIG (65)  /* 64+1; allows indexing of potential signal 64  (typical value of NSIG/_NSIG/SIGRTMAX) in array */
#endif


static os_sigaction old_signalHandler[OS_NSIG];

static os_signalHandler signalHandlerObj = NULL;

static int installSignalHandler = 1;

static void os__signalHandlerThreadStop(os_signalHandler _this);

struct sig_context {
    siginfo_t info;
    os_ulong_int ThreadIdSignalRaised;
    os_int32 domainId;
#ifdef OS_HAS_UCONTEXT_T
    ucontext_t uc;
#endif
};

/* Include the common implementation for callback handling */
#include "../common/code/os_signalHandlerCallback.c"

void
os_signalHandlerIgnoreJavaSignals (void)
{
    /* Ignore the SIGHUP, SIGINT and SIGTERM on POSIX platforms for JAVA see OSPL-6588
     * so only first 2 signals in the quits array count
     */
    quits_len = 2;
}

/* private functions */
static int
isSignallingSafe(
    int reportReason)
{
#ifndef JAVA_IS_PERC
    if(!installSignalHandler && reportReason){
        OS_REPORT(OS_WARNING, "OS abstraction layer", 0,
                  "Did not install signal handlers to cleanup resources.\n"\
                  "              To ensure cleanup for Java applications, the path to the 'jsig' library\n"\
                  "              (libjsig.so) must be set in the LD_PRELOAD environment variable.\n"\
                  "              This library is part of your Java distribution.\n"\
                  "              To ensure proper cleanup set this before starting your application.");
    }
#else
    OS_UNUSED_ARG(reportReason);
#endif
    return installSignalHandler;
}

/**
 * This call causes an immediate forced ABRT of the current process.
 * It is a forced ABRT because any potentially chained signal-handlers for
 * SIG_ABRT are ignored. The handler is explicitly reset to SIG_DFL. This call
 * will never return execution to the caller.
 * The first count characters found in panicmsg will be printed to
 * STDERR_FILENO, after which the ABRT will be performed.
 * @param panicmsg The message to be printed
 * @param count The length of the message to be printed
 */
static void
panic(
    const char * const panicmsg,
    const size_t count)
{
    sigset_t abort_set;
    os_write (STDERR_FILENO, panicmsg, count);
    sigemptyset (&abort_set);
    sigaddset (&abort_set, SIGABRT);
    /* Forcedly set SIG_DFL handler; there is no possibility anymore for a
     * a graceful exit. */
    signal (SIGABRT, SIG_DFL);
    sigprocmask (SIG_UNBLOCK, &abort_set, NULL);
    raise (SIGABRT);
    /* This line will not be reached anymore... */
}

/**
 * Returns if the current thread is the signalHandlerThread.
 *
 * @remarks Do not perform any signal-handling context unsafe operations in this
 * function.
 *
 * @return OS_TRUE if the current thread is the signalHandlerThread, or
 *         OS_FALSE if it's not.
 */
static os_boolean
inSignalHandlerThread (void)
{
    os_int match;
    os_signalHandler _this = signalHandlerObj;

#ifndef NDEBUG
    /* Assert preconditions (regular assert may trigger this action, so it
     * is not used here). */
    if (_this == NULL) {
        const char panicmsg[] = "Assertion failed: _this != NULL in " __FILE__ ":inSignalHandlerThread\n";
        panic(panicmsg, sizeof(panicmsg) - 1);
        /* This line will not be reached anymore */
    }
#endif

    match = os_threadIdToInteger (_this->threadId) ==
        os_threadIdToInteger (os_threadIdSelf ());

    return match ? OS_TRUE : OS_FALSE;
}

/**
 * This call runs in signalhandler context and notifies the signalHandlerThread
 * of a signal. This function will write the os_signalHandlerPipe through a pipe to the
 * signalHandlerThread in one atomic write and optionally synchronizes on the
 * result of the signalHandlerThread, (which will be written to a pipe by the
 * signalHandlerThread in case of a synchronous exception). If sigInfo
 * != NULL, this function will wait for this input, so the caller must assure
 * that sync is only set for exceptions, otherwise this call may block
 * indefinitely.
 * The call returns nothing on success/fail; any failure will cause a panic
 * (immediate forced ABRT), causing the function not to return. So if the call
 * returns, the post-conditions hold.
 *
 * @remarks Do not perform any signal-handling context unsafe operations in this
 * function.
 *
 * @param sigInfo Information and context of the signal that was caught
 * @param sync A pointer to storage for the signal-context on which was
 * synchronised
 * @post If this call returns, the signal-context in info was successfully
 * and atomically written to the pipe of the signalHandlerThread and if sigInfo
 * != NULL sizeof(struct sig_context) bytes were successfully read from the pipe. Any
 * error will cause a panic (immediate forced ABRT).
 */
static void
signalHandlerThreadNotify(
    struct sig_context sigInfo,
    struct sig_context *sync)
{
    ssize_t r;
    os_signalHandler _this = signalHandlerObj;

#ifndef NDEBUG
    /* Assert preconditions (regular assert may trigger this action, so it
     * is not used here). */
    if(_this == NULL) {
        const char panicmsg[] = "Assertion failed: _this != NULL in " __FILE__ ":signalHandlerThreadNotify\n";
        panic(panicmsg, sizeof(panicmsg) - 1);
        /* This line will not be reached anymore */
    }
    if(sync && sigismember(&exceptionsMask, sigInfo.info.si_signo) == 0){
        /* In this case signalHandlerThreadNotify may wait indefinately, since
         * the signalHandlerThread will not notify when it is done handling the
         * signal. */
        const char panicmsg[] = "Assertion failed: sync != NULL && sigInfo.info.si_signo is not an exception in " __FILE__ ":signalHandlerThreadNotify\n";
        panic(panicmsg, sizeof(panicmsg) - 1);
        /* This line will not be reached anymore */
    }
    if(sync && sigInfo.info.si_code == SI_USER){
        /* An asynchronously received exception may not be handled synchronously. */
        const char panicmsg[] = "Assertion failed: sync != NULL && sigInfo.info.si_code is SI_USER, meaning the signal was asynchronously delivered in " __FILE__ ":signalHandlerThreadNotify\n";
        panic(panicmsg, sizeof(panicmsg) - 1);
        /* This line will not be reached anymore */
    }
#endif
   /* The following write and read statement implement a synchronous call
    * to the signalHandlerThread; the signalHandlerThread will detach the
    * user-layer from the Domain. If sync is !NULL, this operation will
    * block until the signalHandlerThread has finished.
    * NOTE: The signalHandlerThread will ONLY signal that it has finished for
    * exceptions; calling signalHandlerThreadDetach with sync == NULL for
    * quits will cause starvation!
    * Due to the compile-time constraint enforced through the
    * struct atomic_write_constraint, the write of info is ensured to be atomic. */
    do {
        r = write(_this->pipeIn[1], &sigInfo, sizeof(sigInfo));
        if (r == -1 && os_getErrno() != EINTR){
            /* We understand that EINTR may have occurred, the rest of the
             * possible errors are reason for serious panic; the only means we
             * had of handling this cleanly is this piece of code :s, so panic
             * it is. */
            const char panicmsg[] = "FATAL ERROR: Atomic write to pipe in signalHandlerThreadNotify failed\n";
            panic(panicmsg, sizeof(panicmsg) - 1);
            /* This line will not be reached anymore */
        }
    } while(r == -1);

    if(sync){ /* If caller wants to wait for the signal-handler thread to finish */
        size_t nread = 0;

        /* There is no guarantee that read will return all available data/all
         * data that is atomically written will be available for an atomic read
         * (just that writes of size < PIPE_BUF will not be interleaved). So en-
         * sure we get the entire struct. */
        do{
            r = read(_this->pipeOut[0], sync + nread, sizeof(*sync) - nread);
            if (r == -1 && os_getErrno() != EINTR){
                /* We understand that EINTR may have occurred, the rest of the
                 * possible errors are reason for serious panic; the only means we
                 * had of handling this cleanly is this piece of code :s, so panic
                 * it is. */
                const char panicmsg[] = "FATAL ERROR: Read from pipe in signalHandlerThreadNotify failed\n";
                panic(panicmsg, sizeof(panicmsg) - 1);
                /* This line will not be reached anymore */
            }
            if (r > 0) {
                nread += (size_t) r;
            }
        } while(nread < sizeof(*sync));
    }
    return;
}



/* In order to prevent interleaved writes on the pipe or the need to introduce
 * logic to recover from them, we do a compile-time assertion here. It is
 * expected to always pass on POSIX.1-2001 compliant platforms, since
 * POSIX.1-2001 requires PIPE_BUF to be at least 512. On Linux PIPE_BUF is 4096.
 * The size of struct sigcontext is approximately 476 bytes. */

struct atomic_write_constraint {
    char require_sizeof_os_signalHandlerPipe_lte_PIPE_BUF [sizeof(struct sig_context) <= PIPE_BUF];
    char non_empty_dummy_last_member[1];
};

/**
 * This is the signal-handler routine that is performed in case of a signal. It
 * distinguishes:
 * - synchronous:
 *     - exceptions
 * - asynchronous:
 *     - exceptions
 *     - quits (termination requests).
 *
 * @remarks Do not perform any signal-handling context unsafe operations in this
 * function.
 */
static void
signalHandler(
    int sig,
    siginfo_t *info,
    void* uap)
{
    struct sig_context sync;
    struct sig_context sigInfo;

    /* info can be NULL on Solaris */
    if (info == NULL) {
        /* Pretend that it was an SI_USER signal. */
        memset(&sigInfo.info, 0, sizeof(siginfo_t));
        sigInfo.info.si_signo = sig;
        sigInfo.info.si_code = SI_USER;
        sigInfo.info.si_pid = getpid();
        sigInfo.info.si_uid = getuid();
    } else {
        sigInfo.info = *info;
    }
    sigInfo.ThreadIdSignalRaised = os_threadIdToInteger(os_threadIdSelf());
    sigInfo.domainId = os_reportGetDomain();
#ifdef OS_HAS_UCONTEXT_T
    sigInfo.uc = *(ucontext_t *)uap;
#endif

    /* WARNING: Don't do any async/signalling-unsafe calls here (so refrain from
     * using OS_REPORT_X and the like). */
    if (sigismember(&exceptionsMask, sig) == 1 && sigInfo.info.si_code != SI_USER){
        os_sigaction *xo;

        if (inSignalHandlerThread()) {
            /* The signalHandlerThread caught an exception (synchronous)
             * itself. The fact that the signalHandlerThread caught an
             * exception means there is a bug in the error handling code. */
            const char panicmsg[] = "FATAL ERROR: Synchronously trapped signal in signalHandlerThread\n";
            panic(panicmsg, sizeof(panicmsg) - 1);
            /* This line will not be reached anymore */
        }

        /* We have an exception (synchronous) here. The assumption is
         * that exception don't occur in middleware-code, so we can
         * synchronously call the signalHandlerThread in order to detach user-
         * layer from all Domains (kernels). */
        signalHandlerThreadNotify(sigInfo, &sync);
        /* BEWARE: This may be an interleaved handling of an exception, so use
         * sync from now on instead of sigInfo.*/

        /* Reset the original signal-handler. If the exception was synchronous,
         * running out of this handler will regenerate the signal, which will
         * then be handled by the original signal-handler. Otherwise it needs
         * to be re-raised. */
        xo = &old_signalHandler[sync.info.si_signo];
        os_sigactionSet(sync.info.si_signo, xo, NULL);

        /* Positive values are reserved for kernel-generated signals, i.e.,
         * actual synchronous hard errors. The rest are 'soft' errors and thus
         * need to be re-raised. */
        if(sigInfo.info.si_code <= 0){
            raise(sig);
        }
    } else {
        /* Pass signal to signal-handler thread for asynchronous handling */
        signalHandlerThreadNotify(sigInfo, NULL);
    }
}

os_result
os_signalHandlerFinishExitRequest(
    os_callbackArg arg)
{
    os_result r = os_resultSuccess;
    int sig = (int)(os_address)arg.sig;
    os_sigaction * xo;

    /* This is a request from the application to round up an (asynchronous)
     * exit-request. */
    if (sig < 1 || sig >= OS_NSIG){
        OS_REPORT(OS_WARNING,
            "os_signalHandlerFinishExitRequest", 0,
            "Callback-arg contains invalid signal, value out of range 1-%d: arg = %d",
            OS_NSIG, sig);
        r = os_resultInvalid;
    } else if (sigismember(&quitsMask, sig) == 0){
#if OS_NSIG >= 100
#error "Worst-case allocation assumes max. signal of 99, which apparently is not correct"
#endif
        /* We know which signal-number exist, all take at most 2 digits + ", ",
         * so allocate worst-case 4 * quits_len */
        char *expected = os_malloc(quits_len * 4 + 1);
        if(expected){
            unsigned i;
            int pos;
            assert(quits_len > 0);
            pos = sprintf(expected, "%d", quits[0]);
            for(i = 1; i < quits_len; i++){
                pos += sprintf(expected + pos, ", %d", quits[i]);
            }
        }
        OS_REPORT(OS_WARNING,
            "os_signalHandlerFinishExitRequest", 0,
            "Unexpected Signal, value out of range: signal = %d. Expected one of %s.",
            sig, expected ? expected : "the asynchronous exit request signals");
        os_free(expected);
        r = os_resultInvalid;
    }

    if(r == os_resultSuccess){
        /* We need to restore the original signal-handler and than re-raise the
         * original signal. */
        xo = &old_signalHandler[sig];
        if(os_sigactionSet(sig, xo, NULL) != 0){
            OS_REPORT(OS_WARNING,
               "os_signalHandlerFinishExitRequest", 0,
               "Resetting original signal handler for signal %d failed, sigaction did not return 0.",sig);
            abort(); /* We were unable to reset the original handler, which is pretty serious. */
        } else {
            os_sigset current_sigset, old_sigset;
            /* Determine the current mask, and make sure that the signal to be
             * raised is not blocked (this code is typically executed in the
             * signalHandlerThread (if callback is implemented synchronously),
             * which blocks all signals). */
            os_sigThreadSetMask(NULL, &current_sigset);
            os_sigsetDel(&current_sigset, sig);
            raise(sig);
            /* Set mask temporarily, this should raise the pending signal set above. */
            os_sigThreadSetMask(&current_sigset, &old_sigset);
            /* Reset the mask to the original state. If this is the signal-
             * HandlerThread this is essential (if sig is handled), otherwise just
             * the proper thing to do. */
            os_sigThreadSetMask(&old_sigset, NULL);
        }
    }
    return r;
}


static void *
signalHandlerThread(
    void *arg)
{
    ssize_t r;
    size_t nread;
    int sig, pid;
    struct sig_context info;
    int cont = 1;
    os_signalHandler _this = (os_signalHandler)arg;

    if (_this == NULL) return NULL;

    while (cont) {
        nread = 0;
        r = 0;
        while(nread < sizeof(info) && ((r = read(_this->pipeIn[0], &info + nread, sizeof(info) - nread)) > 0)){
            nread += (size_t) r;
        }
        if (r < 0) {
            int errorNr = os_getErrno();
            OS_REPORT(OS_ERROR,
                        "os_signalHandlerThread", 0,
                        "read(_this->pipeIn[0]) failed, error %d: %s",
                        errorNr, os_strError(errorNr));
            assert(OS_FALSE);
            sig = SIGNAL_THREAD_STOP;
        } else {
            sig = info.info.si_signo;
        }
        if(sig != SIGNAL_THREAD_STOP){
            if (sig < 1 || sig >= OS_NSIG) {
                OS_REPORT_WID(OS_WARNING,
                            "os_signalHandlerThread", 0, info.domainId,
                            "Unexpected signal, value out of range 1-%d: signal = %d",
                            OS_NSIG, sig);
            } else {
                if(sigismember(&exceptionsMask, sig) == 1){
                    if(info.info.si_code == SI_USER || info.info.si_code == SI_QUEUE){
                        /* Sent by kill or sigqueue, so we can report the origin
                         * of the asynchronous delivery. */
                        OS_REPORT_WID(OS_INFO, "signalHandlerThread", 0, info.domainId,
                            "Synchronous exception (signal %d) asynchronously received from PID %d%s, UID %d",
                            sig,
                            info.info.si_pid,
                            info.info.si_pid == getpid() ? " (self)" : "",
                            info.info.si_uid);
                    } else {
                        OS_REPORT_WID(OS_ERROR, "signalHandlerThread", 0, info.domainId,
                            "Exception (signal %d) %s in process",
                            sig,
                            /* Positive values for si_code are reserved for kernel-
                             * generated signals. This report allows to distinguish
                             * an actual kernel-signal and signals within the process
                             * like from pthread_kill(...) for example. */
                            info.info.si_code > 0 ? "occurred" : "generated");
                    }

                    /* If an exceptionCallback was set, this should always be invoked. The main
                     * goal is to protect SHM in case of an exception, even if a customer
                     * installed a handler as well. */
                    {
                        os_callbackArg cbarg;
                        cbarg.sig = (void*)(os_address)sig;
                        cbarg.ThreadId = info.ThreadIdSignalRaised;
                        os__signalHandlerExceptionCallbackInvoke(&_this->callbackInfo, cbarg);
                    }
                    if(info.info.si_code == SI_USER){
                        /* Mimic an exception by re-raising it. A random thread received the signal
                         * asynchronously, so when we raise it again another random thread will
                         * receive the signal. */
                        os_sigaction *xo;

                        /* Reset the original signal-handler. */
                        xo = &old_signalHandler[info.info.si_signo];
                        os_sigactionSet(info.info.si_signo, xo, NULL);

                        /* Since the exception was actually asynchronous (issued by an external/soft
                         * source), the original signal-handler will not be called by running
                         * out of the handler. We chain by re-raising, since SIG_DFL or SIG_IGN
                         * cannot be called directly. */
                        /* OSPL-2762: Instead of re-raising we use kill to
                           make sure the signal isn't delivered to this thread
                           as that would cause a dead lock. */
                        pid = getpid();
                        /* Don't think it's possible to not send the signal */
                        OS_REPORT_WID (OS_DEBUG, "os_signalHandlerThread", 0, info.domainId,
                            "Invoking kill (signal %d) on PID %d (self)",
                            info.info.si_signo, pid);
                        (void)kill (pid, info.info.si_signo);
                    } else {
                        /* Notify waiting signal handler to unblock. */
                        r = write(_this->pipeOut[1], &info, sizeof(info));
                        if (r<0) {
                            int errorNr = os_getErrno();
                            OS_REPORT_WID(OS_ERROR,
                                      "os_signalHandlerThread", 0, info.domainId,
                                      "write(_this->pipeOut[1]) failed, error %d: %s",
                                      errorNr, os_strError(errorNr));
                            assert(OS_FALSE);
                        }
                    }
                } else if (sigismember(&quitsMask, sig) == 1){
                    os_callbackArg cbarg;
                    OS_REPORT_WID(OS_INFO, "signalHandlerThread", 0, info.domainId,
                        "Termination request (signal %d) received from PID %d%s, UID %d",
                        sig,
                        (info.info.si_code == SI_USER) ? info.info.si_pid : getpid(),
                        (info.info.si_code != SI_USER  || info.info.si_pid == getpid()) ? " (self)" : "",
                        (info.info.si_code == SI_USER) ? info.info.si_uid : getuid());

                    cbarg.sig = (void*)(os_address)sig;
                    cbarg.ThreadId = info.ThreadIdSignalRaised;

                    /* Quit-signals which were set to SIG_IGN shouldn't have our signal-handler
                     * set at all. */
                    assert(old_signalHandler[sig].sa_handler != SIG_IGN);
                    if(old_signalHandler[sig].sa_handler == SIG_DFL){
                        (void) os__signalHandlerExitRequestCallbackInvoke(&_this->callbackInfo, cbarg);
                    } else {
                        /* Execute the original signal-handler within our safe context. For quit-signals our
                         * own exitRequestCallback is not executed. We don't chain exit-request-handlers. */
                        if ((old_signalHandler[sig].sa_flags & SA_SIGINFO) == SA_SIGINFO) {
#ifdef OS_HAS_UCONTEXT_T
                            old_signalHandler[sig].sa_sigaction(sig, &info.info, &info.uc);
#else
                            old_signalHandler[sig].sa_sigaction(sig, &info.info, NULL);
#endif
                        } else {
                            old_signalHandler[sig].sa_handler(sig);
                        }
                    }
                } /* Else do nothing */
            }
        } else {
            cont = 0;
        }
    }
    return NULL;
}

static os_result
os_signalHandlerInit(
    os_signalHandler _this);

os_result
os_signalHandlerNew(
    void)
{
    os_signalHandler _this;
    os_result result = os_resultFail;

#if !defined INTEGRITY && !defined VXWORKS_RTP
    _this = os_malloc(sizeof(*_this));
    if (_this == NULL) {
        OS_REPORT(OS_ERROR, "os_signalHandlerNew", 0, "os_malloc(%"PA_PRIuSIZE") failed.", sizeof(*_this));
        goto err_malloc;
    }
    signalHandlerObj = _this;
    if((result = os_signalHandlerInit(_this)) != os_resultSuccess){
        goto err_init;
    }
#endif
    return result;

err_init:
    signalHandlerObj = NULL;
    os_free(_this);
err_malloc:
    return os_resultFail;
}

static os_result
os_signalHandlerEnableExitSignals (
    void);

static os_result
os_signalHandlerInit(
    os_signalHandler _this)
{
    os_result result = os_resultSuccess;
    os_sigset block_all_sigset, old_sigset;
    unsigned i;
    int r;
    os_threadAttr thrAttr;

    assert(_this);

    _this->handleExceptions = OS_FALSE;

    if(os__signalHandlerCallbackInit(&_this->callbackInfo) != os_resultSuccess) {
        goto err_callbackInfoInit;
    }

    if(!isSignallingSafe(1)){
        return os_resultSuccess;
    }

    /* Initialise the exceptionsMask */
    sigemptyset(&exceptionsMask);
    for(i = 0; i < lengthof(exceptions); i++){
        sigaddset(&exceptionsMask, exceptions[i]);
    }

    /* Initialise the quitsMask */
    sigemptyset(&quitsMask);
    for(i = 0; i < quits_len; i++){
        sigaddset(&quitsMask, quits[i]);
    }

    /* create signal handling pipes */
    r = pipe(_this->pipeIn);
    if (r < 0) {
        OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0, "pipe(_this->pipeIn) failed, result = %d", r);
        goto err_pipeIn;
    }

    r = fcntl(_this->pipeIn[0], F_SETFD, 1);
    if (r < 0) {
        OS_REPORT(OS_WARNING, "os_signalHandlerInit", 0, "fcntl(_this->pipeIn[0]) failed, result = %d", r);
        goto err_pipeInFcntl;
    }

    r = fcntl(_this->pipeIn[1], F_SETFD, 1);
    if (r < 0) {
        OS_REPORT(OS_WARNING, "os_signalHandlerInit", 0, "fcntl(_this->pipeIn[1]) failed, result = %d", r);
        goto err_pipeInFcntl;
    }

    r = pipe(_this->pipeOut);
    if (r < 0) {
        OS_REPORT(OS_ERROR, "os_signalHandlerInit", 1, "pipe(_this->pipeOut) failed, result = %d", r);
        goto err_pipeOut;
    }

    r = fcntl(_this->pipeOut[0], F_SETFD, 1);
    if (r < 0) {
        OS_REPORT(OS_WARNING, "os_signalHandlerInit", 0, "fcntl(_this->pipeOut[0]) failed, result = %d", r);
        goto err_pipeOutFcntl;
    }

    r = fcntl(_this->pipeOut[1], F_SETFD, 1);
    if (r < 0) {
        OS_REPORT(OS_WARNING, "os_signalHandlerInit", 0, "fcntl(_this->pipeOut[1]) failed, result = %d", r);
        goto err_pipeOutFcntl;
    }

    /* Block all signals in order to start the signalHandlerThread with all
     * signals blocked. */
    result = os_sigsetFill(&block_all_sigset);
    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                "os_sigsetFill(&block_all_sigset) failed: %s", os_resultImage(result));
        goto err_sigsetMask;
    }

    /* Remove signals that cannot be blocked. */
    for (i = 0; i < lengthof(excludes); i++) {
        const int sig = excludes[i];
        if (os_sigsetDel(&block_all_sigset, sig) != 0) {
            OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0, "os_sigsetDel(0x%"PA_PRIxADDR", %d) failed, result = %d",
                    (os_address)&_this->action, sig, r);
            goto err_sigsetMask;
        }
    }

    result = os_sigThreadSetMask(&block_all_sigset, &old_sigset);
    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0, "os_sigThreadSetMask(0x%"PA_PRIxADDR", 0x%"PA_PRIxADDR") failed: %s",
                    (os_address)&block_all_sigset, (os_address)&old_sigset, os_resultImage(result));
        goto err_sigsetMask;
    }

    /* Setup signal handler thread. */
    os_threadAttrInit(&thrAttr);
    thrAttr.stackSize = 4*1024*1024; /* 4 MB */
    result = os_threadCreate(&_this->threadId,
                             "signalHandler",
                             &thrAttr,
                             signalHandlerThread,
                             (void*)_this);

    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                    "os_threadCreate(0x%"PA_PRIxADDR", 0x%"PA_PRIxADDR",0x%"PA_PRIxADDR",0) failed: %s",
                    (os_address)&_this->threadId,
                    (os_address)&thrAttr,
                    (os_address)signalHandlerThread,
                    os_resultImage(result));
        goto err_threadCreate;
    }

    /* Reset signal mask to original value. */
    result = os_sigThreadSetMask(&old_sigset, NULL);
    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                    "os_sigThreadSetMask(0x%"PA_PRIxADDR", NULL) failed: %s",
                    (os_address)&old_sigset, os_resultImage(result));
        goto err_sigResetMask;
    }

    /* install signal handlers */
    _this->action = os_sigactionNew(signalHandler, &block_all_sigset, SA_SIGINFO);

    if (os_signalHandlerEnableExitSignals() != os_resultSuccess) {
        goto err_enableExitHandling;
    }

    return os_resultSuccess;

/* Error handling */
err_enableExitHandling:
err_sigResetMask:
    os__signalHandlerThreadStop(_this);
err_threadCreate:
    (void) os_sigThreadSetMask(&old_sigset, NULL);
err_sigsetMask:
    /* No undo needed for fcntl's/sigsetFill */
err_pipeOutFcntl:
    (void) close(_this->pipeOut[0]);
    (void) close(_this->pipeOut[1]);
err_pipeOut:
    /* No undo needed for fcntl's */
err_pipeInFcntl:
    (void) close(_this->pipeIn[0]);
    (void) close(_this->pipeIn[1]);
err_pipeIn:
    os__signalHandlerCallbackDeinit(&_this->callbackInfo);
err_callbackInfoInit:
    return os_resultFail;
}


static os_result
os_signalHandlerEnableExitSignals (
    void)
{
    os_signalHandler _this = signalHandlerObj;
    unsigned iExit;
    int r;

    if (isSignallingSafe(0) && _this) {

        for (iExit = 0; iExit < quits_len; iExit++) {
            const int sig = quits[iExit];
            /* By passing NULL we only retrieve the currently set handler. If
             * the signal should be ignored, we don't do anything. Otherwise we
             * chain the old handler to our own.
             * man-page of sigaction only states behaviour when new
             * action is non-NULL, but all known implementations act as
             * expected. That is: return the currently set signal-hand-
             * ler (and not the previous, as the man-pages state).
             * NOTE: Not MT-safe */
            r = os_sigactionSet(sig, NULL, &old_signalHandler[sig]);
            if (r < 0) {
                OS_REPORT(OS_ERROR, "os_signalHandlerEnableQuitSignals", 0,
                        "Could not retrieve currently set signal-handler for signal %d", sig);
                goto err_exitSigSet;
            }
            if(old_signalHandler[sig].sa_handler != SIG_IGN){
                /* We don't know if the oldHandler has been modified in the mean
                 * time, since there is no way to make the signal handler reentrant.
                 * It doesn't make sense to look for modifications now, since a
                 * new modification could be on its way while we are processing
                 * the current modification.
                 * For that reason we will ignore any intermediate modifications
                 * and continue setting our own handler. Processes should therefore
                 * refrain from modifying the signal handler in multiple threads.
                 */
                r = os_sigactionSet(sig, &_this->action, NULL);
                if (r != 0) {
                    OS_REPORT(OS_ERROR, "os_signalHandlerEnableQuitSignals", 0,
                            "os_sigactionSet(%d, 0x%"PA_PRIxADDR", 0x%"PA_PRIxADDR") failed, result = %d",
                            sig, (os_address)&_this->action, (os_address)&old_signalHandler[sig], r);
                    goto err_exitSigSet;
                }
            }
        }
    }

    return os_resultSuccess;

err_exitSigSet:
    while(iExit) {
        const int sig = quits[--iExit];
        r = os_sigactionSet(sig, &old_signalHandler[sig], NULL);
        if (r < 0) {
            OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                "Failed to restore original handler: os_sigactionSet(%d, 0x%"PA_PRIxADDR", NULL) failed, result = %d",
                sig, (os_address)&old_signalHandler[sig], r);
        }
    }

    return os_resultFail;
}

os_result
os_signalHandlerEnableExceptionSignals (
    void)
{
    os_signalHandler _this = signalHandlerObj;
    unsigned i, iException;
    int r;

    if (isSignallingSafe(0) && _this) {

        for (i = 0; i < lengthof(exceptions); i++) {
            const int sig = exceptions[i];
            r = os_sigsetDel(&_this->action.sa_mask, sig);
            if (r < 0) {
                OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                        "os_sigsetDel(0x%"PA_PRIxADDR", %d) failed, result = %d",
                        (os_address)&_this->action, sig, r);
                goto err_exceptionSigSetDel;
            }
        }

        for (iException = 0; iException < lengthof(exceptions); iException++) {
            const int sig = exceptions[iException];
            /* For exceptions we always set our own signal handler, since
             * applications that cause an exception are not in a position
             * to ignore it. However, we will chain the old handler to our
             * own. */
            r = os_sigactionSet(sig, &_this->action, &old_signalHandler[sig]);
            if (r < 0) {
                OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                        "os_sigactionSet(%d, 0x%"PA_PRIxADDR", 0x%"PA_PRIxADDR") failed, result = %d",
                        sig, (os_address)&_this->action, (os_address)&old_signalHandler[sig], r);
                goto err_exceptionSigSet;
            }
        }

        _this->handleExceptions = OS_TRUE;
    }

    return os_resultSuccess;

err_exceptionSigSet:
    while(iException) {
        const int sig = exceptions[--iException];
        r = os_sigactionSet(sig, &old_signalHandler[sig], NULL);
        if (r < 0) {
            OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                      "Failed to restore original handler: os_sigactionSet(%d, 0x%"PA_PRIxADDR", NULL) failed, result = %d",
                       sig, (os_address)&old_signalHandler[sig], r);
        }
    }
err_exceptionSigSetDel:
    /* No undo needed for os_sigsetDel(...) */
    return os_resultFail;
}


static void
os__signalHandlerThreadStop(
        os_signalHandler _this)
{
    struct sig_context info;
    void *thread_result;

    assert(_this);

    memset (&info, 0, sizeof(info));
    info.info.si_signo = SIGNAL_THREAD_STOP;
    (void) write(_this->pipeIn[1], &info, sizeof(info));
    /* When the signalHandlerThread itself is the exiting thread (this can happen
     * when an exit call is done in a signalHandler installed by a customer for
     * example), we should not invoke os_threadWaitExit but just let this call
     * return immediately. */
    if (os_threadIdSelf() != _this->threadId ) {
        (void) os_threadWaitExit(_this->threadId, &thread_result);
    }
}

void
os_signalHandlerFree(
    void)
{
#if !defined INTEGRITY && !defined VXWORKS_RTP
    int i;
    os_ssize_t r;
    os_signalHandler _this = signalHandlerObj;

    if (isSignallingSafe(0) && _this) {
        if (_this->handleExceptions) {
            for (i=0; i<lengthof(exceptions); i++) {
                const int sig = exceptions[i];
                r = os_sigactionSet(sig, &old_signalHandler[sig], NULL);
                if (r<0) {
                    OS_REPORT(OS_ERROR,
                            "os_signalHandlerFree", 0,
                            "os_sigactionSet(%d, 0x%"PA_PRIxADDR", NULL) failed, result = %"PA_PRIdSIZE,
                            sig, (os_address)&old_signalHandler[sig], r);
                    assert(OS_FALSE);
                }
            }
        }
        os__signalHandlerThreadStop(_this);
        close(_this->pipeIn[0]);
        close(_this->pipeIn[1]);
        close(_this->pipeOut[0]);
        close(_this->pipeOut[1]);

        os__signalHandlerCallbackDeinit(&_this->callbackInfo);

        os_free(_this);
        signalHandlerObj = NULL;
    }
#endif
}


os_result
os_signalHandlerSetEnabled(
    os_uint enabled)
{
    if(enabled == 0){
        installSignalHandler = 0;
    } else {
        installSignalHandler = 1;
    }
    return os_resultSuccess;
}

static os_signalHandlerCallbackInfo*
os__signalHandlerGetCallbackInfo(void)
{
    os_signalHandler _this = signalHandlerObj;

    assert(_this);

    return &_this->callbackInfo;
}
