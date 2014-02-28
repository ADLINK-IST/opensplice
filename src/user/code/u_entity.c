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
#include "u__entity.h"
#include "u__types.h"
#include "u__qos.h"
#include "u_partition.h"
#include "u__topic.h"
#include "u_group.h"
#include "u_groupQueue.h"
#include "u_publisher.h"
#include "u_subscriber.h"
#include "u__writer.h"
#include "u_reader.h"
#include "u_dataReader.h"
#include "u_dataView.h"
#include "u_networkReader.h"
#include "u_query.h"
#include "u__participant.h"
#include "u_serviceManager.h"
#include "u_waitset.h"
#include "u__user.h"

#include "v_entity.h"
#include "v_public.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v_writer.h"
#include "v_partition.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_status.h"
#include "v_service.h"
#include "v_networking.h"
#include "v_durability.h"
#include "v_cmsoap.h"
#include "v_rnr.h"
#include "v_dataReader.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"
#include "v_spliced.h"
#include "v_waitset.h"
#include "v_networkReader.h"
#include "v_groupQueue.h"
#include "v_dataView.h"

#include "os_report.h"


static u_result
u_entityClaimCommon(
    u_entity e,
    v_entity* ke,
    os_boolean performMemCheck);

static u_domain
u_entityDomain (
    u_entity _this)
{
    u_domain domain = NULL;

    /* Precondition: entity must be locked. */
    if (_this != NULL) {
        switch (_this->kind ) {
        case U_SERVICE:
            domain = u_participantDomain(u_participant(_this));
            if (domain == NULL) {
                /* could be a cm object */
                domain = u_participantDomain(_this->participant);
            }
        break;
        case U_PARTICIPANT:
            domain = u_participantDomain(u_participant(_this));
        break;
        case U_DOMAIN:
            domain = u_domain(_this);
        break;
        default:
            domain = u_participantDomain(_this->participant);
        }
    }
    return domain;
}


u_entity
u_entityNew(
    v_entity ke,
    u_participant p,
    c_bool owner)
{
    u_entity e = NULL;

    if (ke == NULL) {
        OS_REPORT(OS_ERROR,
                  "user::u_entity::u_entityNew", 0,
                  "No Kernel entity specified.");
        return NULL;
    }

/* Note! The following memset is required because this method doesn't
 * fully initializes the members of subclasses.
 */
#define _NEW_(t,l) \
    e = u_entity(os_malloc((os_uint32)sizeof(C_STRUCT(t)))); \
    if (e == NULL) { \
        OS_REPORT(OS_ERROR, "u_entityNew", 0, \
                  "Failed to allocate User proxy."); \
        return NULL; \
    } else { \
        memset(e,0,(os_uint32)sizeof(C_STRUCT(t))); \
    } e->kind = l

    switch(v_objectKind(ke)) {
    case K_DOMAIN:         _NEW_(u_partition,U_PARTITION);            break;
    case K_TOPIC:          _NEW_(u_topic, U_TOPIC);                   break;
    case K_GROUP:          _NEW_(u_group, U_GROUP);                   break;
    case K_PUBLISHER:      _NEW_(u_publisher, U_PUBLISHER);           break;
    case K_SUBSCRIBER:     _NEW_(u_subscriber, U_SUBSCRIBER);         break;
    case K_WRITER:         _NEW_(u_writer, U_WRITER);                 break;
    case K_DATAREADER:     _NEW_(u_dataReader, U_READER);             break;
    case K_DELIVERYSERVICE:_NEW_(u_dataReader, U_READER);             break;
    case K_DATAVIEWQUERY:   /* is just a query, so no break here */
    case K_DATAREADERQUERY: /* is just a query, so no break here */
    case K_QUERY:          _NEW_(u_query, U_QUERY);                   break;
    case K_DATAVIEW:       _NEW_(u_dataView, U_DATAVIEW);             break;
    case K_PARTICIPANT:    _NEW_(u_participant, U_PARTICIPANT);
        u_participant(e)->domain = u_participantDomain(p);
        p = u_participant(e);                                         break;
    case K_SERVICEMANAGER: _NEW_(u_serviceManager, U_SERVICEMANAGER); break;
    case K_NETWORKING:
    case K_DURABILITY:
    case K_CMSOAP:
    case K_RNR:
    case K_SERVICE:        _NEW_(u_service, U_SERVICE);               break;
    case K_SPLICED:        _NEW_(u_spliced, U_SERVICE);               break;
    case K_WAITSET:        _NEW_(u_waitset, U_WAITSET);               break;
    case K_NETWORKREADER:  _NEW_(u_networkReader, U_NETWORKREADER);   break;
    case K_GROUPQUEUE:     _NEW_(u_groupQueue, U_GROUPQUEUE);         break;
    case K_KERNEL:         _NEW_(u_domain, U_DOMAIN);                 break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "user::u_entity::u_entityNew", 0,
                    "Unknown entity %d", v_objectKind(ke));
        e = NULL;
        assert(FALSE);
    break;
    }

