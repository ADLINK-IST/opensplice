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
#include "u__entity.h"
#include "u__handle.h"
#include "u__types.h"
#include "u__qos.h"
#include "u_partition.h"
#include "u_topic.h"
#include "u_publisher.h"
#include "u_subscriber.h"
#include "u__writer.h"
#include "u_reader.h"
#include "u_dataReader.h"
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
#include "v_dataReader.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"
#include "v_spliced.h"
#include "v_waitset.h"
#include "v_networkReader.h"
#include "v_groupQueue.h"
#include "v_dataView.h"

#include "os_report.h"

u_entity
u_entityNew(
    v_entity ke,
    u_participant p,
    c_bool owner)
{
    u_entity e = NULL;

    if (ke == NULL) {
        OS_REPORT(OS_ERROR, "u_entityNew", 0,
                  "No Kernel entity specified.");
        return NULL;
    }
#define _NEW_(t,l) \
    e = u_entity(os_malloc((os_uint32)sizeof(C_STRUCT(t)))); \
    if (e == NULL) { \
        OS_REPORT(OS_ERROR, "u_entityNew", 0, \
                  "Failed to allocate User proxy."); \
        return NULL; \
    } e->kind = l

    switch(v_objectKind(ke)) {
    case K_DOMAIN:         _NEW_(u_partition,U_PARTITION);                  break;
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
        u_participant(e)->kernel = u_participantKernel(p);
        p = u_participant(e);                                         break;
    case K_SERVICEMANAGER: _NEW_(u_serviceManager, U_SERVICEMANAGER); break;
    case K_NETWORKING:
    case K_DURABILITY:
    case K_CMSOAP:
    case K_SERVICE:        _NEW_(u_service, U_SERVICE);               break;
    case K_SPLICED:        _NEW_(u_spliced, U_SERVICE);               break;
    case K_WAITSET:        _NEW_(u_waitset, U_WAITSET);               break;
    case K_NETWORKREADER:  _NEW_(u_networkReader, U_NETWORKREADER);   break;
    case K_GROUPQUEUE:     _NEW_(u_groupQueue, U_GROUPQUEUE);         break;
    default:
        OS_REPORT_1(OS_ERROR, "u_entityNew", 0,
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

    assert(e != NULL);
    assert(ke != NULL);

    e->magic = v_objectKernel(ke);
    e->gid = v_publicGid(v_public(ke));
    e->handle = u_handleNew(v_public(ke));
    e->participant = p;
    e->userData = NULL;
    
    e->enabled = TRUE;

    e->flags = 0;
    if (owner) {
        e->flags |= U_ECREATE_OWNED;
        v_entitySetUserData(ke,e); /* only set user data when owned! */
    }
    os_mutexAttrInit(&mutexAttr);
    mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    os_mutexInit(&e->mutex,&mutexAttr);

    return U_RESULT_OK;
}

u_result
u_entityDeinit(
    u_entity e)
{
    v_entity o;
    u_result result;

    if (e->flags & U_ECREATE_OWNED) {
        o = u_entityClaim(e);

#define _FREE_(c) c##Free(c(o)); break

        if (o != NULL) {
            switch(v_objectKind(o)) {
            case K_PARTICIPANT:      _FREE_(v_participant);
            case K_PUBLISHER:        _FREE_(v_publisher);
            case K_SUBSCRIBER:       _FREE_(v_subscriber);
            case K_WRITER:           _FREE_(v_writer);
            case K_DATAREADER:       _FREE_(v_dataReader);
            case K_NETWORKREADER:    _FREE_(v_networkReader);
            case K_GROUPQUEUE:       _FREE_(v_groupQueue);
            case K_DATAVIEW:         _FREE_(v_dataView);
            case K_TOPIC:            _FREE_(v_topic);
            case K_DOMAIN:           _FREE_(v_partition);
            case K_GROUP:            _FREE_(v_group);
            case K_SERVICEMANAGER:   /* Is never freed! */
            break;
            case K_SPLICED:          _FREE_(v_spliced);
            case K_SERVICE:          _FREE_(v_service);
            case K_NETWORKING:       _FREE_(v_networking);
            case K_DURABILITY:       _FREE_(v_durability);
            case K_CMSOAP:           _FREE_(v_cmsoap);
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
                OS_REPORT(OS_ERROR, "u_entityFree",
                          0, "deinit of abstract class K_QUERY");
            break;
            case K_DATAVIEWQUERY:    _FREE_(v_dataViewQuery);

            case K_DATAREADERQUERY:  _FREE_(v_dataReaderQuery);
            case K_WAITSET:          _FREE_(v_waitset);
            case K_WRITERINSTANCE:
            case K_DATAREADERINSTANCE:
            break;
            default:
                OS_REPORT_1(OS_ERROR,"u_entityFree",0,
                            "illegal entity kind (%d) specified",
                            v_objectKind(o));
            break;
            }
            result = u_entityRelease(e);
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_ERROR,"u_entityFree",0,
                          "Internal error: release entity failed.");
            }
        }/* else 
          * This happens when an entity is freed and its u_participant
          * already has been freed. It means this case is a valid situation.
          */
        os_mutexDestroy(&e->mutex);
    }
#undef _FREE_

    return U_RESULT_OK;
}

