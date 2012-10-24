/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef U_TYPES_H
#define U_TYPES_H

#include "os_heap.h"
#include "c_typebase.h"
#include "c_iterator.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief The result values all methods of the classes within the
 * user component can return.
 */

typedef v_result u_result;

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
#define U_RESULT_INCONSISTENT_QOS     V_RESULT_INCONSISTENT_QOS
#define U_RESULT_IMMUTABLE_POLICY     V_RESULT_IMMUTABLE_POLICY
#define U_RESULT_PRECONDITION_NOT_MET V_RESULT_PRECONDITION_NOT_MET
#define U_RESULT_ALREADY_DELETED      V_RESULT_ALREADY_DELETED
#define U_RESULT_UNSUPPORTED          V_RESULT_UNSUPPORTED

typedef enum {
    U_SERVICE_NETWORKING,
    U_SERVICE_DURABILITY,
    U_SERVICE_CMSOAP,
    U_SERVICE_SPLICED,
    U_SERVICE_INCOGNITO,
    U_SERVICE_DDSI,
    U_SERVICE_SNETWORKING
    /* appending new service type add ends  */
} u_serviceKind;

typedef enum {
    U_UNDEFINED,
    U_ENTITY, U_PARTICIPANT, U_PUBLISHER, U_WRITER, U_SERVICE,
    U_SERVICEMANAGER, U_SUBSCRIBER, U_READER, U_NETWORKREADER,
    U_GROUPQUEUE, U_QUERY, U_DATAVIEW, U_PARTITION, U_TOPIC, U_CFTOPIC,
    U_GROUP, U_WAITSET, U_DOMAIN,
    U_COUNT
} u_kind;

C_CLASS(u_object);
C_CLASS(u_entity);
C_CLASS(u_dispatcher);
C_CLASS(u_domain);
C_CLASS(u_group);
C_CLASS(u_partition);
C_CLASS(u_topic);
C_CLASS(u_contentFilteredTopic);
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
C_CLASS(u_waitsetEvent);
C_CLASS(u_waitsetHistoryDeleteEvent);
C_CLASS(u_waitsetHistoryRequestEvent);
C_CLASS(u_waitsetPersistentSnapshotEvent);

OS_API c_char *
u_resultImage(
    u_result result);

OS_API c_char *
u_kindImage(
    u_kind kind);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
