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
#ifndef GAPI_PUBLISHER_H
#define GAPI_PUBLISHER_H

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

#define U_PUBLISHER_GET(p) u_publisher(U_ENTITY_GET(p))

#define _Publisher(o) ((_Publisher)(o))

#define gapi_publisherClaim(h,r) \
        (_Publisher(gapi_objectClaim(h,OBJECT_KIND_PUBLISHER,r)))

#define gapi_publisherClaimNB(h,r) \
        (_Publisher(gapi_objectClaimNB(h,OBJECT_KIND_PUBLISHER,r)))

#define _PublisherAlloc() \
        (_Publisher(_ObjectAlloc(OBJECT_KIND_PUBLISHER, \
                                 C_SIZEOF(_Publisher), \
                                 NULL)))

_Publisher
_PublisherNew (
    u_participant uParticipant,
    const gapi_publisherQos *qos,
    const struct gapi_publisherListener *listener,
    const gapi_statusMask mask,
    const _DomainParticipant participant);

gapi_returnCode_t
_PublisherFree (
    _Publisher _this);

c_long
_PublisherWriterCount (
    _Publisher _this);

OS_API u_publisher
_PublisherUpublisher (
    _Publisher _this);

gapi_boolean
_PublisherContainsEntity (
    _Publisher _this,
    gapi_instanceHandle_t handle);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* GAPI_PUBLISHER_H */
