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

#ifndef D__DURABILITY_H
#define D__DURABILITY_H

#include "d__types.h"
#include "d__thread.h"
#include "u_user.h"
#include "vortex_os.h"
#include "os_thread.h"
#include "os_time.h"
#include "os_atomics.h"


#if defined (__cplusplus)
extern "C" {
#endif

/* Vendor ids, used for client-durability */
#define D_VENDORID_UNKNOWN                          {{ 0x00, 0x00 }}
#define D_VENDORID_RTI                              {{ 0x01, 0x01 }}
#define D_VENDORID_PRISMTECH_OSPL                   {{ 0x01, 0x02 }}
#define D_VENDORID_OCI                              {{ 0x01, 0x03 }}
#define D_VENDORID_MILSOFT                          {{ 0x01, 0x04 }}
#define D_VENDORID_KONGSBERG                        {{ 0x01, 0x05 }}
#define D_VENDORID_TWINOAKS                         {{ 0x01, 0x06 }}
#define D_VENDORID_LAKOTA                           {{ 0x01, 0x07 }}
#define D_VENDORID_ICOUP                            {{ 0x01, 0x08 }}
#define D_VENDORID_ETRI                             {{ 0x01, 0x09 }}
#define D_VENDORID_RTI_MICRO                        {{ 0x01, 0x0a }}
#define D_VENDORID_PRISMTECH_JAVA                   {{ 0x01, 0x0b }}
#define D_VENDORID_PRISMTECH_GATEWAY                {{ 0x01, 0x0c }}
#define D_VENDORID_PRISMTECH_LITE                   {{ 0x01, 0x0d }}
#define D_VENDORID_TECHNICOLOR                      {{ 0x01, 0x0e }}
#define D_VENDORID_EPROSIMA                         {{ 0x01, 0x0f }}
#define D_VENDORID_PRISMTECH_CLOUD                  {{ 0x01, 0x20 }}

#define MY_VENDOR_ID                                D_VENDORID_PRISMTECH_OSPL

/* Durability version */
#define D_DURABILITY_VERSION_MAJOR                  2
#define D_DURABILITY_VERSION_MINOR                  0  /* From version 2.0 timestamps beyond 2038 are supported */
#define D_DURABILITY_VERSION_VENDOR_ID              MY_VENDOR_ID

/**
 * Macro that checks the d_durability validity.
 * Because d_durability is a concrete class typechecking is required.
 */
#define             d_durabilityIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_DURABILITY)

/**
 * \brief The d_durability cast macro.
 *
 * This macro casts an object to a d_durability object.
 */
#define d_durability(_this) ((d_durability)(_this))

typedef void (*d_durabilityStatisticsCallback)(v_durabilityStatistics statistics, c_voidp userData);

C_STRUCT(d_durability){
    C_EXTENDS(d_object);
    u_service        service;
    u_serviceManager serviceManager;
    d_configuration  configuration;
    d_admin          admin;
    d_serviceState   state;
    os_threadId      statusThread;
    volatile c_bool  splicedRunning;
    os_cond          terminateCondition;
    os_mutex         terminateMutex;
    c_bool           died;                /* set to TRUE if something bad happened,
                                           * used in d_durabilityDeinit to annnounce
                                           * the service has died
                                           */
    struct _DDS_DurabilityVersion_t myVersion;
    struct _DDS_Gid_t myServerId;
    pa_uint32_t      requestId;           /* atomic id to handout to new requests */
    pa_uint32_t      conflictId;          /* monotonic conflict-id counter */
    pa_uint32_t      incarnation;         /* incarnation used to detect asymmetric disconnects */
    pa_uint32_t      forceStatusWrite;
};

typedef enum d_connectivity_s {
    D_CONNECTIVITY_UNDETERMINED,
    D_CONNECTIVITY_OK,
    D_CONNECTIVITY_INCOMPATIBLE_STATE,
    D_CONNECTIVITY_INCOMPATIBLE_DATA_MODEL
} d_connectivity;

#define D_OSPL_NODE "__NODE"
#define D_OSPL_BUILTIN_PARTITION "BUILT-IN PARTITION__"

d_durability        d_durabilityNew                     (const c_char* uri,
                                                         const c_char* serviceName,
                                                         c_long domainId);

void                d_durabilityInit                    (d_durability durability);

void                d_durabilityDeinit                  (d_durability durability);

d_connectivity      d_durabilityDetermineConnectivity   (d_durability durability);

void                d_durabilityHandlePersistentInitial (d_durability durability);

void                d_durabilityHandleInitialAlignment  (d_durability durability);

void                d_durabilityFree                    (d_durability durability);

c_bool              d_durabilityArgumentsProcessing     (int argc,
                                                         char *argv[],
                                                         c_char **uri,
                                                         c_char **serviceName);

void                d_durabilityWatchSpliceDaemon       (v_serviceStateKind spliceDaemonState,
                                                         c_voidp usrData);

void                d_durabilityLoadModule              (v_public entity,
                                                         c_voidp args);

u_service           d_durabilityGetService              (d_durability durability);

d_configuration     d_durabilityGetConfiguration        (d_durability durability);

c_bool              d_durabilityWaitForAttachToGroup    (d_durability durability,
                                                         v_group group);

d_serviceState      d_durabilityGetState                (d_durability durability);

void                d_durabilitySetState                (d_durability durability,
                                                         d_serviceState state);

void                d_durabilityHeartbeatProcessed      (_In_ d_durability durability);

c_bool              d_durabilityMustTerminate           (d_durability durability);

void                d_durabilityTerminate               (d_durability durability,
                                                         c_bool died);

void                d_durabilityUpdateStatistics        (d_durability durability,
                                                         d_durabilityStatisticsCallback callback,
                                                         c_voidp userData);

u_result            d_durabilityTakePersistentSnapshot  (d_durability durability,
                                                         c_char* partitionExpr,
                                                         c_char* topicExpr,
                                                         c_char* uri);

void                d_durabilitySetVersion              (d_durability durability,
                                                         c_ushort major,
                                                         c_ushort minor);

struct _DDS_DurabilityVersion_t d_durabilityGetMyVersion (d_durability durability);

struct _DDS_Gid_t   d_durabilityGetMyServerId           (d_durability durability);

c_bool              d_durabilityRequestIsForMe          (d_durability durability,
                                                         c_iter serverIds,
                                                         c_bool *forMe,
                                                         c_bool *forEverybody);

struct _DDS_RequestId_t  d_durabilityGetRequestId       (d_durability durability);

c_ulong             d_durabilityGenerateConflictId      (_Inout_ d_durability durability);

d_fellow            d_durabilityGetOrCreateFellowFromMessage(d_admin admin,
                                                             d_networkAddress fellowAddr,
                                                             d_message message);

c_ulong             d_durabilityGetNewIncarnation       (d_durability durability);

void                d_durabilityDetermineNameSpaceCompleteness (
                                                         d_durability durability);

void                d_durabilityDoInitialMerge          (d_durability durability);

void                d_durabilityReportKernelGroupCompleteness (_In_ d_durability durability);

#if defined (__cplusplus)
}
#endif

#endif /* D__DURABILITY_H */
