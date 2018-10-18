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

#ifndef D__CONFLICT_H
#define D__CONFLICT_H

#include "d__types.h"
#include "d__lock.h"
#include "d_networkAddress.h"
#include "os_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define D_CONFLICT_NAMESPACE_MASTER              (0x0001U << 0)
#define D_CONFLICT_NAMESPACE_NATIVE_STATE        (0x0001U << 1)
#define D_CONFLICT_NAMESPACE_FOREIGN_STATE       (0x0001U << 2)
#define D_CONFLICT_FELLOW_DISCONNECTED           (0x0001U << 4)
#define D_CONFLICT_FELLOW_CONNECTED              (0x0001U << 5)
#define D_CONFLICT_FEDERATION_DISCONNECTED       (0x0001U << 6)
#define D_CONFLICT_INITIAL                       (0x0001U << 7)
#define D_CONFLICT_LOCAL_GROUP_COMPLETE          (0x0001U << 8)
#define D_CONFLICT_HEARTBEAT_PROCESSED           (0x0001U << 9)

#define IS_MASTER_CONFLICT(conflict)                   ((conflict->event == D_CONFLICT_NAMESPACE_MASTER))
#define IS_NATIVE_STATE_CONFLICT(conflict)             ((conflict->event == D_CONFLICT_NAMESPACE_NATIVE_STATE))
#define IS_FOREIGN_STATE_CONFLICT(conflict)            ((conflict->event == D_CONFLICT_NAMESPACE_FOREIGN_STATE))
#define IS_FELLOW_DISCONNECTED_CONFLICT(conflict)      ((conflict->event == D_CONFLICT_FELLOW_DISCONNECTED))
#define IS_FELLOW_CONNECTED_CONFLICT(conflict)         ((conflict->event == D_CONFLICT_FELLOW_CONNECTED))
#define IS_FEDERATION_DISCONNECTED_CONFLICT(conflict)  ((conflict->event == D_CONFLICT_FEDERATION_DISCONNECTED))
#define IS_INITIAL_CONFLICT(conflict)                  ((conflict->event == D_CONFLICT_INITIAL))
#define IS_LOCAL_GROUP_COMPLETE_CONFLICT(conflict)     ((conflict->event == D_CONFLICT_LOCAL_GROUP_COMPLETE))
#define IS_HEARTBEAT_PROCESSED_CONFLICT(conflict)      ((conflict->event == D_CONFLICT_HEARTBEAT_PROCESSED))

#define IS_NAMESPACE_CONFLICT(conflict)          (IS_MASTER_CONFLICT(conflict) || IS_NATIVE_STATE_CONFLICT(conflict) ||  IS_FOREIGN_STATE_CONFLICT(conflict))

/**
 * Macro that checks the d_conflict validity.
 * Because d_conflict is a concrete class typechecking is required.
 */
#define             d_conflictIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_CONFLICT)

/**
 * \brief The <code>d_conflict</code> cast macro.
 *
 * This macro casts an object to a <code>d_conflict</code> object.
 */
#define d_conflict(_this) ((d_conflict)(_this))

C_STRUCT(d_conflict) {
    C_EXTENDS(d_lock);
    c_ulong event;                    /* conflict event */
    c_ulong id;                       /* unique conflict id */
    c_ulong nr;                       /* number of actions to solve as part of this conflict */
    c_ulong reQueueCount;             /* indicates the number of times the conflict is queued */
    os_timeM creationTime;            /* time of conflict creation */
    os_timeM lastQueueTime;           /* last time when added to the conflict resolver queue */
    d_networkAddress fellowAddr;      /* address of the fellow in conflict */
    d_nameSpace nameSpaceCopy;        /* copy of the namespace used when the conflict was determined */
    d_nameSpace fellowNameSpaceCopy;  /* copy of the fellow namespace used when the conflict was determined */
    c_iter foreign_stateConflicts;    /* List of foreign state conflicts between nameSpaceCopy and fellowNameSpaceCopy */
};

d_conflict         d_conflictNew                           (c_ulong conflictEvent,
                                                            d_networkAddress fellowAddr,
                                                            d_nameSpace nameSpaceCopy,
                                                            d_nameSpace fellowNameSpaceCopy);

void               d_conflictDeinit                        (d_object object);

void               d_conflictFree                          (d_conflict conflict);

c_ulong            d_conflictGetId                         (d_conflict conflict);

c_ulong            d_conflictSetId                         (d_conflict conflict,
                                                            d_durability durability);

os_timeM           d_conflictGetCreationTime               (d_conflict conflict);

void               d_conflictUpdate                        (d_conflict conflict,
                                                            d_nameSpace nameSpaceCopy,
                                                            d_nameSpace fellowNameSpaceCopy);

void               d_conflictUpdateQueueTime               (d_conflict conflict);

#if defined (__cplusplus)
}
#endif

#endif /* D__CONFLICT_H */
