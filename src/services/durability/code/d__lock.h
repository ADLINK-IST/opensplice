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

#ifndef D__LOCK_H
#define D__LOCK_H

#include "d__types.h"
#include "os_mutex.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_lock validity.
 * Because the d_lock is an abstract class it is checked
 * if the pointer is non-NULL.
 */
#define d_lockIsValid(_this)  (_this != NULL)

/**
 * \brief The d_lock cast macro.
 *
 * This macro casts an object to a d_lock object.
 */
#define d_lock(d)    ((d_lock)(d))

C_STRUCT(d_lock) {
    C_EXTENDS(d_object);
    d_objectDeinitFunc deinit;
    os_mutex lock;
#ifndef NDEBUG
    os_ulong_int tid;
#endif
};


void    d_lockInit                 (d_lock lock,
                                    d_kind kind,
                                    d_objectDeinitFunc deinit);

void    d_lockDeinit               (d_lock lock);

void    d_lockFree                 (d_lock lock);

void    d_lockLock                 (d_lock lock);

void    d_lockUnlock               (d_lock lock);

#if defined (__cplusplus)
}
#endif

#endif /* D__LOCK_H */