#undef _NEW_

    if (e != NULL) {
        u_entityInit(e,ke,p,owner);
    }
    return e;
}

u_result
u_entityInit(
    u_entity e,
    v_entity ke,
    u_participant p,
    c_bool owner)
{
    os_mutexAttr mutexAttr;
    u_result result;

    assert(e != NULL);
    assert(ke != NULL);

    e->participant = p;
    e->userData = NULL;
    e->refCount = 1;
    e->flags = 0;

    e->enabled = v_entityEnabled(ke);
    e->magic = v_objectKernel(ke);
    e->gid = v_publicGid(v_public(ke));
    e->handle = u_handleNew(v_public(ke));

    if (!u_handleIsNil (e->handle)) {
        if (owner) {
            e->flags |= U_ECREATE_OWNED;
            v_entitySetUserData(ke,e); /* only set user data when owned! */
        }
        os_mutexAttrInit(&mutexAttr);
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        os_mutexInit(&e->mutex,&mutexAttr);

        if (e->kind == U_TOPIC) {
            result = u_topicInit(u_topic(e),v_entityName(ke),p);
        } else {
            result = U_RESULT_OK;
        }
    } else {
        OS_REPORT_1(OS_ERROR,
                    "user::u_entity::u_entityInit", 0,
                    "Out of memory: unable to create handle for Entity 0x%x.",
                    e);
        result = U_RESULT_OUT_OF_MEMORY;
    }

    return result;
}

#ifndef NDEBUG
u_entity
u_entityCheckType(
    u_entity _this,
    u_kind kind)
{
    u_entity result = NULL;

    if (_this) {
        if (u_entityKind(_this) == kind) {
            result = _this;
        } else if ((u_entityKind(_this) == U_SERVICE) &&
                   (kind == U_PARTICIPANT))
        {
            result = _this;
        } else {
            OS_REPORT_2(OS_ERROR,
                        "user::u_entity::u_entityCheckType", 0,
                        "User layer Entity type check failed, "
                        "type = %s but expected %s",
                        u_kindImage(u_entityKind(_this)),
                        u_kindImage(kind));
        }
    }
    return result;
}
#endif

u_entity
u_entityKeep(
    u_entity _this)
{
    /* Precondition: entity must be locked. */
    if ((_this) && (_this->refCount > 0)) {
        pa_increment(&_this->refCount);
        return _this;
    } else {
        return NULL;
    }
}

c_long
u_entityRefCount(
    u_entity _this)
{
    /* Precondition: entity must be locked. */
    if (_this) {
        return _this->refCount;
    } else {
        return 0;
    }
}

