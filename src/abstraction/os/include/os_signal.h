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
#ifndef OS_SIGNAL_H
#define OS_SIGNAL_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_CORE
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

OS_API os_result
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

typedef os_os_actionHandler os_actionHandler; 

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

OS_API os_result
os_sigThreadSetMask(
    os_sigset *mask,
    os_sigset *omask);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_SIGNAL_H */
