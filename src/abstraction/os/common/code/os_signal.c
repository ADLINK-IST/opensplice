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
#include <pthread.h>

os_sigset
os_sigsetNew (
    void
    )
{
    os_sigset s;

    os_sigsetEmpty (&s);
    return s;
}

void
os_sigsetEmpty (
    os_sigset *set
    )
{
    sigemptyset (set);
}

os_result
os_sigsetFill (
    os_sigset *set
    )
{
    os_result result;
    os_int32 r;

    r = sigfillset (set);
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
}

os_int32
os_sigsetAdd (
    os_sigset *set,
    os_signal signal
    )
{
    return sigaddset (set, signal);
}

os_int32
os_sigsetDel (
    os_sigset *set,
    os_signal signal
    )
{
    return sigdelset (set, signal);
}

os_int32
os_sigsetIsMember (
    os_sigset *set,
    os_signal signal
    )
{
    return sigismember (set, signal);
}

os_sigaction
os_sigactionNew (
    os_actionHandler handler,
    os_sigset *mask,
    os_int32 flags
    )
{
    os_sigaction action;

    action.sa_handler = handler;
    action.sa_flags = flags;
    action.sa_mask = *mask;

    return action;
}

os_actionHandler
os_sigactionGetHandler (
    os_sigaction *sigaction
    )
{
    return sigaction->sa_handler;
}

os_sigset
os_sigactionGetMask (
    os_sigaction *sigaction
    )
{
    return sigaction->sa_mask;
}

os_int32
os_sigactionGetFlags (
    os_sigaction *sigaction
    )
{
    return sigaction->sa_flags;
}

void
os_sigactionSetHandler (
    os_sigaction *sigaction,
    os_actionHandler handler
    )
{
    sigaction->sa_handler = handler;
}

void
os_sigactionSetMask (
    os_sigaction *sigaction,
    os_sigset *mask
    )
{
    sigaction->sa_mask = *mask;
}

void
os_sigactionSetFlags (
    os_sigaction *sigaction,
    os_int32 flags
    )
{
    sigaction->sa_flags = flags;
}

os_int32
os_sigactionSet (
    os_signal signal,
    os_sigaction *nsigaction,
    os_sigaction *osigaction
    )
{
    return sigaction (signal, nsigaction, osigaction);
}

void
os_sigProcSetMask (
    os_sigset *mask,
    os_sigset *omask
    )
{
    sigprocmask (SIG_SETMASK, mask, omask);
}

os_result
os_sigThreadSetMask (
    os_sigset *mask,
    os_sigset *omask
    )
{
    os_result result;
    os_int32 r;

    r = pthread_sigmask (SIG_SETMASK, mask, omask);
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
}
