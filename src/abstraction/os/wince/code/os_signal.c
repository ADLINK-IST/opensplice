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
/** \file os/win32/code/os_signal.c
 *  \brief Signal Handling
 *
 * Implements signal handling
 */

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
}

os_result
os_sigsetFill(os_sigset *set)
{
    return os_resultSuccess;
}

os_int32
os_sigsetAdd(
    os_sigset *set,
    os_signal signal)
{
    return 0;
}

os_int32
os_sigsetDel(
    os_sigset *set,
    os_signal signal)
{
    return 0;
}

os_int32
os_sigsetIsMember(
    os_sigset *set,
    os_signal signal)
{
    return 0;
}

os_sigaction
os_sigactionNew(
    os_actionHandler handler,
    os_sigset *mask,
    os_int32 flags)
{
    return 1;
}

os_actionHandler
os_sigactionGetHandler(
    os_sigaction *sigaction)
{
    return 0;
}

os_sigset
os_sigactionGetMask(
    os_sigaction *sigaction)
{
    return 0;
}

os_int32
os_sigactionGetFlags(
    os_sigaction *sigaction)
{
    return 0;
}

void
os_sigactionSetHandler(
    os_sigaction *sigaction,
    os_actionHandler handler)
{
}

void
os_sigactionSetMask(
    os_sigaction *sigaction,
    os_sigset *mask)
{
}

void
os_sigactionSetFlags(
    os_sigaction *sigaction,
    os_int32 flags)
{
}

os_int32
os_sigactionSet(
    os_signal ossignal,
    os_sigaction *nsigaction,
    os_sigaction *osigaction)
{
    return 0;
}

void
os_sigProcSetMask(
    os_sigset *mask,
    os_sigset *omask)
{
}

os_result
os_sigThreadSetMask(
    os_sigset *mask,
    os_sigset *omask)
{
    return os_resultSuccess;
}
