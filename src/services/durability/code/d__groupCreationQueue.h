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
#ifndef D__GROUPCREATIONQUEUE_H
#define D__GROUPCREATIONQUEUE_H

#include "d__types.h"
#include "d__lock.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_groupCreationQueue validity.
 * Because d_groupCreationQueue is a concrete class typechecking is required.
 */
#define             d_groupCreationQueueIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_GROUP_CREATION_QUEUE)

/**
 * \brief The d_groupCreationQueue cast macro.
 *
 * This macro casts an object to a d_groupCreationQueue object.
 */
#define d_groupCreationQueue(_this) ((d_groupCreationQueue)(_this))

C_STRUCT(d_groupCreationQueue) {
    C_EXTENDS(d_lock);
    os_threadId actionThread;
    c_bool terminate;
    c_iter groups;
    d_admin admin;
    c_ulong groupsToCreateVolatile;
    c_ulong groupsToCreateTransient;
    c_ulong groupsToCreatePersistent;
};

d_groupCreationQueue    d_groupCreationQueueNew     (d_admin admin);

void                    d_groupCreationQueueDeinit  (d_groupCreationQueue queue);

void                    d_groupCreationQueueFree    (d_groupCreationQueue queue);

c_bool                  d_groupCreationQueueAdd     (d_groupCreationQueue queue,
                                                     d_group group);

c_bool                  d_groupCreationQueueIsEmpty (d_groupCreationQueue queue);

#if defined (__cplusplus)
}
#endif

#endif /* D__GROUPCREATIONQUEUE_H */