u_result
u_entityDeinit(
    u_entity _this)
{
    v_entity o;
    u_result r;
    os_result osr;
    u_domain domain;

    if (_this == NULL) {
        r = U_RESULT_ILL_PARAM;
    } else {
        if (_this->flags & U_ECREATE_OWNED) {
            domain = u_entityDomain(_this);
            if (domain) {
                r = u_domainProtect(domain);
                if (r == U_RESULT_OK) {
                    r = u_entityReadClaim(_this, &o);

#define _FREE_(c) c##Free(c(o)); break

                    if (r == U_RESULT_OK) {
                        switch(v_objectKind(o)) {
                        case K_PARTICIPANT:      _FREE_(v_participant);
                        case K_PUBLISHER:        _FREE_(v_publisher);
                        case K_SUBSCRIBER:       _FREE_(v_subscriber);
                        case K_WRITER:           _FREE_(v_writer);
                        case K_DATAREADER:       _FREE_(v_dataReader);
                        case K_NETWORKREADER:    _FREE_(v_networkReader);
                        case K_GROUPQUEUE:       _FREE_(v_groupQueue);
                        case K_DATAVIEW:         _FREE_(v_dataView);
                        case K_TOPIC: break;     /* _FREE_(v_topic); due to refcount issue */
                        case K_DOMAIN:           _FREE_(v_partition);
                        case K_GROUP:            _FREE_(v_group);
                        case K_SERVICEMANAGER:   /* Is never freed! */
                        break;
                        case K_SPLICED:          _FREE_(v_spliced);
                        case K_SERVICE:          _FREE_(v_service);
                        case K_NETWORKING:       _FREE_(v_networking);
                        case K_DURABILITY:       _FREE_(v_durability);
                        case K_CMSOAP:           _FREE_(v_cmsoap);
                        case K_RNR:              _FREE_(v_rnr);
                        case K_SERVICESTATE:   /* Is never freed! */
                        break;
                        case K_DELIVERYSERVICE:
                        case K_KERNELSTATUS:
                        case K_PARTICIPANTSTATUS:
                        case K_DOMAINSTATUS:
                        case K_TOPICSTATUS:
                        case K_SUBSCRIBERSTATUS:
                        case K_READERSTATUS:
                        case K_WRITERSTATUS:     _FREE_(v_status);
                        case K_QUERY:
                            OS_REPORT(OS_ERROR, "u_entityDeinit",
                                      0, "deinit of abstract class K_QUERY");
                        break;
                        case K_DATAVIEWQUERY:    _FREE_(v_dataViewQuery);

                        case K_DATAREADERQUERY:  _FREE_(v_dataReaderQuery);
                        case K_WAITSET:          _FREE_(v_waitset);
                        case K_WRITERINSTANCE:
                        case K_DATAREADERINSTANCE:
                        break;
                        default:
                            OS_REPORT_1(OS_ERROR,
                                        "user::u_entity::u_entityDeinit",0,
                                        "illegal entity kind (%d) specified",
                                        v_objectKind(o));
                        break;
                        }
                        r = u_entityRelease(_this);
                        if (r != U_RESULT_OK) {
                            OS_REPORT_2(OS_ERROR,
                                        "user::u_entity::u_entityDeinit",0,
                                        "Operation u_entityRelease(entity=0x%x) failed."
                                        OS_REPORT_NL "Result = %s.",
                                        _this, u_resultImage(r));
                        }
                    }/* else
                      * This happens when an entity is freed and its u_participant
                      * already has been freed. It means this case is a valid situation.
                      */
#undef _FREE_
                    u_domainUnprotect(domain);
                } else {
                    OS_REPORT(OS_ERROR,
                              "user::u_entity::u_entityDeinit",0,
                              "u_domainProtect() failed.");
                }
            } else {
                OS_REPORT_1(OS_ERROR,"user::u_entity::u_entityDeinit",0,
                            "Operation u_entityDomain(entity=0x%x) failed.",
                            _this);
            }
        }
        _this->kind = U_UNDEFINED;
        _this->magic = NULL;
        _this->gid = v_publicGid(NULL);
        _this->participant = NULL;
        _this->userData = NULL;
        /* Since this call needs to be done in locked condition, the mutex needs
         * to be unlocked here before destroying. */
        os_mutexUnlock(&_this->mutex);
        if((osr = os_mutexDestroy(&_this->mutex)) != os_resultSuccess){
            OS_REPORT_1(OS_ERROR, "user::u_entity::u_entityDeinit", 0, "Operation os_mutexDestroy failed, result: %s\n", os_resultImage(osr));
        }

    }
    return U_RESULT_OK;
}