u_result
u_entityFree(
    u_entity e)
{
    u_result r;
    u_kernel k;

    if (e == NULL) {
        r = U_RESULT_ILL_PARAM;
    } else {
        k = u_participantKernel(e->participant);
        if (k) {
            r = u_kernelProtect(k);
            if (r == U_RESULT_OK) {
                r = u_entityDeinit(e);
                os_free(e);
                if (r == U_RESULT_OK) {
                    r = u_kernelUnprotect(k);
                } else {
                    u_kernelUnprotect(k);
                }
            } else {
                OS_REPORT(OS_ERROR,"u_entityFree",0,
                          "Internal error: u_kernelProtect() failed.");
            }
        } else {
            OS_REPORT(OS_ERROR,"u_entityFree",0,
                      "Internal error: u_participantKernel() failed.");
        }
    }
    return r;
}

v_entity
u_entityClaim(
    u_entity e)
{
    v_entity ve = NULL;
    u_participant p;
    u_kernel k;
    u_result r;

    if (e != NULL) {
        if (e->participant == NULL) {
            p = u_participant(e);
        } else {
            p = e->participant;
        }
        k = u_participantKernel(p);
        if (k) {
            r = u_kernelProtect(k);
            if (r == U_RESULT_OK) {
                r = u_handleClaim(e->handle,(v_object *)&ve);
                if (r != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_entityClaim", 0,
                              "Illegal handle detected");
                    u_kernelUnprotect(k);
                }
            } else if (r == U_RESULT_DETACHING) {
                OS_REPORT(OS_WARNING, "u_entityClaim", 0,
                          "Claim failed because the process "
                          "is detaching from the kernel.");
            } else {
                OS_REPORT(OS_ERROR, "u_entityClaim", 0,
                          "u_kernelProtect() failed.");
            }
        } else {
            OS_REPORT(OS_ERROR,"u_entityClaim",0,
                      "u_participantKernel() failed.");
        }
    }
    return ve;
}

