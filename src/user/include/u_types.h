/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef U_TYPES_H
#define U_TYPES_H

#include "os_heap.h"
#include "c_typebase.h"
#include "c_iterator.h"
#include "v_copyIn.h"
#include "v_kernel.h"
#include "v_event.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief The result values all methods of the classes within the
 * user component can return.
 */

typedef os_uchar     u_bool;
typedef os_address   u_size;

typedef v_result     u_result;
typedef v_eventMask  u_eventMask;
typedef v_sampleMask u_sampleMask;
typedef os_int32     u_domainId_t;

typedef v_waitsetEvent u_waitsetEvent;

typedef v_participantQos u_participantQos;
typedef v_partitionQos   u_partitionQos;
typedef v_topicQos       u_topicQos;
typedef v_publisherQos   u_publisherQos;
typedef v_subscriberQos  u_subscriberQos;
typedef v_writerQos      u_writerQos;
typedef v_readerQos      u_readerQos;
typedef v_dataViewQos    u_dataViewQos;

typedef v_typeHash       u_typeHash;
typedef v_dataRepresentationId_t u_dataRepresentationId_t;

C_STRUCT(u_typeRepresentation) {
    const os_char *typeName;
    u_dataRepresentationId_t dataRepresentationId;
    u_typeHash typeHash;
    const os_uchar *metaData;
    os_uint32 metaDataLen;
    const os_uchar *extentions;
    os_uint32 extentionsLen;
};
C_CLASS(u_typeRepresentation);

typedef struct v_participantInfo  u_participantInfo;
typedef struct v_topicInfo        u_topicInfo;
typedef struct v_publicationInfo  u_publicationInfo;
typedef struct v_subscriptionInfo u_subscriptionInfo;

typedef void *u_status;
typedef u_result (*u_statusAction)(u_status info, void *arg);

#define U_RESULT_UNDEFINED            V_RESULT_UNDEFINED
#define U_RESULT_OK                   V_RESULT_OK
#define U_RESULT_INTERRUPTED          V_RESULT_INTERRUPTED
#define U_RESULT_NOT_INITIALISED      V_RESULT_NOT_ENABLED
#define U_RESULT_OUT_OF_MEMORY        V_RESULT_OUT_OF_MEMORY
#define U_RESULT_INTERNAL_ERROR       V_RESULT_INTERNAL_ERROR
#define U_RESULT_ILL_PARAM            V_RESULT_ILL_PARAM
#define U_RESULT_CLASS_MISMATCH       V_RESULT_CLASS_MISMATCH
#define U_RESULT_DETACHING            V_RESULT_DETACHING
#define U_RESULT_TIMEOUT              V_RESULT_TIMEOUT
#define U_RESULT_OUT_OF_RESOURCES     V_RESULT_OUT_OF_RESOURCES
#define U_RESULT_INCONSISTENT_QOS     V_RESULT_INCONSISTENT_QOS
#define U_RESULT_IMMUTABLE_POLICY     V_RESULT_IMMUTABLE_POLICY
#define U_RESULT_PRECONDITION_NOT_MET V_RESULT_PRECONDITION_NOT_MET
#define U_RESULT_ALREADY_DELETED      V_RESULT_ALREADY_DELETED
#define U_RESULT_HANDLE_EXPIRED       V_RESULT_HANDLE_EXPIRED
#define U_RESULT_NO_DATA              V_RESULT_NO_DATA
#define U_RESULT_UNSUPPORTED          V_RESULT_UNSUPPORTED

#define U_STATE_READ_SAMPLE           (0x0001U << 0)
#define U_STATE_NOT_READ_SAMPLE       (0x0001U << 1)
#define U_STATE_NEW_VIEW              (0x0001U << 2)
#define U_STATE_NOT_NEW_VIEW          (0x0001U << 3)
#define U_STATE_ALIVE_INSTANCE        (0x0001U << 4)
#define U_STATE_DISPOSED_INSTANCE     (0x0001U << 5)
#define U_STATE_NOWRITERS_INSTANCE    (0x0001U << 6)

#define U_STATE_ANY_SAMPLE           (U_STATE_READ_SAMPLE | \
                                      U_STATE_NOT_READ_SAMPLE)
#define U_STATE_ANY_VIEW             (U_STATE_NEW_VIEW | \
                                      U_STATE_NOT_NEW_VIEW)
#define U_STATE_ANY_INSTANCE         (U_STATE_DISPOSED_INSTANCE | \
                                      U_STATE_NOWRITERS_INSTANCE | \
                                      U_STATE_ALIVE_INSTANCE)

#define U_STATE_ANY                  (U_STATE_ANY_SAMPLE | \
                                      U_STATE_ANY_VIEW | \
                                      U_STATE_ANY_INSTANCE)

typedef enum {
    U_UNDEFINED,
    U_ENTITY, U_PARTICIPANT, U_PUBLISHER, U_WRITER, U_SPLICED, U_SERVICE,
    U_SERVICEMANAGER, U_SUBSCRIBER, U_READER, U_NETWORKREADER,
    U_GROUPQUEUE, U_QUERY, U_DATAVIEW, U_PARTITION, U_TOPIC, U_CFTOPIC,
    U_GROUP, U_WAITSET, U_WAITSETENTRY, U_DOMAIN, U_LISTENER, U_STATUSCONDITION,
    U_COUNT
} u_kind;

C_CLASS(u_object);
C_CLASS(u_observable);
C_CLASS(u_entity);
C_CLASS(u_dispatcher);
C_CLASS(u_domain);
C_CLASS(u_group);
C_CLASS(u_partition);
C_CLASS(u_topic);
C_CLASS(u_waitset);
C_CLASS(u_participant);
C_CLASS(u_publisher);
C_CLASS(u_writer);
C_CLASS(u_subscriber);
C_CLASS(u_reader);
C_CLASS(u_dataReader);
C_CLASS(u_networkReader);
C_CLASS(u_groupQueue);
C_CLASS(u_query);
C_CLASS(u_dataView);
C_CLASS(u_service);
C_CLASS(u_serviceManager);
C_CLASS(u_spliced);
C_CLASS(u_statusCondition);
C_CLASS(u_listener);
C_CLASS(u_waitsetHistoryDeleteEvent);
C_CLASS(u_waitsetHistoryRequestEvent);
C_CLASS(u_waitsetPersistentSnapshotEvent);
C_CLASS(u_waitsetConnectWriterEvent);

typedef v_result (*u_publicationInfo_action)  (u_publicationInfo *info, void *arg);
typedef v_result (*u_subscriptionInfo_action) (u_subscriptionInfo *info, void *arg);

OS_API const os_char *
u_resultImage(
        u_result result)
    __attribute_returns_nonnull__
    __attribute_const__;

OS_API const os_char *
u_kindImage(
        u_kind kind)
    __attribute_returns_nonnull__
    __attribute_const__;

OS_API u_typeHash
u_typeHashFromArray(
        const os_uchar *array,
        os_uint32 arrLen);

/* TODO:
 * Following listener prototypes are depricated.
 * They will change or be removed after serviceManager and dispatcher are revised.
 */
typedef void (*u_serviceSplicedaemonListener)( v_serviceStateKind spliceDaemonState, void *usrData);
typedef u_eventMask (*u_observableListener)(u_observable o, u_eventMask event, void *usrData);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