c_bool
u_entityDereference(
    u_entity _this)
{
    os_uint32 refCount = 1;

    /* Precondition: entity must be locked. */
    if (_this) {
        if (_this->refCount > 0) {
            refCount = pa_decrement(&_this->refCount);
        }
    }
    return (refCount == 0);
}

void
u_entityDealloc (
    u_entity _this)
{
    os_free(_this);
}

u_result
u_entityFree(
    u_entity _this)
{
    u_result r = U_RESULT_ILL_PARAM;

    if (_this) {
#define _FREE_(c) r = c##Free(c(_this)); break
        switch(_this->kind) {
        case U_PARTICIPANT:      _FREE_(u_participant);
        case U_PUBLISHER:        _FREE_(u_publisher);
        case U_SUBSCRIBER:       _FREE_(u_subscriber);
        case U_WRITER:           _FREE_(u_writer);
        case U_READER:           _FREE_(u_dataReader);
        case U_NETWORKREADER:    _FREE_(u_networkReader);
        case U_GROUPQUEUE:       _FREE_(u_groupQueue);
        case U_DATAVIEW:         _FREE_(u_dataView);
        case U_TOPIC:            _FREE_(u_topic);
        case U_PARTITION:        _FREE_(u_partition);
        case U_SERVICEMANAGER:   _FREE_(u_serviceManager);
        case U_SERVICE:          _FREE_(u_service);
        case U_QUERY:            _FREE_(u_query);
        case U_WAITSET:          _FREE_(u_waitset);
        case U_GROUP:            _FREE_(u_group);
        default:
            OS_REPORT_1(OS_ERROR,
                        "user::u_entity::u_entityFree",0,
                        "illegal entity kind (%d) specified",
                        _this->kind);
        break;
        }
#undef _FREE_
    }
    return r;
}

u_result
u_entityWriteClaim(
    u_entity e,
    v_entity* ke)
{
    return u_entityClaimCommon(e, ke, OS_TRUE);
}

u_result
u_entityReadClaim(
    u_entity e,
    v_entity* ke)
{
    return u_entityClaimCommon(e, ke, OS_FALSE);
}

