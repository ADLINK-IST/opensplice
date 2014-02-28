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

#include "os_signalHandler.h"

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
#include <errno.h>
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



#define SIGNAL_THREAD_STOP (-1)
#define lengthof(a) ((int)(sizeof(a) / sizeof(*a)))

/* These are the signals for which the User shall detach from the Domain.
 * For these signals a handler is set to unregister the user from the domain.
 */
static const int exceptions[] = {
    SIGILL, SIGTRAP, SIGABRT, SIGFPE, SIGBUS, SIGSEGV, SIGSYS
};

static const int quits[] = {
    SIGINT, SIGQUIT, SIGTERM, SIGHUP, SIGPIPE
};

static sigset_t exceptionsMask;
static sigset_t quitsMask;

struct os_signalHandler_s {
    os_threadId threadId;   /* The thread id of the signalHandlerThread. */
    int pipeIn[2]; /* a pipe to send a signal to the signal handler thread. */
    int pipeOut[2]; /* a pipe to receive a result from the signal handler thread. */
    os_signalHandlerExitRequestCallback exitRequestCallback;
    os_signalHandlerExceptionCallback exceptionCallback;
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


struct sig_context {
    siginfo_t info;
#ifdef OS_HAS_UCONTEXT_T
    ucontext_t uc;
#endif
};

/* private functions */
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
    write (STDERR_FILENO, panicmsg, count);
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
        if (r == -1 && errno != EINTR){
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
            if (r == -1 && errno != EINTR){
                /* We understand that EINTR may have occurred, the rest of the
                 * possible errors are reason for serious panic; the only means we
                 * had of handling this cleanly is this piece of code :s, so panic
                 * it is. */
                const char panicmsg[] = "FATAL ERROR: Read from pipe in signalHandlerThreadNotify failed\n";
                panic(panicmsg, sizeof(panicmsg) - 1);
                /* This line will not be reached anymore */
            }
            if (r > 0) {
                nread += r;
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

    sigInfo.info = *info;
#ifdef OS_HAS_UCONTEXT_T
    sigInfo.uc = *(ucontext_t *)uap;
#endif

    /* WARNING: Don't do any async/signalling-unsafe calls here (so refrain from
     * using OS_REPORT_X and the like). */
    if (sigismember(&exceptionsMask, sig) == 1 && sigInfo.info.si_code != SI_USER){
        os_sigaction *xo;
        /* We have an exception (synchronous) here. The assumption is
         * that exception don't occur in middleware-code, so we can
         * synchronously call the signalHandlerThread in order to detach user-
         * layer from all Domains (kernels). */
        signalHandlerThreadNotify(sigInfo, &sync);
        /* BEWARE: This may be an interleaved handling of an exception, so use
         * sync from now on instead of sigInfo.*/

        /* Reset the original signal-handler. Since the exception was
         * synchronous, running out of this handler will regenerate the signal,
         * which will then be handled by the original signal-handler. */
        xo = &old_signalHandler[sync.info.si_signo];
        os_sigactionSet(sync.info.si_signo, xo, NULL);
    } else {
        /* Pass signal to signal-handler thread for asynchronous handling */
        signalHandlerThreadNotify(sigInfo, NULL);
    }
}

os_signalHandlerExitRequestCallback
os_signalHandlerSetExitRequestCallback(
    os_signalHandlerExitRequestCallback cb)
{
    os_signalHandlerExitRequestCallback oldCb;
    os_signalHandler _this = signalHandlerObj;

    oldCb = _this->exitRequestCallback;
    _this->exitRequestCallback = cb;
    return oldCb;
}

os_signalHandlerExceptionCallback
os_signalHandlerSetExceptionCallback(
    os_signalHandlerExceptionCallback cb)
{
    os_signalHandlerExceptionCallback oldCb;
    os_signalHandler _this = signalHandlerObj;

    oldCb = _this->exceptionCallback;
    _this->exceptionCallback = cb;
    return oldCb;
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
        OS_REPORT_2(OS_WARNING,
            "os_signalHandlerFinishExitRequest", 0,
            "Callback-arg contains invalid signal, value out of range 1-%d: arg = %d",
            OS_NSIG, sig);
        r = os_resultInvalid;
    } else if (sigismember(&quitsMask, sig) == 0){
#if OS_NSIG >= 100
#error "Worst-case allocation assumes max. signal of 99, which apparently is not correct"
#endif
        /* We know which signal-number exist, all take at most 2 digits + ", ",
         * so allocate worst-case 4 * lengthof(quits) */
        char *expected = os_malloc(lengthof(quits) * 4 + 1);
        if(expected){
            int i, pos;
            assert(lengthof(quits) > 0);
            pos = sprintf(expected, "%d", quits[0]);
            for(i = 1; i < lengthof(quits); i++){
                pos += sprintf(expected + pos, ", %d", quits[i]);
            }
        }
        OS_REPORT_2(OS_WARNING,
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
            r = os_resultFail;
            OS_REPORT_1(OS_WARNING,
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
        while(nread < sizeof(info) && ((r = read(_this->pipeIn[0], &info + nread, sizeof(info) - nread)) > 0)){
            nread += r;
        }
        if (r < 0) {
            int errorNr = errno;
            OS_REPORT_2(OS_ERROR,
                        "os_signalHandlerThread", 0,
                        "read(_this->pipeIn[0]) failed, error %d: %s",
                        errorNr, strerror(errorNr));
            assert(OS_FALSE);
            sig = SIGNAL_THREAD_STOP;
        } else {
            sig = info.info.si_signo;
        }
        if(sig != SIGNAL_THREAD_STOP){
            if (sig < 1 || sig >= OS_NSIG) {
                OS_REPORT_2(OS_WARNING,
                            "os_signalHandlerThread", 0,
                            "Unexpected signal, value out of range 1-%d: signal = %d",
                            OS_NSIG, sig);
            } else {
                if(sigismember(&exceptionsMask, sig) == 1){
                    if(info.info.si_code == SI_USER){
                        OS_REPORT_4(OS_INFO, "signalHandlerThread", 0,
                            "Synchronous exception (signal %d) asynchronously received from PID %d%s, UID %d",
                            sig,
                            info.info.si_pid,
                            info.info.si_pid == getpid() ? " (self)" : "",
                            info.info.si_uid);
                    } else {
                        OS_REPORT_1(OS_INFO, "signalHandlerThread", 0,
                            "Exception (signal %d) occurred in process",
                            sig);
                    }

                    /* If an exceptionCallback was set, this should always be invoked. The main
                     * goal is to protect SHM in case of an exception, even if a customer
                     * installed a handler as well. */
                    if (_this->exceptionCallback &&
                        (_this->exceptionCallback() != os_resultSuccess)) {
                        OS_REPORT(OS_ERROR,
                            "os_signalHandlerThread", 0,
                            "Exception-callback failed");
                    }

                    if(info.info.si_code == SI_USER){
                        /* Mimic an exception by re-raising it. A random thread received the signal
                         * asynchronously, so when we raise it again another random thread will
                         * receive the signal. */
                        os_sigaction *xo;
                        os_sigset ss;

                        /* Reset the original signal-handler. */
                        xo = &old_signalHandler[info.info.si_signo];
                        os_sigactionSet(info.info.si_signo, xo, NULL);

                        /* Since the exception was actually asynchronous (issued by an external
                         * source), the original signal-handler will not be called by running
                         * out of the handler. We chain by re-raising, since SIG_DFL or SIG_IGN
                         * cannot be called directly. */
                        /* OSPL-2762: Instead of re-raising we use kill to
                           make sure the signal isn't delivered to this thread
                           as that would cause a dead lock. */
                        pid = getpid();
                        /* Don't think it's possible to not send the signal */
                        OS_REPORT_2 (OS_DEBUG, "os_signalHandlerThread", 0,
                            "Invoking kill (signal %d) on PID %d (self)",
                            info.info.si_signo, pid);
                        (void)kill (pid, info.info.si_signo);
                    } else {
                        /* Notify waiting signal handler to unblock. */
                        r = write(_this->pipeOut[1], &info, sizeof(info));
                        if (r<0) {
                            int errorNr = errno;
                            OS_REPORT_2(OS_ERROR,
                                        "os_signalHandlerThread", 0,
                                        "write(_this->pipeOut[1]) failed, error %d: %s",
                                        errorNr, strerror(errorNr));
                            assert(OS_FALSE);
                        }
                    }
                } else if (sigismember(&quitsMask, sig) == 1){
                    os_callbackArg arg;
                    OS_REPORT_4(OS_INFO, "signalHandlerThread", 0,
                        "Termination request (signal %d) received from PID %d%s, UID %d",
                        sig,
                        (info.info.si_code == SI_USER) ? info.info.si_pid : getpid(),
                        (info.info.si_code != SI_USER  || info.info.si_pid == getpid()) ? " (self)" : "",
                        (info.info.si_code == SI_USER) ? info.info.si_uid : getuid());

                    arg.sig = (void*)(os_address)sig;

                    /* Quit-signals which were set to SIG_IGN shouldn't have our signal-handler
                     * set at all. */
                    assert(old_signalHandler[sig].sa_handler != SIG_IGN);
                    if(old_signalHandler[sig].sa_handler == SIG_DFL){
                        if (_this->exitRequestCallback &&
                            (_this->exitRequestCallback(arg) != os_resultSuccess)) {
                            OS_REPORT(OS_ERROR,
                                "os_signalHandlerThread", 0,
                                "Exit request-callback failed");
                        }
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
    os_signalHandler _this = NULL;
    os_result result = os_resultFail;

#if !defined INTEGRITY && !defined VXWORKS_RTP
    _this = (os_signalHandler)os_malloc(sizeof(struct os_signalHandler_s));

    if (_this != NULL) {
        signalHandlerObj = _this;
        result = os_signalHandlerInit(_this);
    } else {
        OS_REPORT_1(OS_ERROR, "os_signalHandlerNew", 0,
                    "os_malloc(%d) failed.",
                     sizeof(struct os_signalHandler_s));
    }
#endif
    return result;
}

static os_result
os_signalHandlerInit(
    os_signalHandler _this)
{
    os_result result = os_resultSuccess;
    os_sigaction action;
    os_sigset block_all_sigset, old_sigset;
    int i, r;

    if (_this == NULL) {
        result = os_resultFail;
    }
    _this->exitRequestCallback = (os_signalHandlerExitRequestCallback)0;
    _this->exceptionCallback = (os_signalHandlerExceptionCallback)0;


    if(isSignallingSafe(1)){
        /* Initialise the exceptionsMask */
        sigemptyset(&exceptionsMask);
        for(i = 0; i < lengthof(exceptions); i++){
            sigaddset(&exceptionsMask, exceptions[i]);
        }

        /* Initialise the quitsMask */
        sigemptyset(&quitsMask);
        for(i = 0; i < lengthof(quits); i++){
            sigaddset(&quitsMask, quits[i]);
        }

        /* create signal handling pipes */
        if (result == os_resultSuccess) {
            r = pipe(_this->pipeIn);
            if (r<0) {
                OS_REPORT_1(OS_ERROR,
                            "os_signalHandlerInit", 0,
                            "pipe(_this->pipeIn) failed, result = %d",
                            r);
                result = os_resultFail;
            } else {
                r = fcntl(_this->pipeIn[0], F_SETFD, 1);
                if (r<0) {
                    OS_REPORT_1(OS_WARNING,
                                "os_signalHandlerInit", 0,
                                "fcntl(_this->pipeIn[0]) failed, result = %d", r);
                    assert(OS_FALSE);
                }
                r = fcntl(_this->pipeIn[1], F_SETFD, 1);
                if (r<0) {
                    OS_REPORT_1(OS_WARNING,
                                "os_signalHandlerInit", 0,
                                "fcntl(_this->pipeIn[1]) failed, result = %d", r);
                    assert(OS_FALSE);
                }
            }
        }
        if (result == os_resultSuccess) {
            r = pipe(_this->pipeOut);
            if (r<0) {
                OS_REPORT_1(OS_ERROR,
                            "os_signalHandlerInit", 1,
                            "pipe(_this->pipeOut) failed, result = %d",
                            r);
                result = os_resultFail;
            } else {
                r = fcntl(_this->pipeOut[0], F_SETFD, 1);
                if (r<0) {
                    OS_REPORT_1(OS_WARNING,
                                "os_signalHandlerInit", 0,
                                "fcntl(_this->pipeOut[0]) failed, result = %d",
                                r);
                    assert(OS_FALSE);
                }
                r = fcntl(_this->pipeOut[1], F_SETFD, 1);
                if (r<0) {
                    OS_REPORT_1(OS_WARNING,
                                "os_signalHandlerInit", 0,
                                "fcntl(_this->pipeOut[1]) failed, result = %d",
                                r);
                    assert(OS_FALSE);
                }
            }
        }
        /* Block all signals */
        if (result == os_resultSuccess) {
            result = os_sigsetFill(&block_all_sigset);
            if (result != os_resultSuccess) {
                OS_REPORT_1(OS_ERROR,
                            "os_signalHandlerInit", 0,
                            "os_sigsetFill(&block_all_sigset) failed, result = %d",
                            r);
                assert(OS_FALSE);
            } else {
                result = os_sigThreadSetMask(&block_all_sigset, &old_sigset);
                if (result != os_resultSuccess) {
                    OS_REPORT_3(OS_ERROR,
                                "os_signalHandlerInit", 0,
                                "os_sigThreadSetMask(0x%x, 0x%x) failed, result = %d",
                                &block_all_sigset, &old_sigset, r);
                    assert(OS_FALSE);
                }
            }
        }
        /* Setup signal handler thread. */
        if (result == os_resultSuccess) {
            os_threadAttr thrAttr;

            result = os_threadAttrInit(&thrAttr);
            if (result != os_resultSuccess) {
                OS_REPORT_2(OS_ERROR,
                            "os_signalHandlerInit", 0,
                            "pthread_attr_init(0x%x) failed, result = %d",
                            &thrAttr, r);
                assert(OS_FALSE);
            } else {
                thrAttr.stackSize = 4*1024*1024; /* 4 MB */
                result = os_threadCreate(&_this->threadId,
                                         "signalHandler",
                                         &thrAttr,
                                         signalHandlerThread,
                                         (void*)_this);

                if (result != os_resultSuccess) {
                    OS_REPORT_4(OS_ERROR,
                                "os_signalHandlerInit", 0,
                                "os_threadCreate(0x%x, 0x%x,0x%x,0) failed, result = %d",
                                &_this->threadId,
                                &thrAttr,
                                signalHandlerThread,
                                result);
                    assert(OS_FALSE);
                }
            }
        }
        /* Reset signal mask to original value. */
        if (result == os_resultSuccess) {
            result = os_sigThreadSetMask(&old_sigset, NULL);
            if (result != os_resultSuccess) {
                OS_REPORT_2(OS_ERROR,
                            "os_signalHandlerInit", 0,
                            "os_sigThreadSetMask(0x%x, NULL) failed, result = %d",
                            &old_sigset, r);
                result = os_resultFail;
                assert(OS_FALSE);
            }
        }
        /* install signal handlers */
        if (result == os_resultSuccess) {
            os_sigset mask;
            /* block all signals during handling of a signal */
            result = os_sigsetFill(&mask);
            if (result != os_resultSuccess) {
                OS_REPORT_2(OS_ERROR,
                            "os_signalHandlerInit", 0,
                            "os_sigsetFill(0x%x) failed, result = %d",
                            &action.sa_mask, result);
            } else {
                action = os_sigactionNew(signalHandler, &mask, SA_SIGINFO);
            }
            if (result == os_resultSuccess) {
                for (i=0; i<lengthof(exceptions); i++) {
                    const int sig = exceptions[i];
                    r = os_sigsetDel(&action.sa_mask, sig);
                    if (r<0) {
                        OS_REPORT_3(OS_ERROR,
                                    "os_signalHandlerInit", 0,
                                    "os_sigsetDel(0x%x, %d) failed, result = %d",
                                    &action, sig, r);
                        result = os_resultFail;
                        assert(OS_FALSE);
                    }
                }
            }
            if (result == os_resultSuccess) {
                for (i=0; i<lengthof(exceptions); i++) {
                    const int sig = exceptions[i];
                    /* For exceptions we always set our own signal handler, since
                     * applications that cause an exception are not in a position
                     * to ignore it. However, we will chain the old handler to our
                     * own.
                     */
                    r = os_sigactionSet(sig, &action, &old_signalHandler[sig]);
                    if (r < 0) {
                        OS_REPORT_4(OS_ERROR,
                                    "os_signalHandlerInit", 0,
                                    "os_sigactionSet(%d, 0x%x, 0x%x) failed, result = %d",
                                    sig, &action, &old_signalHandler[sig], r);
                        result = os_resultFail;
                        assert(OS_FALSE);
                    }
                }
            }
            if (result == os_resultSuccess) {
                for (i=0; i<lengthof(quits); i++) {
                    const int sig = quits[i];
                    /* By passing NULL we only retrieve the currently set handler. If
                     * the signal should be ignored, we don't do anything. Otherwise we
                     * chain the old handler to our own.
                     * man-page of sigaction only states behaviour when new
                     * action is non-NULL, but all known implementations act as
                     * expected. That is: return the currently set signal-hand-
                     * ler (and not the previous, as the man-pages state).
                     * NOTE: Not MT-safe */
                    r = os_sigactionSet(sig, NULL, &old_signalHandler[sig]);
                    if (r == 0) {
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
                            r = os_sigactionSet(sig, &action, NULL);
                            if (r != 0) {
                                OS_REPORT_4(OS_ERROR,
                                            "os_signalHandlerInit", 0,
                                            "os_sigactionSet(%d, 0x%x, 0x%x) failed, result = %d",
                                            sig, &action, &old_signalHandler[sig], r);
                                result = os_resultFail;
                                assert(OS_FALSE);
                            }
                        } else {
                            OS_REPORT_1(OS_INFO,
                                       "os_signalHandlerThread", 0,
                                       "Not installing a signal handler for the already ignored signal %d.",
                                       sig);
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, "os_signalHandlerInit", 0, "Could not retrieve currently set signal-handler for signal %d", sig);
                        result = os_resultFail;
                        assert(OS_FALSE);
                    }
                }
            }
        }
    } else {
        result = os_resultSuccess;
    }
    return result;
}

os_result
os_signalHandlerSetHandler(
    os_signal signal,
    os_actionHandler handler)
{
    os_sigset mask;
    os_result result;
    os_sigaction action;
    int r;

    /* block all signals during handling of a signal */
    result = os_sigsetFill(&mask);
    if (result != os_resultSuccess)
    {
        OS_REPORT_2(OS_ERROR,
                    "os_signalHandlerInit", 0,
                    "os_sigsetFill(0x%x) failed, result = %d",
                    &action.sa_mask, result);
    } else
    {
        action = os_sigactionNew(handler, &mask, SA_SIGINFO);
    }
    if (result == os_resultSuccess)
    {
        r = os_sigsetDel(&action.sa_mask, signal);
        if (r < 0)
        {
            OS_REPORT_3(OS_ERROR,
                        "os_signalHandlerInit", 0,
                        "os_sigsetDel(0x%x, %d) failed, result = %d",
                        &action, signal, r);
            result = os_resultFail;
            assert(OS_FALSE);
        }
    }
    if (result == os_resultSuccess)
    {
        r = os_sigactionSet(signal, &action, &old_signalHandler[signal]);
        if (r < 0) {
            OS_REPORT_4(OS_ERROR,
                        "os_signalHandlerInit", 0,
                        "os_sigactionSet(%d, 0x%x, 0x%x) failed, result = %d",
                        signal, &action, &old_signalHandler[signal], r);
            result = os_resultFail;
            assert(OS_FALSE);
        }
    }
    return result;
}

void
os_signalHandlerFree(
    void)
{
#if !defined INTEGRITY && !defined VXWORKS_RTP
    void *thread_result;
    int i, r;
    os_signalHandler _this = signalHandlerObj;
    struct sig_context info;

    if (isSignallingSafe(0) && _this) {
        for (i=0; i<lengthof(exceptions); i++) {
            const int sig = exceptions[i];
            r = os_sigactionSet(sig, &old_signalHandler[sig], NULL);
            if (r<0) {
                OS_REPORT_3(OS_ERROR,
                            "os_signalHandlerFree", 0,
                            "os_sigactionSet(%d, 0x%x, NULL) failed, result = %d",
                            sig, &old_signalHandler[sig], r);
                assert(OS_FALSE);
            }
        }
        memset (&info, 0, sizeof(info));
        info.info.si_signo = SIGNAL_THREAD_STOP;
        r = write(_this->pipeIn[1], &info, sizeof(info));
        /* when signalhandler is the exiting thread itself, this can happen when an exit call is done in the signalhandler of the customer
         * do not call os_threadWaitExit but just let this thread run out of its main function */
        if (os_threadIdSelf() != _this->threadId ) {
            os_threadWaitExit(_this->threadId, &thread_result);
        }
        close(_this->pipeIn[0]);
        close(_this->pipeIn[1]);
        close(_this->pipeOut[0]);
        close(_this->pipeOut[1]);
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
