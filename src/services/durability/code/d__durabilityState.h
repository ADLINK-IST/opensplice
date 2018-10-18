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

#ifndef D__DURABILITY_STATE_H
#define D__DURABILITY_STATE_H

#include "d__types.h"
#include "d__admin.h"
#include "d__durabilityStateRequest.h"
#include "client_durabilitySplType.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_durabilityState validity.
 * Because d_durabilityState is a concrete class typechecking is required.
 */
#define             d_durabilityStateIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_DURABILITY_STATE)


/**
 * \brief The d_durabilityState cast macro.
 *
 * This macro casts an object to a d_durabilityState object.
 */
#define d_durabilityState(s) ((d_durabilityState)(s))


C_STRUCT(d_durabilityState) {
    C_EXTENDS(d_object);
    struct _DDS_DurabilityVersion_t version;
    struct _DDS_Gid_t serverId;
    c_iter requestIds;
    c_iter dataState;
    c_iter extensions;
    d_durabilityStateRequest request;    /* The request to respond to */
};


d_durabilityState                d_durabilityStateNew                     (d_admin admin);

void                             d_durabilityStateDeinit                  (d_durabilityState durabilityState);

void                             d_durabilityStateFree                    (d_durabilityState durabilityState);

#if defined (__cplusplus)
}
#endif

#endif /* D__DURABILITY_STATE_H */