u_result
u_entityRelease(
    u_entity e)
{
    u_participant p;
    u_kernel k;
    u_result result = U_RESULT_OK;

    if (e != NULL) {
        result = u_handleRelease(e->handle);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_ERROR, "u_entityRelease", 0,
                      "Illegal handle detected");
        }
        if (e->participant == NULL) {
            p = u_participant(e);
        } else {
            p = e->participant;
        }
        k = u_participantKernel(p);
        if (k) {
            result = u_kernelUnprotect(k);
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, "u_entityRelease", 0,
                          "u_kernelUnprotect() failed.");
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "u_entityRelease", 0,
                  "Illegal parameter specified");
        result = U_RESULT_ILL_PARAM;
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
        osResult = os_mutexLock(&e->mutex);
        if (osResult == os_resultSuccess) {
            result = U_RESULT_OK;
        } else {
            OS_REPORT(OS_ERROR, "u_entityLock", 0,
                      "Illegal mutex detected");
            result = U_RESULT_ILL_PARAM;
        }
    } else {
        OS_REPORT(OS_ERROR, "u_entityLock", 0,
                  "Illegal parameter specified");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_entityUnlock(
    u_entity e)
{
    os_result osResult;
    u_result result;

    if (e != NULL) {
        osResult = os_mutexUnlock(&e->mutex);
        if (osResult == os_resultSuccess) {
            result = U_RESULT_OK;
        } else {
            OS_REPORT(OS_ERROR, "u_entityUnlock", 0,
                      "Illegal mutex detected");
            result = U_RESULT_ILL_PARAM;
        }
    } else {
        OS_REPORT(OS_ERROR, "u_entityUnlock", 0,
                  "Illegal parameter specified");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

void
u_entityEnable(
    u_entity e)
{
    v_entity ke;
    e->enabled = TRUE;

    ke = u_entityClaim(e);
    if (ke != NULL) {
        v_entityEnable(ke);
        u_entityRelease(e);
    } else {
        OS_REPORT(OS_ERROR, "u_entityEnable", 0,
                    "Unable to enable entity");
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
    c_bool result = FALSE;

    if (_this != NULL) {
        result = _this->enabled;
    }
    return result;
}

u_result
u_entityAction(
    u_entity _this,
    void (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    u_result result;
    v_entity ke;

    ke = u_entityClaim(_this);

    if(ke != NULL){
        action(ke,arg);
        u_entityRelease(_this);
        result = U_RESULT_OK;
    } else {
        result = U_RESULT_ILL_PARAM;
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

    ke = u_entityClaim(_this);

    if(ke != NULL){
        completeness = v_entityWalkEntities(ke,action,arg);
        u_entityRelease(_this);
        if (completeness == TRUE) {
            result = U_RESULT_OK;
        } else {
            result = U_RESULT_INTERRUPTED;
        }
    } else {
        result = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_entityWalkEntities", 0,
                  "u_entityClaim failed");
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

    ke = u_entityClaim(_this);

    if(ke != NULL){
        completeness = v_entityWalkDependantEntities(ke,action,arg);
        u_entityRelease(_this);
        if (completeness == TRUE) {
            result = U_RESULT_OK;
        } else {
            result = U_RESULT_INTERRUPTED;
        }
    } else {
        result = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_entityWalkDependantEntities", 0,
                  "u_entityClaim failed");
    }
    return result;
}

c_type
u_entityResolveType(
    u_entity _this)
{
    v_entity ke;
    c_type type = NULL;

    ke = u_entityClaim(_this);
    if(ke != NULL){
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
    return _this->userData;
}

c_voidp
u_entitySetUserData(
    u_entity _this,
    c_voidp userData)
{
    c_voidp old = NULL;

    if (_this == NULL) {
        OS_REPORT(OS_ERROR,"User Entity",0,
                  "No entity specified for set userData");
        return NULL;
    } else {
        old = _this->userData;
        _this->userData = userData;
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
        ke = u_entityClaim(_this);
        if (ke != NULL) {
            vq = v_entityGetQos(ke);
            /* now copy qos */
            *qos = u_qosNew(vq);
            c_free(vq);
            u_entityRelease(_this);
            result = U_RESULT_OK;
        } else {
            OS_REPORT(OS_ERROR, "u_entityQoS", 0,
                     "u_entityClaim failed");
            result = U_RESULT_ILL_PARAM;
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
        ke = u_entityClaim(_this);
        if (ke != NULL) {
            result = u_resultFromKernel(v_entitySetQos(ke,qos));
            u_entityRelease(_this);
        } else {
            OS_REPORT(OS_ERROR, "u_entitySetQoS", 0,
                      "u_entityClaim failed");
            result = U_RESULT_ILL_PARAM;
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

    if (_this) {
        ke = u_entityClaim(_this);
        if (ke != NULL) {
/* TODO : the handle retrieval is incorrect,
 *        must be retrieved from build-in reader.
 */
            handle = u_instanceHandleFromGID(v_publicGid(v_public(ke)));
            u_entityRelease(_this);
        } else {
            OS_REPORT(OS_ERROR, "u_entityGetInstanceHandle", 0,
                      "Illegal handle detected");
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

    ke = u_entityClaim(_this);

    if(ke != NULL){
        id = u_userServerId(v_public(ke));
        u_entityRelease(_this);
    } else {
        id = 0;
        OS_REPORT(OS_ERROR, "u_entitySystemId", 0,
                  "Illegal handle detected");
    }
    return id;
}
