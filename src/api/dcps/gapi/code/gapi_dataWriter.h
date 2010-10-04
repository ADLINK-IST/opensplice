/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef GAPI_DATAWRITER_H
#define GAPI_DATAWRITER_H

#include "gapi.h"
#include "gapi_common.h"
#include "gapi_domainEntity.h"
#include "gapi_hashTable.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define U_WRITER_GET(w)    (u_writer(_Entity(w)->uEntity))
#define U_WRITER_SET(w,e)  _EntitySetUserEntity(_Entity(w), u_entity(e))

#define _DataWriter(o) ((_DataWriter)(o))

#define gapi_dataWriterClaim(h,r) \
        (_DataWriter(gapi_objectClaim(h,OBJECT_KIND_DATAWRITER,r)))

#define gapi_dataWriterReadClaim(h,r) \
        (_DataWriter(gapi_objectReadClaim(h,OBJECT_KIND_DATAWRITER,r)))

#define gapi_dataWriterClaimNB(h,r) \
        (_DataWriter(gapi_objectClaimNB(h,OBJECT_KIND_DATAWRITER,r)))

#define _DataWriterAlloc() \
        (_DataWriter(_ObjectAlloc(OBJECT_KIND_DATAWRITER, \
                                  C_SIZEOF(_DataWriter), \
                                  NULL)))

C_STRUCT(_DataWriter) {
    C_EXTENDS(_DomainEntity);
    _Topic                           topic;
    struct gapi_dataWriterListener   listener;
    _Publisher                       publisher;
    u_writerCopy                     uWriterCopy;
    gapi_copyIn                      copy_in;
    gapi_copyOut                     copy_out;
    gapi_copyCache                   copy_cache;
    gapi_hashTable                   registry;
};

typedef struct writerInfo_s {
    _DataWriter writer;
    void *data;
} writerInfo;

_DataWriter
_DataWriterNew (
    const _Topic topic,
    const _TypeSupport typesupport,
    const gapi_dataWriterQos *qos,
    const struct gapi_dataWriterListener *a_listener,
    const gapi_statusMask mask,
    const _Publisher publisher);

void
_DataWriterFree (
    _DataWriter _this);

gapi_boolean
_DataWriterPrepareDelete (
    _DataWriter _this);

gapi_instanceHandle_t
_DataWriterRegisterInstance (
    _DataWriter _this,
    const void *instanceData,
    c_time timestamp);

gapi_returnCode_t
_DataWriterUnregisterInstance (
    _DataWriter _this,
    const void *instanceData,
    const gapi_instanceHandle_t handle,
    c_time timestamp);

gapi_returnCode_t
_DataWriterGetKeyValue (
    _DataWriter _this,
    void *instance,
    const gapi_instanceHandle_t handle);

gapi_returnCode_t
_DataWriter_get_liveliness_lost_status (
    _DataWriter _this,
    c_bool reset,
    gapi_livelinessLostStatus *status);

gapi_returnCode_t
_DataWriter_get_offered_deadline_missed_status (
    _DataWriter _this,
    c_bool reset,
    gapi_offeredDeadlineMissedStatus *status);

gapi_returnCode_t
_DataWriter_get_offered_incompatible_qos_status (
    _DataWriter _this,
    c_bool reset,
    gapi_offeredIncompatibleQosStatus *status);

gapi_returnCode_t
_DataWriter_get_publication_matched_status (
    _DataWriter _this,
    c_bool reset,
    gapi_publicationMatchedStatus *status);

void
_DataWriterNotifyListener(
    _DataWriter _this,
    gapi_statusMask triggerMask);

#if defined (__cplusplus)
}
#endif

#endif /* GAPI_DATAWRITER_H */
