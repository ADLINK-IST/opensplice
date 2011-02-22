/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef OS_SIGNAL_H
#define OS_SIGNAL_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Include OS specific header file              */
#include "include/os_signal.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef os_os_sigset	os_sigset;

typedef os_os_sigaction	os_sigaction;

typedef os_os_signal	os_signal;

OS_API os_sigset
os_sigsetNew(void);

OS_API void
os_sigsetEmpty(
    os_sigset *set);

OS_API void
os_sigsetFill(
    os_sigset *set);

OS_API os_int32
os_sigsetAdd(
    os_sigset *set,
    os_signal signal);

OS_API os_int32
os_sigsetDel(
    os_sigset *set,
    os_signal signal);

OS_API os_int32
os_sigsetIsMember(
    os_sigset *set,
    os_signal signal);

typedef void (*os_actionHandler)();

OS_API os_sigaction
os_sigactionNew(
    os_actionHandler handler,
    os_sigset *mask,
    os_int32 flags);

OS_API os_actionHandler
os_sigactionGetHandler(
    os_sigaction *sigaction);

OS_API os_sigset
os_sigactionGetMask(
    os_sigaction *sigaction);

OS_API os_int32
os_sigactionGetFlags(
    os_sigaction *sigaction);

OS_API void
os_sigactionSetHandler(
    os_sigaction *sigaction,
    os_actionHandler handler);

OS_API void
os_sigactionSetMask(
    os_sigaction *sigaction,
    os_sigset *mask);

OS_API void
os_sigactionSetFlags(
    os_sigaction *sigaction,
    os_int32 flags);

OS_API os_int32
os_sigactionSet(
    os_signal signal,
    os_sigaction *sigaction,
    os_sigaction *osigaction);

OS_API void
os_sigProcSetMask(
    os_sigset *mask,
    os_sigset *omask);

OS_API void
os_sigThreadSetMask(
    os_sigset *mask,
    os_sigset *omask);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_SIGNAL_H */
