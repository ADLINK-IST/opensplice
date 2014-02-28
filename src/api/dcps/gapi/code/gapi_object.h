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
#ifndef GAPI_OBJECT_H
#define GAPI_OBJECT_H

#include "gapi_common.h"

#include "u_user.h"
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

typedef enum {
    OBJECT_KIND_UNDEFINED                 = 0x00000000,
    OBJECT_KIND_ENTITY                    = 0x00000001,
    OBJECT_KIND_DOMAINENTITY              = 0x00000003,
    OBJECT_KIND_DOMAINPARTICIPANT         = 0x00000005,
    OBJECT_KIND_TYPESUPPORT               = 0x00000008,
    OBJECT_KIND_TOPICDESCRIPTION          = 0x00000010,
    OBJECT_KIND_TOPIC                     = 0x00000033,
    OBJECT_KIND_CONTENTFILTEREDTOPIC      = 0x00000050,
    OBJECT_KIND_MULTITOPIC                = 0x00000090,
    OBJECT_KIND_PUBLISHER                 = 0x00000103,
    OBJECT_KIND_SUBSCRIBER                = 0x00000203,
    OBJECT_KIND_DATAWRITER                = 0x00000403,
    OBJECT_KIND_DATAREADER                = 0x00000803,
    OBJECT_KIND_FOOTYPESUPPORT            = 0x00001008,
    OBJECT_KIND_FOODATAWRITER             = 0x00002403,
    OBJECT_KIND_FOODATAREADER             = 0x00004803,
    OBJECT_KIND_CONDITION                 = 0x00008000,
    OBJECT_KIND_GUARDCONDITION            = 0x00018000,
    OBJECT_KIND_STATUSCONDITION           = 0x00028000,
    OBJECT_KIND_READCONDITION             = 0x00048000,
    OBJECT_KIND_QUERYCONDITION            = 0x000C8000,
    OBJECT_KIND_WAITSET                   = 0x00100000,
    OBJECT_KIND_STATUS                    = 0x00200000,
    OBJECT_KIND_PARTICIPANT_STATUS        = 0x00600000,
    OBJECT_KIND_TOPIC_STATUS              = 0x00A00000,
    OBJECT_KIND_PUBLISHER_STATUS          = 0x01200000,
    OBJECT_KIND_SUBSCRIBER_STATUS         = 0x02200000,
    OBJECT_KIND_WRITER_STATUS             = 0x04200000,
    OBJECT_KIND_READER_STATUS             = 0x08200000,
    OBJECT_KIND_DATAVIEW                  = 0x10000001,
    OBJECT_KIND_FOODATAVIEW               = 0x30000001,
    OBJECT_KIND_DOMAINPARTICIPANTFACTORY  = 0x40000001,
    OBJECT_KIND_DOMAIN                    = 0x50000001,
    OBJECT_KIND_ERRORINFO                 = 0x60000000,
    OBJECT_KIND_QOSPROVIDER               = 0x70000000
} _ObjectKind;


#define _Object(o) ((_Object)o)
#define _RWLOCK_

C_CLASS(_ObjectRegistry);
C_CLASS(_Object);

C_STRUCT(_Object) {
    gapi_object handle;
};

OS_API _Object
gapi_objectClaim (
    gapi_object handle,
    _ObjectKind kind,
    gapi_returnCode_t *result);

_Object
gapi_objectClaimNB (
    gapi_object handle,
    _ObjectKind kind,
    gapi_returnCode_t *result);

_Object
gapi_objectPeek (
    gapi_object handle,
    _ObjectKind kind);

#ifdef _RWLOCK_
OS_API _Object
gapi_objectReadClaim (
    gapi_object handle,
    _ObjectKind kind,
    gapi_returnCode_t *result);

_Object
gapi_objectReadClaimNB (
    gapi_object handle,
    _ObjectKind kind,
    gapi_returnCode_t *result);

_Object
gapi_objectReadPeek (
    gapi_object handle,
    _ObjectKind kind);
#endif

gapi_object
_ObjectToHandle (
    _Object object);

OS_API _Object
gapi_objectPeekUnchecked (
    gapi_object handle);

OS_API gapi_object
gapi_objectRelease (
    gapi_object handle);

#ifdef _RWLOCK_
gapi_object
gapi_objectReadRelease (
    gapi_object handle);
#endif

void
gapi_objectClearBusy (
    gapi_object handle);

_ObjectKind
gapi_objectGetKind(
    gapi_object handle);

_ObjectRegistry
_ObjectRegistryNew (
    void);

void
_ObjectRegistryFree (
    _ObjectRegistry registry);

void
_ObjectRegistryRegister (
    _ObjectRegistry registry,
    _Object         object);

OS_API _Object
_ObjectAlloc (
    _ObjectKind     kind,
    long            size,
    gapi_boolean    (*freeFunc)(void *));

void
_ObjectDelete (
    _Object object);

OS_API void
_ObjectClaim (
    _Object object);

void
_ObjectClaimNotBusy (
    _Object object);

OS_API gapi_object
_ObjectRelease (
    _Object);

#ifdef _RWLOCK_
OS_API void
_ObjectReadClaim (
    _Object object);

void
_ObjectReadClaimNotBusy (
    _Object object);

OS_API gapi_object
_ObjectReadRelease (
    _Object);
#endif

gapi_boolean
_ObjectIsValid (
    _Object object);

void *
_ObjectGetUserData (
    _Object object);

void
_ObjectSetUserData (
    _Object object,
    void *userData);

void
_ObjectSetDeleteAction (
    _Object object,
    gapi_deleteEntityAction action,
    void *actionData);

gapi_boolean
_ObjectGetDeleteAction (
    _Object object,
    gapi_deleteEntityAction *action,
    void **actionData);

void
_ObjectSetBusy (
    _Object object);

void
_ObjectClearBusy (
    _Object object);

_ObjectKind
_ObjectGetKind(
    _Object object);

os_result
_ObjectTimedWait(
    _Object object,
    os_cond *cv,
    const os_time *timeout);

os_result
_ObjectWait(
    _Object object,
    os_cond *cv);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* GAPI_OBJECT_H */
