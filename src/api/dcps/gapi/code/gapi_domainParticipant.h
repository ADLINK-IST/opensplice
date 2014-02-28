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
#ifndef GAPI_DOMAINPARTICIPANT_H
#define GAPI_DOMAINPARTICIPANT_H

#include "gapi_common.h"
#include "gapi_object.h"

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

#define _DomainParticipant(o) ((_DomainParticipant)(o))

#define gapi_domainParticipantClaim(h,r) \
        (_DomainParticipant(gapi_objectClaim(h,OBJECT_KIND_DOMAINPARTICIPANT,r)))

#define gapi_domainParticipantClaimNB(h,r) \
        (_DomainParticipant(gapi_objectClaimNB(h,OBJECT_KIND_DOMAINPARTICIPANT,r)))

#define _DomainParticipantAlloc() \
        (_DomainParticipant(_ObjectAlloc(OBJECT_KIND_DOMAINPARTICIPANT, \
                                          C_SIZEOF(_DomainParticipant), \
                                          NULL)))
#define U_PARTICIPANT_GET(t)       u_participant(U_ENTITY_GET(t))
#define U_PARTICIPANT_SET(t,e)     _EntitySetUserEntity(_Entity(t), u_entity(e))

_DomainParticipant
_DomainParticipantNew (
    gapi_domainName_t domainId,
    const gapi_domainParticipantQos *qos,
    const struct gapi_domainParticipantListener *listener,
    const gapi_statusMask mask,
    _DomainParticipantFactory theFactory,
    gapi_listenerThreadAction threadStartAction,
    gapi_listenerThreadAction threadStopAction,
    void *actionArg,
    const gapi_context *context,
    gapi_domainId_int_t dId,
    const char *name
);

gapi_returnCode_t
_DomainParticipantFree (
    _DomainParticipant _this);

gapi_boolean
_DomainParticipantPrepareDelete (
    _DomainParticipant  _this,
    const gapi_context *context);

/* precondition: participant must be locked */
_TypeSupport
_DomainParticipantFindType (
    _DomainParticipant _this,
    const gapi_char *registry_name);

/* precondition: participant must be locked */
OS_API u_participant
_DomainParticipantUparticipant (
    _DomainParticipant _this);

/* precondition: participant must be locked */
gapi_returnCode_t
_DomainParticipantRegisterType (
    _DomainParticipant _this,
    _TypeSupport type_support,
    const gapi_char *registry_name);

/* precondition: participant must be locked */
gapi_boolean
_DomainParticipantContainsTypeSupport (
    _DomainParticipant _this,
    _TypeSupport       typeSupport);

/* precondition: participant must be locked */
_TypeSupport
_DomainParticipantFindRegisteredTypeSupport (
    _DomainParticipant _this,
    _TypeSupport       typeSupport);

/* precondition: participant must be locked */
const gapi_char *
_DomainParticipantGetRegisteredTypeName (
    _DomainParticipant _this,
    _TypeSupport       typeSupport);

/* precondition: participant must be locked */
_TopicDescription
_DomainParticipantFindTopicDescription (
    _DomainParticipant _this,
    const gapi_char *topic_name);

gapi_boolean
_DomainParticipantTopicDescriptionExists (
    _DomainParticipant _this,
    _TopicDescription  topicDescription);

gapi_boolean
_DomainParticipantSetListenerInterestOnChildren (
    _DomainParticipant    _this,
    _ListenerInterestInfo info);

gapi_domainName_t
_DomainParticipantGetDomainId (
    _DomainParticipant _this);

c_metaObject
_DomainParticipant_get_type_metadescription (
    _DomainParticipant _this,
    const gapi_char *type_name);

_Topic
_DomainParticipantFindBuiltinTopic (
    _DomainParticipant _this,
    const gapi_char   *topic_name);

_Subscriber
_DomainParticipantGetBuiltinSubscriber (
    _DomainParticipant participant);

void
_DomainParticipantGetListenerActionInfo (
    _DomainParticipant _this,
    gapi_listenerThreadAction *startAction,
    gapi_listenerThreadAction *stopAction,
    void                      **actionArg);

gapi_boolean
_DomainParticipantAddListenerInterest (
    _DomainParticipant _this,
    _Status            status);

gapi_boolean
_DomainParticipantRemoveListenerInterest (
    _DomainParticipant _this,
    _Status            status);

void
_DomainParticipantCleanup (
    _DomainParticipant _this);

gapi_returnCode_t
_DomainParticipantDeleteContainedEntitiesNoClaim (
    _DomainParticipant _this);

#undef OS_API
#if defined (__cplusplus)
}
#endif

#endif /* GAPI_DOMAINPARTICIPANT_H */
