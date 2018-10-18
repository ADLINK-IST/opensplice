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

#ifndef D__LISTENER_H
#define D__LISTENER_H

#include "d__types.h"
#include "d__lock.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define USE_WAITSET 1

/**
 * Macro that checks the d_listener validity.
 * Because d_listener is an abstract class typechecking can
 * only be done on the abstract listener object.
 */
#define             d_listenerIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_LISTENER)

/**
 * \brief The d_listener cast macro.
 *
 * This macro casts an object to a d_listener object.
 */
#define d_listener(_this) ((d_listener)(_this))

typedef void (*d_listenerAction)(d_listener listener, d_message message);

typedef enum d_listenerKind {
    D_STATUS_LISTENER,
    D_GROUP_REMOTE_LISTENER,
    D_GROUP_LOCAL_LISTENER, 
    D_GROUPS_REQ_LISTENER,
    D_SAMPLE_REQ_LISTENER,
    D_SAMPLE_CHAIN_LISTENER,
    D_NAMESPACES_REQ_LISTENER,
    D_NAMESPACES_LISTENER,
    D_PERSISTENT_DATA_LISTENER,
    D_DELETE_DATA_LISTENER,
    D_REMOTE_READER_LISTENER,
    D_DCPS_HEARTBEAT_LISTENER,
    D_HISTORICAL_DATA_REQ_LISTENER,
    D_DURABILITY_STATE_REQ_LISTENER,
    D_DCPS_PUBLICATION_LISTENER,
    D_CAPABILITY_LISTENER
} d_listenerKind;


C_STRUCT(d_listener) {
    C_EXTENDS(d_lock);
    d_listenerKind      kind;
    d_admin             admin;
    d_listenerAction    action;
    c_bool              attached;
};

void                d_listenerInit          (d_listener listener,
                                             d_listenerKind kind,
                                             d_subscriber subscriber,
                                             d_listenerAction action,
                                             d_objectDeinitFunc deinit);

c_bool              d_listenerIsValidKind   (d_listener listener,
                                             d_listenerKind kind);

void                d_listenerDeinit        (d_listener listener);

d_listenerAction    d_listenerGetAction     (d_listener listener);

c_bool              d_listenerIsAttached    (d_listener listener);

d_admin             d_listenerGetAdmin      (d_listener listener);

void                d_listenerLock          (d_listener listener);

void                d_listenerUnlock        (d_listener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__LISTENER_H */
