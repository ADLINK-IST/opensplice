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

#ifndef D__PUBLISHER_H
#define D__PUBLISHER_H

#include "d__types.h"
#include "u_user.h"
#include "os_mutex.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_publisher validity.
 * Because d_publisher is a concrete class typechecking is required.
 */
#define             d_publisherIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_PUBLISHER)

/**
 * \brief The d_publisher cast macro.
 *
 * This macro casts an object to a d_publisher object.
 */
#define d_publisher(_this) ((d_publisher)(_this))

C_STRUCT(d_publisher){
    C_EXTENDS(d_object);
    d_admin         admin;
    u_publisher     publisher;     /* publisher for durability protocol topics */
    u_publisher     publisher2;    /* publisher for client durability */
    c_bool          enabled;
    c_bool          capabilitySupport;
    u_writer        groupsRequestWriter;
    c_ulong         groupsRequestNumber;
    u_writer        sampleRequestWriter;
    c_ulong         sampleRequestNumber;
    u_writer        statusWriter;
    c_ulong         statusNumber;
    u_writer        newGroupWriter;
    c_ulong         newGroupNumber;
    u_writer        sampleChainWriter;
    c_ulong         sampleChainNumber;
    u_writer        nameSpacesRequestWriter;
    c_ulong         nameSpacesRequestNumber;
    u_writer        nameSpacesWriter;
    c_ulong         nameSpacesNumber;
    u_writer        deleteDataWriter;
    c_ulong         deleteDataNumber;
    u_writer        durabilityStateWriter;
    c_ulong         durabilityStateNumber;
    c_ulong         historicalDataNumber;
    u_writer        capabilityWriter;
    c_ulong         capabilityNumber;
};

v_copyin_result d_publisherMessageWriterCopy            (d_message msgFrom,
                                                         d_message msgTo);

v_copyin_result d_publisherStatusWriterCopy             (c_type type,
                                                         const void *data,
                                                         void *to);

v_copyin_result d_publisherNewGroupWriterCopy           (c_type type,
                                                         const void *data,
                                                         void *to);

v_copyin_result d_publisherGroupsRequestWriterCopy      (c_type type,
                                                         const void *data,
                                                         void *to);


v_copyin_result d_publisherSampleRequestWriterCopy      (c_type type,
                                                         const void *data,
                                                         void *to);

v_copyin_result d_publisherSampleChainWriterCopy        (c_type type,
                                                         const void *data,
                                                         void *to);

v_copyin_result d_publisherNameSpacesWriterCopy         (c_type type,
                                                         const void *data,
                                                         void *to);

v_copyin_result d_publisherNameSpacesRequestWriterCopy  (c_type type,
                                                         const void *data,
                                                         void *to);

v_copyin_result d_publisherDeleteDataWriterCopy         (c_type type,
                                                         const void *data,
                                                         void *to);

v_copyin_result d_publisherCapabilityWriterCopy         (c_type type,
                                                         const void *data,
                                                         void *to);

void            d_publisherInitMessage                  (d_publisher publisher,
                                                         d_message message);

void            d_publisherEnsureServicesAttached       (v_public entity,
                                                         c_voidp args);

void            d_publisherDeinit                       (d_publisher publisher);

d_publisher     d_publisherNew                          (d_admin admin);

void            d_publisherFree                         (d_publisher publisher);

c_bool          d_publisherStatusWrite                  (d_publisher publisher,
                                                         d_status message,
                                                         d_networkAddress addressee);

c_bool          d_publisherStatusResend                 (d_publisher publisher,
                                                         d_status message);

c_bool          d_publisherNewGroupWrite                (d_publisher publisher,
                                                         d_newGroup message,
                                                         d_networkAddress addressee);

c_bool          d_publisherGroupsRequestWrite           (d_publisher publisher,
                                                         d_groupsRequest message,
                                                         d_networkAddress addressee);

c_bool          d_publisherSampleRequestWrite           (d_publisher publisher,
                                                         d_sampleRequest message,
                                                         d_networkAddress addressee);

c_bool          d_publisherSampleChainWrite             (d_publisher publisher,
                                                         d_sampleChain message,
                                                         d_networkAddress addressee);

c_bool          d_publisherNameSpacesRequestWrite       (d_publisher publisher,
                                                         d_nameSpacesRequest message,
                                                         d_networkAddress addressee,
                                                         d_serviceState state);

c_bool          d_publisherNameSpacesWrite              (d_publisher publisher,
                                                         d_nameSpaces message,
                                                         d_networkAddress addressee);

c_bool          d_publisherDeleteDataWrite              (d_publisher publisher,
                                                         d_deleteData message,
                                                         d_networkAddress addressee);

c_bool          d_publisherCapabilityWrite              (d_publisher publisher,
                                                         d_capability message,
                                                         d_networkAddress addressee);

c_bool          d_publisherDurabilityStateWrite         (d_publisher publisher,
                                                         d_durabilityState durabilityState);

c_bool          d_publisherUnregisterInstances          (d_publisher publisher,
                                                         d_networkAddress addressee);

#if defined (__cplusplus)
}
#endif

#endif /* D__PUBLISHER_H */