u_result
u_entityClaimCommon(
    u_entity _this,
    v_entity* ke,
    os_boolean performMemCheck)
{
    static os_boolean serviceWarningGiven = OS_FALSE;
    static os_boolean appWarningGiven = OS_FALSE;
    u_domain domain;
    u_result r;
    u_kind entityKind;
    os_boolean isService = OS_FALSE;

    /* Precondition: entity must be locked. */
    if (_this != NULL && ke != NULL) {
        *ke = NULL;
        entityKind = u_entityKind(_this);
        /* Verify if this entity is or belong to a service */
        if(_this->participant == NULL)
        {
            if(entityKind == U_SERVICE)
            {
                isService = OS_TRUE;
            }
        } else if(u_entityKind(u_entity(_this->participant)) == U_SERVICE)
        {
            isService = OS_TRUE;
        }
        domain = u_entityDomain(_this);
        if (domain) {
            r = u_domainProtect(domain);
            if (r == U_RESULT_OK) {
                if(entityKind == U_DOMAIN)
                {
                    *ke = v_entity(u_domain(_this)->kernel);
                    if(*ke == NULL)
                    {
                        OS_REPORT_1(OS_ERROR,
                                    "user::u_entity::u_entityClaimCommon", 0,
                                    "Unable to obtain kernel entity for domain 0x%x",
                                    _this);
                        r = U_RESULT_INTERNAL_ERROR;
                        u_domainUnprotect(domain);
                    }
                } else
                {
                    r = u_handleClaim(_this->handle,ke);
                    if (r != U_RESULT_OK) {
                        OS_REPORT_4(OS_WARNING,
                                    "user::u_entity::u_entityClaimCommon", 0,
                                    "Invalid handle detected: result = %s, "
                                    "Handle = %d, Entity = 0x%x (kind = %s)",
                                    u_resultImage(r), 0,
                                    _this, u_kindImage(_this->kind));
                        u_domainUnprotect(domain);
                    }
                }
            } else if (r == U_RESULT_DETACHING) {
                OS_REPORT_2(OS_WARNING,
                            "user::u_entity::u_entityClaimCommon", 0,
                            "Claim Entity failed because the process "
                            "is detaching from the domain. Entity = 0x%x (kind = %s)",
                            _this, u_kindImage(_this->kind));
            } else {
                OS_REPORT_4(OS_ERROR, "u_entityClaimCommon", 0,
                            "u_domainProtect() failed: result = %s, "
                            "Domain = 0x%x, Entity = 0x%x (kind = %s)",
                            u_resultImage(r), domain, _this, u_kindImage(_this->kind));
            }
            if (r == U_RESULT_OK)
            {
                /* If the participant we found earlier is not a service, then
                 * we must verify if the spliced is still running. If it not
                 * running anymore we must return an error and not allow the
                 * claim operation to complete.
                 */
                if(!isService && u_entityKind(_this) != U_PARTICIPANT)
                {
                    v_kernel kernel;

                    kernel = v_objectKernel(v_object(*ke));
                    if(!kernel->splicedRunning)
                    {
                        OS_REPORT_1(OS_ERROR, "u_entityClaimCommon", 0,
                            "The splice deamon is no longer running for entity 0x%x. "
                            "Unable to continue, a restart of the splice deamon and "
                            "all applications is required.",
                            _this);
                        r = U_RESULT_INTERNAL_ERROR;
                    }/* else let splicedRunning remain false */
                }
                if(performMemCheck && r == U_RESULT_OK)
                {
                    c_memoryThreshold status;
                    c_base base;

                    base = c_getBase(c_object(*ke));
                    status = c_baseGetMemThresholdStatus(base);
                    if(isService)
                    {
                        if (status == C_MEMTHRESHOLD_SERV_REACHED)
                        {
                            if(!serviceWarningGiven)
                            {
                                serviceWarningGiven = OS_TRUE;
                                OS_REPORT(OS_WARNING, "u_entityClaimCommon", 0,
                                      "Unable to complete claim for service. Shared "
                                      "memory has run out. You can try to free up some "
                                      "memory by terminating (a) DDS application(s).");
                            }
                            r = U_RESULT_OUT_OF_MEMORY;
                        }
                        else
                        {
                            r = U_RESULT_OK;
                        }
                    } else if(status != C_MEMTHRESHOLD_OK) /* APP- or SERVICE-threshold reached */
                    {
                        if(!appWarningGiven)
                        {
                            appWarningGiven = OS_TRUE;
                            OS_REPORT(OS_WARNING, "u_entityClaimCommon", 0,
                                "Unable to complete claim for application. Shared "
                                "memory has run out. You can try to free up some "
                                "memory by terminating (a) DDS application(s).");
                        }
                        r = U_RESULT_OUT_OF_MEMORY;
                    } else
                    {
                        r = U_RESULT_OK;
                    }
                }
                /* If the result has become not ok, release the handle claim */
                if(r != U_RESULT_OK)
                {
                    u_result r2;

                    *ke = NULL;
                    r2 = u_entityRelease(_this);
                    if(r2 != U_RESULT_OK)
                    {
                        OS_REPORT(OS_WARNING,"u_entityClaimCommon",0,
                            "u_entityRelease() failed.");
                    }
                }
            }
        } else {
            OS_REPORT_2(OS_ERROR,"u_entityClaimCommon",0,
                        "Could not resolve Domain from Entity 0x%x (kind = %s)",
                        _this, u_kindImage(_this->kind));
            r = U_RESULT_ILL_PARAM;
        }
    } else
    {
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_entityRelease(
    u_entity _this)
{
    u_domain domain;
    u_result result = U_RESULT_OK;
    u_kind entityKind;

    if (_this != NULL) {
        entityKind = u_entityKind(_this);
        if(entityKind != U_DOMAIN)
        {
            result = u_handleRelease(_this->handle);
            if (result != U_RESULT_OK) {
                OS_REPORT_3(OS_INFO,
                            "user::u_entity::u_entityRelease", 0,
                            "Failed to release the handle of entity 0x%x (kind = %s),"
                            OS_REPORT_NL "result = %s (This is normal on process exit)",
                            _this, u_kindImage(entityKind), u_resultImage(result));
            }
        }
        domain = u_entityDomain(_this);
        if (domain) {
            result = u_domainUnprotect(domain);
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_INFO,
                          "user::u_entity::u_entityRelease", 0,
                          "u_domainUnprotect() failed.");
            }
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "user::u_entity::u_entityRelease", 0,
                  "Invalid parameter specified");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

static c_bool
u_entityCheck(
    u_entity _this)
{
    c_bool result = FALSE;

    if (_this) {
        if ((_this->kind > U_UNDEFINED) && (_this->kind < U_COUNT)) {
            result = v_gidIsFromKernel(_this->gid, _this->magic);
            if (!result) {
                OS_REPORT_3(OS_WARNING,
                            "u_entityCheck",0,
                            "Invalid Entity (0x%x) GID.systemId (%d) != magic (%d)",
                            _this, _this->gid.systemId, _this->magic);
            }
        } else {
            OS_REPORT_2(OS_WARNING,
                        "u_entityCheck",0,
                        "Invalid Entity kind: Entity = (0x%x), kind = %d",
                        _this, _this->kind);
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_entityCheck",0,
                  "Given Entity = <NULL>");
    }
    return result;
}

u_result
u_entityLock(
    u_entity e)
{
    os_result osResult;
    u_result result;

    if (e != NULL) {
        /* Check integrity of the Entity (e) */
        if (u_entityCheck(e)) {
            osResult = os_mutexLock(&e->mutex);
            if (osResult == os_resultSuccess) {
                result = U_RESULT_OK;
            } else {
                OS_REPORT(OS_ERROR,
                          "u_entityLock", 0,
                          "Invalid mutex detected");
                result = U_RESULT_ILL_PARAM;
            }
        } else {
            OS_REPORT_1(OS_WARNING,
                        "u_entityLock", 0,
                        "Lock operation cancelled: invalid Entity = 0x%x",
                        e);
            result = U_RESULT_ILL_PARAM;
        }
    } else {
        OS_REPORT(OS_INFO,
                  "u_entityLock", 0,
                  "Lock operation cancelled: Entity = <NULL>");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_entityUnlock(
    u_entity _this)
{
    os_result osResult;
    u_result result;

    if (_this != NULL) {
        /* Check integrity of the Entity (_this) */
        if (u_entityCheck(_this)) {
            osResult = os_mutexUnlock(&_this->mutex);
            if (osResult == os_resultSuccess) {
                result = U_RESULT_OK;
            } else {
                OS_REPORT_1(OS_ERROR, "u_entityUnlock", 0,
                            "Invalid Entity mutex detected, Entity = 0x%x",
                            _this);
                result = U_RESULT_ILL_PARAM;
            }
        } else {
            OS_REPORT_1(OS_WARNING,
                        "u_entityUnlock", 0,
                        "Unlock operation cancelled: invalid Entity = 0x%x",
                        _this);
            result = U_RESULT_ILL_PARAM;
        }
    } else {
        OS_REPORT(OS_WARNING, "u_entityUnlock", 0,
                  "Unlock operation cancelled: Entity = <NULL>");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

void
u_entityEnable(
    u_entity _this)
{
    v_entity ke;
    u_result result;

    result = u_entityWriteClaim(_this, &ke);
    if (result == U_RESULT_OK) {
        v_entityEnable(ke);
        _this->enabled = TRUE;
        u_entityRelease(_this);
    } else {
        OS_REPORT_1(OS_ERROR, "u_entityEnable", 0,
                    "Unable to enable Entity (0x%x)",
                    _this);
    }
}

u_participant
u_entityParticipant(
    u_entity _this)
{
    if (_this == NULL) {
        return NULL;
    }
    return _this->participant;
}

c_bool
u_entityOwner(
    u_entity _this)
{
    c_bool result = FALSE;

    if (_this != NULL) {
        result = ((_this->flags & U_ECREATE_OWNED) == U_ECREATE_OWNED);
    }
    return result;
}

c_bool
u_entityEnabled (
    u_entity _this)
{
    v_entity ke;
    c_bool enabled = FALSE;
    u_result result;

    if (_this != NULL) {
        enabled = _this->enabled;
        if (!enabled) {
            result = u_entityReadClaim(_this, &ke);
            if(result == U_RESULT_OK)
            {
                enabled = v_entityEnabled(ke);
                _this->enabled = enabled;
                u_entityRelease(_this);
            } else {
                OS_REPORT_1(OS_ERROR, "u_entityEnabled", 0,
                            "Unable to enable entity, result code = %d", result);
            }
        }
    }
    return enabled;
}

u_result
u_entityAction(
    u_entity _this,
    void (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    u_result result;
    v_entity ke;

    result = u_entityReadClaim(_this, &ke);

    if(result == U_RESULT_OK){
        action(ke,arg);
        u_entityRelease(_this);
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_entityWriteAction(
    u_entity _this,
    void (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    u_result result;
    v_entity ke;

    result = u_entityWriteClaim(_this, &ke);

    if(result == U_RESULT_OK){
        action(ke,arg);
        u_entityRelease(_this);
        result = U_RESULT_OK;
    }
    return result;
}


u_result
u_entityWalkEntities(
    u_entity _this,
    c_bool (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    u_result result;
    v_entity ke;
    c_bool completeness;

    result = u_entityReadClaim(_this, &ke);
    if(result == U_RESULT_OK){
        completeness = v_entityWalkEntities(ke,action,arg);
        u_entityRelease(_this);
        if (completeness == TRUE) {
            result = U_RESULT_OK;
        } else {
            result = U_RESULT_INTERRUPTED;
        }
    } else {
        OS_REPORT_1(OS_ERROR,
                    "u_entityWalkEntities", 0,
                    "u_entityClaim failed: entity kind = %s",
                    u_kindImage(u_entityKind(_this)));
    }
    return result;
}

u_result
u_entityWalkDependantEntities(
    u_entity _this,
    c_bool (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    u_result result;
    v_entity ke;
    c_bool completeness;

    result = u_entityReadClaim(_this, &ke);

    if(result == U_RESULT_OK){
        completeness = v_entityWalkDependantEntities(ke,action,arg);
        u_entityRelease(_this);
        if (completeness == TRUE) {
            result = U_RESULT_OK;
        } else {
            result = U_RESULT_INTERRUPTED;
        }
    } else {
        if (_this) {
            OS_REPORT_1(OS_ERROR,
                        "u_entityWalkDependantEntities", 0,
                        "u_entityClaim failed: entity kind = %s",
                        u_kindImage(u_entityKind(_this)));
        }else {
            OS_REPORT(OS_ERROR,
                        "u_entityWalkDependantEntities", 0,
                        "u_entityClaim failed: entity = nil");
        }
    }
    return result;
}

c_type
u_entityResolveType(
    u_entity _this)
{
    v_entity ke;
    c_type type = NULL;
    u_result result;

    result = u_entityReadClaim(_this, &ke);
    if(result == U_RESULT_OK){
        switch(v_objectKind(ke)){
        case K_TOPIC:
            type = v_topicDataType(ke);
        break;
        default:
            type = c_getType((c_object)ke);
        break;
        }
        u_entityRelease(_this);
    } else {
        OS_REPORT(OS_ERROR, "u_entityResolveType", 0,
                  "u_entityClaim failed");
     }
    return type;
}

c_voidp
u_entityGetUserData(
    u_entity _this)
{
    if (_this == NULL) {
        return NULL;
    }
    if (!u_entityCheck(_this)) {
        OS_REPORT(OS_WARNING,
                  "u_entityGetUserData", 0,
                  "Invalid Entity detected");
        return NULL;
    }
    return _this->userData;
}

c_voidp
u_entitySetUserData(
    u_entity _this,
    c_voidp userData)
{
    c_voidp old = NULL;

    if (_this == NULL) {
        OS_REPORT(OS_WARNING,
                  "u_entitySetUserData",0,
                  "No entity specified for set userData");
        return NULL;
    } else {
        if (u_entityCheck(_this)) {
            old = _this->userData;
            _this->userData = userData;
            if (old && userData) {
                OS_REPORT_1(OS_WARNING,
                            "u_entitySetUserData", 0,
                            "Old value existed! and is now overwritten. "
                            "Participant = '%s', Entity = 0x%x",
                            u_entityName(u_entity(_this->participant)));
            }
        } else {
            OS_REPORT(OS_WARNING,
                      "u_entitySetUserData", 0,
                      "Invalid Entity detected");
        }
    }
    return old;
}



u_result
u_entityQoS(
    u_entity _this,
    v_qos *qos)
{
    v_entity ke;
    v_qos    vq;
    u_result result;

    if ((qos == NULL) || (_this == NULL)) {
        result = U_RESULT_ILL_PARAM;
    } else {
        result = u_entityReadClaim(_this, &ke);
        if (result == U_RESULT_OK) {

            vq = v_entityGetQos(ke);
            /* now copy qos */
            *qos = u_qosNew(vq);
            c_free(vq);
            u_entityRelease(_this);
            result = U_RESULT_OK;
        } else {
            OS_REPORT(OS_ERROR, "u_entityQoS", 0,
                     "u_entityClaim failed");
         }
    }
    return result;
}

u_result
u_entitySetQoS(
    u_entity _this,
    v_qos qos)
{
    v_entity ke;
    u_result result;

    if ((qos == NULL) || (_this == NULL)) {
        result = U_RESULT_ILL_PARAM;
    } else {
        result = u_entityWriteClaim(_this, &ke);
        if (result == U_RESULT_OK) {
            result = u_resultFromKernel(v_entitySetQos(ke,qos));
            u_entityRelease(_this);
        }
    }
    return result;
}

u_kind
u_entityKind(
    u_entity _this)
{
    assert(_this != NULL);

    return _this->kind;
}

v_gid
u_entityGid (
    u_entity _this)
{
    assert(_this != NULL);

    return _this->gid;
}

u_handle
u_entityHandle (
    u_entity _this)
{
    assert(_this != NULL);

    return _this->handle;
}

u_instanceHandle
u_entityGetInstanceHandle(
    u_entity _this)
{
    v_entity ke;
    u_instanceHandle handle = U_INSTANCEHANDLE_NIL;
    u_result result;

    if (_this) {
        result = u_entityReadClaim(_this, &ke);
        if (result == U_RESULT_OK) {
/* TODO : the handle retrieval is incorrect,
 *        must be retrieved from build-in reader.
 */
            handle = u_instanceHandleFromGID(v_publicGid(v_public(ke)));
            u_entityRelease(_this);
        } else {
            OS_REPORT_1(OS_ERROR, "u_entityGetInstanceHandle", 0,
                      "Invalid handle detected, result code %d", result);
        }
    }
    return handle;
}

c_long
u_entitySystemId(
    u_entity _this)
{
    c_long id;
    v_entity ke;
    u_result result;

    result = u_entityReadClaim(_this, &ke);

    if(result == U_RESULT_OK){
        id = u_userServerId(v_public(ke));
        u_entityRelease(_this);
    } else {
        id = 0;
        OS_REPORT(OS_ERROR, "u_entitySystemId", 0,
                  "Invalid handle detected");
    }
    return id;
}

c_char *
u_entityName(
    u_entity _this)
{
    c_char *name = NULL;
    v_entity ke;
    u_result result;

    result = u_entityReadClaim(_this, &ke);

    if(result == U_RESULT_OK){
        name = v_entityName(ke);
        if (name) {
            name = os_strdup(name);
        } else {
            name = os_strdup("No Name");
        }
        u_entityRelease(_this);
    } else {
        name = os_strdup("Invalid Entity");
    }
    return name;
}
