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
/** \file os/win32/code/os_signal.c
 *  \brief Signal Handling
 *
 * Implements signal handling
 */

#include <signal.h>

#include "os_defs.h"
#include "os_signal.h"

#include <stdio.h>

os_sigset
os_sigsetNew(void)
{
    os_sigset s;

    os_sigsetEmpty (&s);
    return s;
}

void
os_sigsetEmpty(os_sigset *set)
{
    OS_UNUSED_ARG(set);
}

os_result
os_sigsetFill(os_sigset *set)
{
    OS_UNUSED_ARG(set);

    return os_resultSuccess;
}

os_int32
os_sigsetAdd(
    os_sigset *set,
    os_signal signal)
{
    OS_UNUSED_ARG(set);
    OS_UNUSED_ARG(signal);

    return 0;
}

os_int32
os_sigsetDel(
    os_sigset *set,
    os_signal signal)
{
    OS_UNUSED_ARG(set);
    OS_UNUSED_ARG(signal);

    return 0;
}

os_int32
os_sigsetIsMember(
    os_sigset *set,
    os_signal signal)
{
    OS_UNUSED_ARG(set);
    OS_UNUSED_ARG(signal);

    return 0;
}

os_sigaction
os_sigactionNew(
    os_actionHandler handler,
    os_sigset *mask,
    os_int32 flags)
{
    OS_UNUSED_ARG(handler);
    OS_UNUSED_ARG(mask);
    OS_UNUSED_ARG(flags);

    return 1;
}

os_actionHandler
os_sigactionGetHandler(
    os_sigaction *sigaction)
{
    OS_UNUSED_ARG(sigaction);

    return 0;
}

os_sigset
os_sigactionGetMask(
    os_sigaction *sigaction)
{
    OS_UNUSED_ARG(sigaction);

    return 0;
}

os_int32
os_sigactionGetFlags(
    os_sigaction *sigaction)
{
    OS_UNUSED_ARG(sigaction);

    return 0;
}

void
os_sigactionSetHandler(
    os_sigaction *sigaction,
    os_actionHandler handler)
{
    OS_UNUSED_ARG(sigaction);
    OS_UNUSED_ARG(handler);
}

void
os_sigactionSetMask(
    os_sigaction *sigaction,
    os_sigset *mask)
{
    OS_UNUSED_ARG(sigaction);
    OS_UNUSED_ARG(mask);
}

void
os_sigactionSetFlags(
    os_sigaction *sigaction,
    os_int32 flags)
{
    OS_UNUSED_ARG(sigaction);
    OS_UNUSED_ARG(flags);
}

os_int32
os_sigactionSet(
    os_signal ossignal,
    os_sigaction *nsigaction,
    os_sigaction *osigaction)
{
    OS_UNUSED_ARG(ossignal);
    OS_UNUSED_ARG(nsigaction);
    OS_UNUSED_ARG(osigaction);

    return 0;
}

void
os_sigProcSetMask(
    os_sigset *mask,
    os_sigset *omask)
{
    OS_UNUSED_ARG(mask);
    OS_UNUSED_ARG(omask);
}

os_result
os_sigThreadSetMask(
    os_sigset *mask,
    os_sigset *omask)
{
    OS_UNUSED_ARG(mask);
    OS_UNUSED_ARG(omask);

    return os_resultSuccess;
}
