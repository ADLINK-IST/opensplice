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

#ifndef D__CONFLICT_RESOLVER_H
#define D__CONFLICT_RESOLVER_H

#include "d__types.h"
#include "d__conflict.h"
#include "d__admin.h"
#include "d__lock.h"
#include "os_time.h"
#include "os_mutex.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_conflictResolver validity.
 * Because d_conflictResolver is a concrete class typechecking is required.
 */
#define             d_conflictResolverIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_CONFLICT_RESOLVER)

/**
 * \brief The <code>d_conflictResolver</code> cast macro.
 *
 * This macro casts an object to a <code>d_conflictResolver</code> object.
 */
#define d_conflictResolver(_this) ((d_conflictResolver)(_this))

C_STRUCT(d_conflictResolver) {
    C_EXTENDS(d_lock);
    d_admin admin;
    os_threadId actionThread;
    os_timeM lastTimeResolved;
    os_timeM lastTimeChecked;
    c_ulong totalConflicts;
    c_bool threadTerminate;
    c_ulong conflictInProgress;  /* id of the conflict that is currently being resolved, 0 if none */
    d_conflict current_conflict; /* the conflict that is currently handled, NULL if no conflict */
    os_timeM nextReportTime;     /* next time to report progress of the initial conflict */
};


d_conflictResolver   d_conflictResolverNew                     (d_admin admin);

void                 d_conflictResolverDeinit                  (d_object object);

void                 d_conflictResolverFree                    (d_conflictResolver conflictResolver);

void                 d_conflictResolverAddConflict             (d_conflictResolver conflictResolver,
                                                                d_conflict conflict);

c_bool               d_conflictResolverConflictExists          (d_conflictResolver conflictResolver,
                                                                d_conflict conflict);

void                 d_conflictResolverSetConflictInProgress   (d_conflictResolver conflictResolver,
                                                                d_conflict conflict);

void                 d_conflictResolverResetConflictInProgress (d_conflictResolver conflictResolver,
                                                                d_conflict conflict);

c_bool               d_conflictResolverHasConflictInProgress   (d_conflictResolver conflictResolver);


#if defined (__cplusplus)
}
#endif

#endif /* D__CONFLICT_RESOLVER_H */
