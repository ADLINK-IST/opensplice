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

#ifndef D__DURABILITY_STATE_REQUEST_H
#define D__DURABILITY_STATE_REQUEST_H

#include "d__types.h"
#include "d__admin.h"
#include "client_durabilitySplType.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_durabilityStateRequest validity.
 * Because d_durabilityStateRequest is a concrete class typechecking is required.
 */
#define             d_durabilityStateRequestIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_DURABILITY_STATE_REQ)


/**
 * \brief The d_durabilityStateRequest cast macro.
 *
 * This macro casts an object to a d_durabilityStateRequest object.
 */
#define d_durabilityStateRequest(s) ((d_durabilityStateRequest)(s))


C_STRUCT(d_durabilityStateRequest) {
    C_EXTENDS(d_object);
    struct _DDS_DurabilityStateRequest *request;
    struct _DDS_DurabilityVersion_t version;
    struct _DDS_RequestId_t requestId;
    char *topic;
    c_iter partitions;
    c_iter serverIds;
    c_time timeout;
    c_iter extensions;
    c_bool forMe;
    c_bool forEverybody;
};


d_durabilityStateRequest                d_durabilityStateRequestNew         (d_admin admin,
                                                                             struct _DDS_DurabilityVersion_t version,
                                                                             struct _DDS_RequestId_t requestId,
                                                                             d_topic topic,
                                                                             c_iter partitions,
                                                                             c_iter serverIds,
                                                                             c_time timeout,
                                                                             c_iter extensions);

void                                    d_durabilityStateRequestDeinit      (d_durabilityStateRequest durabilityStateRequest);

void                                    d_durabilityStateRequestFree        (d_durabilityStateRequest durabilityStateRequest);

c_ulong                                 d_durabilityStateRequestSanityCheck (d_durabilityStateRequest durabilityStateRequest);

#if defined (__cplusplus)
}
#endif

#endif /* D__DURABILITY_STATE_REQUEST_H */
