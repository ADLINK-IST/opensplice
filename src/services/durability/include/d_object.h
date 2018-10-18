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

#ifndef D_OBJECT_H
#define D_OBJECT_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef enum d_kind{
    D_BAD_TYPE,          /* Must be the first element */
    D_DURABILITY,
    D_CONFIGURATION,
    D_FELLOW,
    D_ADMIN,
    D_GROUP,
    D_LISTENER,
    D_POLICY,
    D_NAMESPACE,
    D_PUBLISHER,
    D_SUBSCRIBER,
    D_TABLE,
    D_CHAIN,
    D_EVENT_LISTENER,
    D_ADMIN_EVENT,
    D_STORE,
    D_WAITSET,
    D_WAITSET_ENTITY,
    D_ACTION,
    D_ACTION_QUEUE,
    D_GROUP_CREATION_QUEUE,
    D_READER_REQUEST,
    D_MERGE_ACTION,
    D_ALIGNER_STATS,
    D_ALIGNEE_STATS,
    D_ADMIN_STATS_INFO,
    D_CONFLICT,
    D_CONFLICT_MONITOR,
    D_CONFLICT_RESOLVER,
    D_HISTORICAL_DATA_REQ,
    D_HISTORICAL_DATA,
    D_DURABILITY_STATE_REQ,
    D_DURABILITY_STATE,
    D_PART_TOPIC_STATE,
    D_FILTER,
    D_CLIENT,
    D_KINDCOUNT         /* Must be the last element! */
} d_kind;

C_CLASS(d_object);

typedef void (*d_objectDeinitFunc) (d_object object);

C_STRUCT(d_object){
    c_ulong confidence;
    d_kind kind;
    pa_uint32_t refCount;
    d_objectDeinitFunc deinit;
};

/**
 * \brief The <code>d_object</code> cast macro.
 *
 * This macro casts an object to a <code>d_object</code> object.
 */
#define d_object(d) ((d_object)(d))

void        d_objectInit        (d_object object,
                                 d_kind kind,
                                 d_objectDeinitFunc deinit);

void        d_objectDeinit      (d_object object);

void        d_objectFree        (d_object object);

c_bool      d_objectIsValid     (d_object object,
                                 d_kind kind);

d_object    d_objectKeep        (d_object object);

c_bool      d_objectValidate    (c_ulong expected,
                                 int enable_allocation_report);

#if defined (__cplusplus)
}
#endif

#endif /* D_OBJECT_H */
