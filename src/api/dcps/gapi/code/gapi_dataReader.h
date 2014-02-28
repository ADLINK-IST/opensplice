/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef GAPI_DATAREADER_H
#define GAPI_DATAREADER_H

#include "gapi.h"
#include "gapi_common.h"
#include "gapi_entity.h"
#include "gapi_loanRegistry.h"
#include "gapi_status.h"
#include "gapi_expression.h"
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

#define U_READER_GET(t)       u_reader(U_ENTITY_GET(t))
#define U_DATAREADER_GET(t)   u_dataReader(U_ENTITY_GET(t))
#define U_DATAREADER_SET(t,e) _EntitySetUserEntity(_Entity(t), u_entity(e))

#define _DataReader(o) ((_DataReader)(o))

#define gapi_dataReaderClaim(h,r) \
        (_DataReader(gapi_objectClaim(h,OBJECT_KIND_DATAREADER,r)))

#define gapi_dataReaderClaimNB(h,r) \
        (_DataReader(gapi_objectClaimNB(h,OBJECT_KIND_DATAREADER,r)))

#define _DataReaderFromHandle(h) \
        (_DataReader(gapi_objectPeek(h,OBJECT_KIND_DATAREADER)))

#define _DataReaderAlloc() \
        (_DataReader(_ObjectAlloc(OBJECT_KIND_DATAREADER, \
                                  C_SIZEOF(_DataReader), \
                                  NULL)))


C_STRUCT(_DataReader) {
    C_EXTENDS(_Entity);
    _TopicDescription               topicDescription;
    struct gapi_dataReaderListener  _Listener;
    gapi_readerCopy                 readerCopy;
    gapi_copyIn                     copy_in;
    gapi_copyOut                    copy_out;
    gapi_copyCache                  copy_cache;
    gapi_unsigned_long              messageOffset;
    gapi_unsigned_long              userdataOffset;
    gapi_unsigned_long              allocSize;
    gapi_topicAllocBuffer           allocBuffer;
    gapi_readerMask                 reader_mask;
    gapi_loanRegistry               loanRegistry;
    gapi_dataReaderViewQos          _defDataReaderViewQos;
};

_DataReader
_DataReaderNew (
    const _TopicDescription topicDescription,
    const _TypeSupport typesupport,
    const gapi_dataReaderQos *qos,
    const struct gapi_dataReaderListener *a_listener,
    const gapi_statusMask mask,
    const _Subscriber subscriber);

c_bool
_DataReaderInit (
    _DataReader _this,
    const _Subscriber subscriber,
    const _TopicDescription topicDescription,
    const _TypeSupport typesupport,
    const struct gapi_dataReaderListener *a_listener,
    const gapi_statusMask mask,
    const u_dataReader uReader);

gapi_returnCode_t
_DataReaderFree (
    _DataReader _this);

gapi_boolean
_DataReaderPrepareDelete (
    _DataReader _this,
    gapi_context *context);

gapi_returnCode_t
_DataReaderGetKeyValue (
    _DataReader _this,
    void *instance,
    const gapi_instanceHandle_t handle);

_Subscriber
_DataReaderSubscriber (
    _DataReader _this);

OS_API u_dataReader
_DataReaderUreader (
    _DataReader _this);

void
_DataReaderCopy (
    gapi_dataSampleSeq *samples,
    gapi_readerInfo    *info);

void
_DataReaderTriggerNotify (
    _DataReader _this);

void
_DataReaderNotifyListener(
    _DataReader _this,
    gapi_statusMask triggerMask);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* GAPI_DATAREADER_H */
