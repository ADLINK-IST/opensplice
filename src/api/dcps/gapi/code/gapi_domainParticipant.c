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
#include "gapi_entity.h"
#include "gapi_kernel.h"
#include "gapi_domainEntity.h"
#include "gapi_publisher.h"
#include "gapi_subscriber.h"
#include "gapi_builtin.h"
#include "gapi_qos.h"
#include "gapi_map.h"
#include "gapi_set.h"
#include "gapi_structured.h"
#include "gapi_topic.h"
#include "gapi_topicDescription.h"
#include "gapi_contentFilteredTopic.h"
#include "gapi_typeSupport.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_domainParticipantStatus.h"
#include "gapi_waitSet.h"
#include "gapi_objManag.h"
#include "gapi_error.h"
#include "gapi_scheduler.h"

#include "os.h"
#include "os_report.h"

#include "u_user.h"
#include "u_waitsetEvent.h"

#define U_PARTICIPANT_GET(t)       u_participant(U_ENTITY_GET(t))
#define U_PARTICIPANT_SET(t,e)     _EntitySetUserEntity(_Entity(t), u_entity(e))

/* value to represent the platform dependent default */
#define GAPI_DEFAULT_LISTENER_STACKSIZE 0


typedef struct DeleteActionInfo_s {
    gapi_deleteEntityAction action;
    void                    *argument;
} DeleteActionInfo;

typedef struct ListenerThreadInfo_s {
    os_mutex                  mutex;
    os_threadId               threadId;
    u_waitset                 waitset;
    gapi_boolean              running;
    gapi_listenerThreadAction startAction;
    gapi_listenerThreadAction stopAction;
    void                      *actionArg;
    gapi_set                  toAddList;
    gapi_schedulingQosPolicy  scheduling;
    os_uint32                 stackSize;
} ListenerThreadInfo;

C_STRUCT(_DomainParticipant) {
    C_EXTENDS(_Entity);
    gapi_domainId_t                        _DomainId;
    gapi_publisherQos                      _defPublisherQos;
    gapi_subscriberQos                     _defSubscriberQos;
    gapi_topicQos                          _defTopicQos;
    struct gapi_domainParticipantListener  _Listener;
    _DomainParticipantFactory              _Factory;
    gapi_map                                typeSupportMap;
    gapi_set                                topicDescriptionSet;
    gapi_map                                builtinTopicMap;
    gapi_set                                publisherSet;
    gapi_set                                subscriberSet;
    void *                                  userData;
    _Subscriber                             builtinSubscriber;
    DeleteActionInfo                        deleteActionInfo;
    ListenerThreadInfo                      listenerThreadInfo;
    gapi_schedulingQosPolicy                watchdogScheduling;
};

typedef struct {
    gapi_long         topic_create_count;
    _TopicDescription topic_description;
} topicInfo;

static gapi_boolean
startListenerEventThread (
    _DomainParticipant participant);

static gapi_boolean
stopListenerEventThread (
    _DomainParticipant participant);

static gapi_boolean
copyParticipantQosIn (
    const gapi_domainParticipantQos *srcQos,
    v_participantQos dstQos)
{
    gapi_boolean copied = FALSE;

    if (dstQos->userData.value) {
        os_free(dstQos->userData.value);
        dstQos->userData.value = NULL;
    }
    dstQos->userData.size = srcQos->user_data.value._length;
    if (dstQos->userData.size) {
        dstQos->userData.value = os_malloc (dstQos->userData.size);
    }
    if ((srcQos->user_data.value._length == 0) || dstQos->userData.value) {
        memcpy(dstQos->userData.value,
               srcQos->user_data.value._buffer,
               srcQos->user_data.value._length);
        copied = TRUE;
    }
    dstQos->entityFactory.autoenable_created_entities =
            srcQos->entity_factory.autoenable_created_entities;
    gapi_scheduleToKernel (&srcQos->watchdog_scheduling,
                           &dstQos->watchdogScheduling);

    return copied;
}


static gapi_boolean
copyParticipantQosOut (
    const v_participantQos  srcQos,
    gapi_domainParticipantQos *dstQos)
{
    assert(srcQos);
    assert(dstQos);

    if ( dstQos->user_data.value._maximum > 0 ) {
        if ( dstQos->user_data.value._release ) {
            gapi_free(dstQos->user_data.value._buffer);
        }
    }

    if ( (srcQos->userData.size > 0) && srcQos->userData.value ) {
        dstQos->user_data.value._buffer = gapi_octetSeq_allocbuf(srcQos->userData.size);
        if ( dstQos->user_data.value._buffer ) {
            dstQos->user_data.value._maximum = srcQos->userData.size;
            dstQos->user_data.value._length  = srcQos->userData.size;
            dstQos->user_data.value._release = TRUE;
            memcpy(dstQos->user_data.value._buffer,
                   srcQos->userData.value,
                   srcQos->userData.size);
        }
    } else {
            dstQos->user_data.value._maximum = 0;
            dstQos->user_data.value._length  = 0;
            dstQos->user_data.value._release = FALSE;
            dstQos->user_data.value._buffer = NULL;
    }

    dstQos->entity_factory.autoenable_created_entities =
            srcQos->entityFactory.autoenable_created_entities;

    gapi_scheduleFromKernel (&srcQos->watchdogScheduling,
                             &dstQos->watchdog_scheduling);

    return TRUE;
}

static _DomainParticipant
allocateParticipant (
    void)
{
    _DomainParticipant participant = _DomainParticipantAlloc();

    if ( participant != NULL ) {
        participant->typeSupportMap = gapi_mapNew (gapi_stringCompare, TRUE, FALSE);
        participant->topicDescriptionSet = gapi_setNew (gapi_objectRefCompare);
        participant->builtinTopicMap = gapi_mapNew (gapi_stringCompare, FALSE, FALSE);
        participant->publisherSet = gapi_setNew (gapi_objectRefCompare);
        participant->subscriberSet = gapi_setNew (gapi_objectRefCompare);
        participant->builtinSubscriber = _Subscriber (0);

        if ( (participant->typeSupportMap      == NULL) ||
             (participant->topicDescriptionSet == NULL) ||
             (participant->builtinTopicMap     == NULL) ||
             (participant->publisherSet        == NULL) ||
             (participant->subscriberSet       == NULL) ) {
            if ( participant->typeSupportMap != NULL ) {
                gapi_mapFree(participant->typeSupportMap);
            }
            if ( participant->topicDescriptionSet != NULL ) {
                gapi_setFree(participant->topicDescriptionSet);
            }
            if ( participant->builtinTopicMap != NULL ) {
                gapi_mapFree(participant->builtinTopicMap);
            }
            if ( participant->publisherSet != NULL ) {
                gapi_setFree(participant->publisherSet);
            }
            if ( participant->subscriberSet != NULL ) {
                gapi_setFree(participant->subscriberSet);
            }
            _EntityDelete(participant);
            participant = NULL;
        }
    } else {
        _EntityDelete(participant);
        participant = NULL;
    }

    return participant;
}


static void
deallocateParticipant (
    _DomainParticipant participant)
{
    if ( participant->_DomainId ) {
        os_free(participant->_DomainId);
    }
    gapi_publisherQos_free(&participant->_defPublisherQos);
    gapi_subscriberQos_free(&participant->_defSubscriberQos);
    gapi_topicQos_free(&participant->_defTopicQos);
    gapi_mapFree(participant->typeSupportMap);
    gapi_setFree(participant->topicDescriptionSet);
    gapi_mapFree(participant->builtinTopicMap);
    gapi_setFree(participant->publisherSet);
    gapi_setFree(participant->subscriberSet);
    _EntityDispose(_Entity(participant));
}


static u_cfData
configurationResolveParameter(
    u_cfElement e,
    const c_char* xpathExpr)
{
    u_cfData result;
    c_iter nodes;
    u_cfNode tmp;

    result = NULL;

    if (e != NULL) {
        nodes = u_cfElementXPath(e, xpathExpr);
        tmp = u_cfNode(c_iterTakeFirst(nodes));

        if (tmp != NULL) {
            if (u_cfNodeKind(tmp) == V_CFDATA) {
                result = u_cfData(tmp);
            } else {
                u_cfNodeFree(tmp);
            }
            tmp = u_cfNode(c_iterTakeFirst(nodes));
        }

        while (tmp != NULL) {
            u_cfNodeFree(tmp);
            tmp = u_cfNode(c_iterTakeFirst(nodes));
        }
        c_iterFree(nodes);
    }
    return result;
}


static gapi_boolean
initListenerThreadInfo (
    ListenerThreadInfo *info,
    u_participant uParticipant,
    gapi_listenerThreadAction threadStartAction,
    gapi_listenerThreadAction threadStopAction,
    void *actionArg,
    const gapi_domainParticipantQos *qos
)
{
    gapi_boolean initialized = FALSE;
    os_result osResult;
    os_mutexAttr osMutexAttr;
    u_cfElement cfg;
    u_cfData cfg_data;
    c_ulong cfg_value;

    memset(info, 0, sizeof(ListenerThreadInfo));

    osResult = os_mutexAttrInit (&osMutexAttr);
    osMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    osResult = os_mutexInit (&info->mutex, &osMutexAttr);

    if ( osResult == os_resultSuccess ) {
        info->threadId     = 0;
        info->running      = FALSE;
        info->waitset      = NULL;
        info->startAction = threadStartAction;
        info->stopAction  = threadStopAction;
        info->actionArg   = actionArg;
        info->scheduling  = qos->listener_scheduling;
        info->toAddList   = gapi_setNew(gapi_objectRefCompare);
        info->stackSize   = GAPI_DEFAULT_LISTENER_STACKSIZE;

        if ( info->toAddList ) {
            info->waitset = u_waitsetNew(uParticipant);
            if ( info->waitset ) {
                initialized = TRUE;
            }
        } else {
            gapi_setFree(info->toAddList);
        }

        /* read stackSize from configuration */
        cfg = u_participantGetConfiguration(uParticipant);

        if(cfg){
            cfg_data = configurationResolveParameter(cfg,"Domain/Listeners/StackSize/#text");
            if (cfg_data != NULL) {
                if (u_cfDataULongValue(cfg_data, &cfg_value)) {
                    info->stackSize = cfg_value;
                }
                u_cfDataFree(cfg_data);
            }
            u_cfElementFree(cfg);
        }



    }

    return initialized;
}

static void
deinitListenerThreadInfo (
    ListenerThreadInfo *info)
{
    if ( info->waitset ) {
        u_waitsetFree(info->waitset);
    }

    if ( info->toAddList ) {
        gapi_setFree(info->toAddList);
    }
    os_mutexDestroy (&info->mutex);
}

_DomainParticipant
_DomainParticipantNew (
    gapi_domainId_t                              domainId,
    const gapi_domainParticipantQos             *qos,
    const struct gapi_domainParticipantListener *a_listener,
    const gapi_statusMask                        mask,
    _DomainParticipantFactory                    theFactory,
    gapi_listenerThreadAction                    startAction,
    gapi_listenerThreadAction                    stopAction,
    void                                        *actionArg,
    const gapi_context                          *context)
{
    _DomainParticipant newParticipant;
    v_participantQos participantQos = NULL;
    u_participant uParticipant;
    char participantId[256];
    c_bool enable = TRUE;

    newParticipant = allocateParticipant();

    if (newParticipant != NULL) {
        _EntityInit (_Entity(newParticipant), NULL, NULL, enable);
        if ( domainId ) {
            newParticipant->_DomainId = gapi_strdup(domainId);
        } else {
            newParticipant->_DomainId = NULL;
        }
        memset (&newParticipant->_defTopicQos, 0,
                sizeof(newParticipant->_defTopicQos));
        gapi_topicQosCopy (&gapi_topicQosDefault,
                           &newParticipant->_defTopicQos);
        memset (&newParticipant->_defPublisherQos, 0,
                sizeof(newParticipant->_defPublisherQos));
        gapi_publisherQosCopy (&gapi_publisherQosDefault,
                               &newParticipant->_defPublisherQos);
        memset (&newParticipant->_defSubscriberQos, 0,
                sizeof(newParticipant->_defSubscriberQos));
        gapi_subscriberQosCopy (&gapi_subscriberQosDefault,
                                &newParticipant->_defSubscriberQos);
        if ( a_listener ) {
            newParticipant->_Listener = *a_listener;
        } else {
            memset(&newParticipant->_Listener, 0,
                   sizeof(newParticipant->_Listener));
        }
        newParticipant->_Factory = theFactory;

        participantQos = u_participantQosNew(NULL);
        if ( participantQos != NULL ) {
            if ( !copyParticipantQosIn(qos, participantQos) ) {
                gapi_errorReport(context, GAPI_ERRORCODE_OUT_OF_RESOURCES);
                deallocateParticipant(newParticipant);
                newParticipant = NULL;
            }
        } else {
            gapi_errorReport(context, GAPI_ERRORCODE_OUT_OF_RESOURCES);
            deallocateParticipant(newParticipant);
            newParticipant = NULL;
        }
    } else {
        gapi_errorReport(context, GAPI_ERRORCODE_OUT_OF_RESOURCES);
    }

    if (newParticipant != NULL) {
        snprintf (participantId, sizeof (participantId), "DPCS Appl %d_%d_"PA_ADDRFMT,
            (int)os_procIdSelf(), (int)os_threadIdSelf(), (PA_ADDRCAST)newParticipant);
        uParticipant = u_participantNew (domainId, 1, participantId, (v_qos)participantQos, enable);
        if ( uParticipant != NULL ) {
             U_PARTICIPANT_SET(newParticipant, uParticipant);
        } else {
            gapi_errorReport(context, GAPI_ERRORCODE_CREATION_KERNEL_ENTITY_FAILED);
            deallocateParticipant(newParticipant);
            newParticipant = NULL;
        }
    }

    if (newParticipant != NULL) {
        newParticipant->watchdogScheduling = qos->watchdog_scheduling;
        if (!initListenerThreadInfo(&newParticipant->listenerThreadInfo,
                                            uParticipant, startAction, stopAction, actionArg, qos)) {
            gapi_errorReport(context, GAPI_ERRORCODE_OUT_OF_RESOURCES);
            u_participantFree(uParticipant);
            deallocateParticipant(newParticipant);
            newParticipant = NULL;
        }
    }

    if (newParticipant != NULL) {
        _EntityStatus(newParticipant) =
            _Status(_DomainParticipantStatusNew(newParticipant,
                                                a_listener,
                                                mask));
        if ( !_EntityStatus(newParticipant) ) {
            gapi_errorReport(context, GAPI_ERRORCODE_OUT_OF_RESOURCES);
            stopListenerEventThread(newParticipant);
            deinitListenerThreadInfo(&newParticipant->listenerThreadInfo);
            u_participantFree(uParticipant);
            deallocateParticipant(newParticipant);
            newParticipant = NULL;
        }
    }
    u_participantQosFree(participantQos);

    return newParticipant;
}

/*
 * Precondition: _DomainParticipantPrepareDelete returned TRUE
 */
gapi_returnCode_t
_DomainParticipantFree (
    _DomainParticipant participant)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_mapIter iterMap;

    assert (participant);

    if (participant->builtinSubscriber != NULL ) {
        _EntityClaim(participant->builtinSubscriber);
        _BuiltinSubscriberFree(participant->builtinSubscriber);
        participant->builtinSubscriber = NULL;
    }

    _DomainParticipantStatusSetListener(_DomainParticipantStatus(_Entity(participant)->status), NULL, 0);
    _DomainParticipantStatusFree(_DomainParticipantStatus(_Entity(participant)->status));

    iterMap = gapi_mapFirst (participant->typeSupportMap);
    while (gapi_mapIterObject(iterMap)) {
        _TypeSupport ts = (_TypeSupport)gapi_mapIterObject(iterMap);
        gapi_free(_EntityHandle(ts));
        gapi_mapIterRemove(iterMap);
    }
    gapi_mapIterFree (iterMap);

    iterMap = gapi_mapFirst(participant->builtinTopicMap);
    while (gapi_mapIterObject(iterMap)) {
        _Topic topic = (_Topic)gapi_mapIterObject(iterMap);
        _EntityClaim(topic);
        _TopicPrepareDelete(topic);
        _TopicFree(topic);
        gapi_mapIterRemove(iterMap);
    }
    gapi_mapIterFree (iterMap);

    _EntityFreeStatusCondition(_Entity(participant));

    stopListenerEventThread(participant);
    deinitListenerThreadInfo(&participant->listenerThreadInfo);

    u_participantFree (U_PARTICIPANT_GET(participant));
    U_PARTICIPANT_SET(participant, NULL);

    gapi_mapFree (participant->typeSupportMap);
    participant->typeSupportMap = NULL;
    gapi_setFree (participant->topicDescriptionSet);
    participant->topicDescriptionSet = NULL;
    gapi_mapFree (participant->builtinTopicMap);
    participant->builtinTopicMap = NULL;
    gapi_setFree (participant->publisherSet);
    participant->publisherSet = NULL;
    gapi_setFree (participant->subscriberSet);
    participant->subscriberSet = NULL;

    gapi_publisherQos_free(&participant->_defPublisherQos);
    gapi_subscriberQos_free(&participant->_defSubscriberQos);
    gapi_topicQos_free(&participant->_defTopicQos);

    if ( participant->_DomainId ) {
        os_free(participant->_DomainId);
    }

    _EntityDispose (_Entity(participant));

    return result;
}

gapi_boolean
_DomainParticipantPrepareDelete (
    _DomainParticipant  participant,
    const gapi_context *context)
{
    gapi_boolean result = TRUE;

    assert (participant);
    assert (_ObjectGetKind(_Object(participant)) == OBJECT_KIND_DOMAINPARTICIPANT);

    if (!gapi_setIsEmpty (participant->topicDescriptionSet)) {
        OS_REPORT(OS_ERROR,
                  "_DomainParticipantPrepareDelete", 0,
                  "DomainParticipantFactory_delete_participant failed:\n"
                  "              Some Topic descriptions exists");
        result = FALSE;
    }
    if (!gapi_setIsEmpty(participant->publisherSet)) {
        OS_REPORT(OS_ERROR,
                  "_DomainParticipantPrepareDelete", 0,
                  "DomainParticipantFactory_delete_participant failed:\n"
                  "              Some Publishers exists");
        result = FALSE;
    }
    if (!gapi_setIsEmpty (participant->subscriberSet)) {
        OS_REPORT(OS_ERROR,
                  "_DomainParticipantPrepareDelete", 0,
                  "DomainParticipantFactory_delete_participant failed:\n"
                  "              Some Subscribers exists");
        result = FALSE;
    }
    if (result == FALSE) {
        gapi_errorReport(context, GAPI_ERRORCODE_CONTAINS_ENTITIES);
    }

    return result;
}

/* precondition: participant must be locked */
_TypeSupport
_DomainParticipantFindType (
    _DomainParticipant participant,
    const char *registry_name)
{
    _TypeSupport typeSupport = NULL;
    gapi_mapIter iter;

    assert(participant);
    assert(registry_name);

    iter = gapi_mapFind (participant->typeSupportMap, (gapi_object)registry_name);
    if (iter) {
        typeSupport = (_TypeSupport)gapi_mapIterObject (iter);
        gapi_mapIterFree (iter);
    }
    return typeSupport;
}

u_participant
_DomainParticipantUparticipant (
    _DomainParticipant participant)
{
    u_participant upart = NULL;

    assert(participant);

    upart = U_PARTICIPANT_GET(participant);
    return upart;
}

_Topic
_DomainParticipantFindBuiltinTopic (
    _DomainParticipant participant,
    const gapi_char   *topic_name)
{
    gapi_mapIter iter;
    _Topic topic = NULL;

    assert(participant);
    assert(topic_name);

    iter = gapi_mapFind(participant->builtinTopicMap, (gapi_object)topic_name);
    if (iter) {
        topic = _Topic(gapi_mapIterObject(iter));
        gapi_mapIterFree (iter);
    }

    return topic;
}

/* precondition: participant must be locked */
gapi_returnCode_t
_DomainParticipantRegisterType (
    _DomainParticipant participant,
    _TypeSupport typeSupport,
    const gapi_char *registryName)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    const BuiltinTopicTypeInfo *builtinInfo;
    _Topic topic;

    assert(participant);
    assert(typeSupport);
    assert(registryName);

    result = gapi_mapAdd(participant->typeSupportMap,
                        (gapi_object)gapi_strdup(registryName),
                        (gapi_object)typeSupport);
    if ( result == GAPI_RETCODE_OK ) {
        builtinInfo = _BuiltinTopicFindTypeInfoByType(typeSupport->type_name);
        if ( builtinInfo ) {
            topic = _DomainParticipantFindBuiltinTopic(participant, builtinInfo->topicName);
            if ( !topic ) {
                topic = _BuiltinTopicNew(participant, builtinInfo->topicName, typeSupport->type_name);
                if ( topic ) {
                    result = gapi_mapAdd(participant->builtinTopicMap,
                                (gapi_object)builtinInfo->topicName,
                                (gapi_object)topic);

                    if ( participant->deleteActionInfo.action ) {
                        _TopicSetDeleteAction(topic,
                                              participant->deleteActionInfo.action,
                                              participant->deleteActionInfo.argument);
                    }
                    _EntityRelease(topic);
                    if ( result != GAPI_RETCODE_OK ) {
                        _TopicFree(topic);
                        result = GAPI_RETCODE_ERROR;
                    }
                } else {
                    result = GAPI_RETCODE_ERROR;
                }
            }
        }
    }

    return result;
}

/* precondition: participant must be locked */
_TypeSupport
_DomainParticipantFindTypeSupport (
    _DomainParticipant participant,
    const gapi_char *type_name)
{
    gapi_mapIter iter;
    _TypeSupport typeSupport = NULL;
    char *typeSupportTypeName;

    assert(participant);
    assert(type_name);

    iter = gapi_mapFirst (participant->typeSupportMap);
    if (iter) {
        typeSupport = _TypeSupport(gapi_mapIterObject (iter));
        if (typeSupport) {
            typeSupportTypeName = _TypeSupportTypeName (typeSupport);
        }
        while (typeSupport && typeSupportTypeName && (strcmp (type_name, typeSupportTypeName) != 0)) {
            gapi_mapIterNext (iter);
            typeSupport = _TypeSupport(gapi_mapIterObject (iter));
            if (typeSupport) {
                typeSupportTypeName = _TypeSupportTypeName (_TypeSupport(typeSupport));
            }
        }
        gapi_mapIterFree(iter);
    }
    return typeSupport;
}

gapi_boolean
_DomainParticipantContainsTypeSupport (
    _DomainParticipant participant,
    _TypeSupport       typeSupport)
{
    gapi_mapIter iter;
    _TypeSupport ts;
    gapi_boolean contains = FALSE;

    assert(participant);
    assert(typeSupport);

    iter = gapi_mapFirst (participant->typeSupportMap);
    if ( iter ) {
        ts = _TypeSupport(gapi_mapIterObject(iter));
        while ( ts ) {
            if ( _TypeSupportEquals(typeSupport, ts) ) {
                contains = TRUE;
            }
            gapi_mapIterNext (iter);
            ts = _TypeSupport(gapi_mapIterObject(iter));
        }
        gapi_mapIterFree(iter);
    }

    return contains;
}

_TypeSupport
_DomainParticipantFindRegisteredTypeSupport (
    _DomainParticipant participant,
    _TypeSupport       typeSupport)
{
    _TypeSupport registered = NULL;
    _TypeSupport ts;
    gapi_mapIter iter;

    assert(participant);
    assert(typeSupport);

    iter = gapi_mapFirst (participant->typeSupportMap);
    if ( iter ) {
        ts = _TypeSupport(gapi_mapIterObject(iter));
        while ( !registered && ts ) {
            if ( _TypeSupportEquals(typeSupport, ts) ) {
                registered = ts;
            }
            gapi_mapIterNext (iter);
            ts = _TypeSupport(gapi_mapIterObject(iter));
        }
        gapi_mapIterFree(iter);
    }

    return registered;
}

const gapi_char *
_DomainParticipantGetRegisteredTypeName (
    _DomainParticipant participant,
    _TypeSupport       typeSupport)
{
    gapi_mapIter iter;
    _TypeSupport t;
    const gapi_char *name = NULL;

    iter = gapi_mapFirst(participant->typeSupportMap);
    if ( iter ) {
        t = (_TypeSupport)gapi_mapIterObject(iter);
        while ( !name && t ) {
            if ( t == typeSupport ) {
                name = (const gapi_char *) gapi_mapIterKey(iter);
            } else {
                gapi_mapIterNext(iter);
                t = (_TypeSupport)gapi_mapIterObject(iter);
            }
        }
        gapi_mapIterFree(iter);
    }

    return name;
}

/* precondition: participant must be locked */
_TopicDescription
_DomainParticipantFindTopicDescription (
    _DomainParticipant participant,
    const gapi_char *topic_name)
{
    gapi_setIter it1;
    gapi_mapIter it2;
    _TopicDescription topicDescription = NULL;
    _TopicDescription td;

    assert(participant);
    assert(topic_name);

    it1 = gapi_setFirst(participant->topicDescriptionSet);
    if ( it1 ) {
        td = (_TopicDescription)gapi_setIterObject(it1);
        while ( td && !topicDescription ) {
            if ( strcmp(td->topic_name, topic_name) == 0 ) {
                 topicDescription = td;
            } else {
                gapi_setIterNext(it1);
                td = (_TopicDescription)gapi_setIterObject(it1);
            }
        }
        gapi_setIterFree(it1);
    }

    if ( !topicDescription ) {
        it2 = gapi_mapFind(participant->builtinTopicMap, (gapi_object)topic_name);
        if ( it2 ) {
            topicDescription = _TopicDescription(gapi_mapIterObject(it2));
            gapi_mapIterFree(it2);
        }
    }

    return topicDescription;
}

gapi_boolean
_DomainParticipantTopicDescriptionExists (
    _DomainParticipant participant,
    _TopicDescription  topicDescription)
{
    gapi_setIter it1;
    gapi_mapIter it2;
    gapi_boolean      result = FALSE;

    assert(participant);
    assert(topicDescription);

    it1 = gapi_setFind(participant->topicDescriptionSet,
                        (gapi_object)topicDescription);
    if ( it1 ) {
        result = TRUE;
        gapi_setIterFree(it1);
    }

    if ( !result ) {
        it2 = gapi_mapFind(participant->builtinTopicMap,
                (gapi_object)topicDescription->topic_name);
        if ( it2 ) {
            result = TRUE;
            gapi_mapIterFree(it2);
        }
    }

    return result;
}

gapi_domainParticipantQos *
_DomainParticipantGetQos (
    _DomainParticipant participant,
    gapi_domainParticipantQos * qos)
{
    v_participantQos participantQos;
    u_participant uParticipant;

    assert(participant);

    uParticipant = U_PARTICIPANT_GET(participant);

    if ( u_entityQoS(u_entity(uParticipant), (v_qos*)&participantQos) == U_RESULT_OK ) {
        copyParticipantQosOut(participantQos,  qos);
        u_participantQosFree(participantQos);

        qos->watchdog_scheduling = participant->watchdogScheduling;
        qos->listener_scheduling = participant->listenerThreadInfo.scheduling;
    }

    return qos;
}

/*     Publisher
 *     create_publisher(
 *         in PublisherQos qos,
 *         in PublisherListener a_listener);
 *
 *      Function will operate indepenedent of the enable flag
 */
gapi_publisher
gapi_domainParticipant_create_publisher (
    gapi_domainParticipant _this,
    const gapi_publisherQos *qos,
    const struct gapi_publisherListener *a_listener,
    const gapi_statusMask mask)
{
    _DomainParticipant participant = (_DomainParticipant)_this;
    _Publisher publisher = NULL;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_PUBLISHER);

    participant = gapi_domainParticipantClaim(_this, NULL);

    if ( participant != NULL ) {
        if ( qos == GAPI_PUBLISHER_QOS_DEFAULT ) {
            qos = &participant->_defPublisherQos;
        }
        if ( gapi_publisherQosIsConsistent(qos, &context) == GAPI_RETCODE_OK ) {
            publisher = _PublisherNew(U_PARTICIPANT_GET(participant), qos, a_listener, mask, participant);
            if ( publisher != NULL ) {
                if ( gapi_setAdd (participant->publisherSet, (gapi_object)publisher) == GAPI_RETCODE_OK ) {
                    _ENTITY_REGISTER_OBJECT(_Entity(participant), (_Object)publisher);
                } else {
                    _PublisherFree (publisher);
                    publisher = NULL;
                }
            }
        }
    }

    _EntityRelease(participant);

    return (gapi_publisher)_EntityRelease(publisher);
}

/*     ReturnCode_t
 *     delete_publisher(
 *         in Publisher p);
 */
gapi_returnCode_t
gapi_domainParticipant_delete_publisher (
    gapi_domainParticipant _this,
    const gapi_publisher p)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;
    _Publisher publisher = NULL;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant ) {
        publisher = gapi_publisherClaimNB(p, NULL);
    }

    if ( publisher ) {
        gapi_setIter publisherIter;

        publisherIter = gapi_setFind (participant->publisherSet, (gapi_object)publisher);
        if ( publisherIter != NULL ) {
            if (gapi_setIterObject(publisherIter) != NULL) {
                if (_PublisherPrepareDelete (publisher)) {
                    gapi_setRemove (participant->publisherSet, (gapi_object)publisher);
                    result = _PublisherFree (publisher);
                    if ( result == GAPI_RETCODE_OK ) {
                        publisher = NULL;
                    }
                } else {
                    /* publisher still contains datareaders */
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                /* publisher not created by this participant */
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            gapi_setIterFree (publisherIter);
        } else {
            /* Iterator could not be allocated */
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    } else {
        /* publisher is not valid */
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    _EntityRelease(publisher);
    _EntityRelease(participant);

    return result;
}

/*     Subscriber
 *     create_subscriber(
 *         in SubscriberQos qos,
 *         in SubscriberListener a_listener);
 *
 * Function will operate indepenedent of the enable flag
 */
gapi_subscriber
gapi_domainParticipant_create_subscriber (
    gapi_domainParticipant _this,
    const gapi_subscriberQos *qos,
    const struct gapi_subscriberListener *a_listener,
    const gapi_statusMask mask)
{
    _DomainParticipant participant = (_DomainParticipant)_this;
    _Subscriber subscriber = NULL;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_SUBSCRIBER);

    participant = gapi_domainParticipantClaim(_this, NULL);

    if ( participant != NULL ) {
        if (qos == GAPI_SUBSCRIBER_QOS_DEFAULT) {
            qos = &participant->_defSubscriberQos;
        }
        if ( gapi_subscriberQosIsConsistent(qos, &context) == GAPI_RETCODE_OK ) {
            subscriber = _SubscriberNew(U_PARTICIPANT_GET(participant), qos, a_listener,mask, participant);
            if (subscriber != NULL) {
                if (gapi_setAdd (participant->subscriberSet, (gapi_object)subscriber) == GAPI_RETCODE_OK) {
                    _ENTITY_REGISTER_OBJECT(_Entity(participant), (_Object)subscriber);
                } else {
                    _SubscriberFree (subscriber);
                    subscriber = NULL;
                }
            }
        }
    }

    _EntityRelease(participant);

    return (gapi_subscriber)_EntityRelease(subscriber);
}

/*     ReturnCode_t
 *     delete_subscriber(
 *         in Subscriber s);
 */
gapi_returnCode_t
gapi_domainParticipant_delete_subscriber (
    gapi_domainParticipant _this,
    const gapi_subscriber s)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;
    _Subscriber subscriber = NULL;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant ) {
        subscriber = gapi_subscriberClaimNB(s, NULL);
    }

    if ( subscriber ) {
        if (subscriber != participant->builtinSubscriber) {
            gapi_setIter subscriberIter;

            subscriberIter = gapi_setFind (participant->subscriberSet, (gapi_object)subscriber);
            if ( subscriberIter != NULL ) {
                if (gapi_setIterObject(subscriberIter) != NULL) {
                    if (_SubscriberPrepareDelete (subscriber)) {
                        gapi_setRemove (participant->subscriberSet, (gapi_object)subscriber);
                        result = _SubscriberFree (subscriber);
                        if ( result == GAPI_RETCODE_OK ) {
                            subscriber = NULL;
                        }
                    } else {
                        /* subscribed still contains datareaders */
                        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                    }
                } else {
                    /* subscriber not created by this participant */
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
                gapi_setIterFree (subscriberIter);
            } else {
                /* Iterator could not be allocated */
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    _EntityRelease(subscriber);
    _EntityRelease(participant);

    return result;
}

/*     Subscriber
 *     get_builtin_subscriber();
 */
gapi_subscriber
gapi_domainParticipant_get_builtin_subscriber (
    gapi_domainParticipant _this)
{
    _DomainParticipant participant;
    _Subscriber subscriber = NULL;

    participant = gapi_domainParticipantClaim(_this, NULL);

    if ( participant != NULL ) {
        if ( _Entity(participant)->enabled ) {
            if ( participant->builtinSubscriber == NULL ) {
                participant->builtinSubscriber = _SubscriberBuiltinNew (
                    U_PARTICIPANT_GET(participant), participant->_Factory, participant);
                _ENTITY_REGISTER_OBJECT(_Entity(participant), (_Object)participant->builtinSubscriber);
                if ( participant->deleteActionInfo.action ) {
                    _SubscriberSetDeleteAction(participant->builtinSubscriber,
                                               participant->deleteActionInfo.action,
                                               participant->deleteActionInfo.argument);
                }
                _EntityRelease(participant->builtinSubscriber);
            }
            subscriber = participant->builtinSubscriber;
        }
    }

    _EntityRelease(participant);

    return (gapi_subscriber)_EntityHandle(subscriber);
}

_Subscriber
_DomainParticipantGetBuiltinSubscriber (
    _DomainParticipant participant)
{
    assert(participant);

    return participant->builtinSubscriber;
}

static gapi_boolean
addTopicDescription (
    _DomainParticipant participant,
    _TopicDescription  description)
{
    gapi_boolean result = FALSE;

    assert(participant);
    assert(description);

    if ( gapi_setAdd(participant->topicDescriptionSet, (gapi_object)description) == GAPI_RETCODE_OK ) {
        result = TRUE;
    }

    return result;
}

/*     Topic
 *     create_topic(
 *         in string topic_name,
 *         in string type_name,
 *         in TopicQos qos,
 *         in TopicListener a_listener);
 *
 *  Function will operate indepenedent of the enable flag
 */
gapi_topic
gapi_domainParticipant_create_topic (
    gapi_domainParticipant _this,
    const gapi_char *topic_name,
    const gapi_char *registered_type_name,
    const gapi_topicQos *qos,
    const struct gapi_topicListener *a_listener,
    const gapi_statusMask mask)
{
    _DomainParticipant participant;
    _Topic             newTopic    = NULL;
    gapi_topic         result      = NULL;
    _TopicDescription  description = NULL;
    _TypeSupport       typeSupport = NULL;
    gapi_string        typeName    = NULL;
    gapi_context       context;
    gapi_topicQos    * found_qos   = NULL;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_TOPIC);

    participant = gapi_domainParticipantClaim(_this, NULL);

    if ( participant != NULL ) {
        if ( (topic_name != NULL) && (registered_type_name != NULL) ) {
            if ( qos == GAPI_TOPIC_QOS_DEFAULT ) {
                qos = &participant->_defTopicQos;
            }
            typeSupport = _DomainParticipantFindType(participant, (const char*)registered_type_name);
            if ( typeSupport != NULL ) {
                typeName = _TypeSupportTypeName(typeSupport);
            }
        }
    } else {
        OS_REPORT_1(OS_API_INFO,
                    "gapi_domainParticipant_create_topic", 0,
                    "for topic <%s> claim participant failed ",
                    topic_name);
    }

    if ( typeSupport != NULL ) {
        description = _DomainParticipantFindTopicDescription(participant, topic_name);
        if ( description != NULL ) {
            _EntityClaim(description);
            if (_ObjectGetKind(_Object(description)) == OBJECT_KIND_TOPIC) {
                newTopic = _Topic(description);
                found_qos = gapi_topicQos__alloc();
                if ( gapi_topicQosEqual(qos, _TopicGetQos(newTopic,found_qos)) ) {
                    if ( _TopicDescriptionHasType(description, registered_type_name) ) {
                        newTopic = _TopicNew(topic_name, registered_type_name,
                                             typeSupport, qos, a_listener,mask, participant, &context);
                    } else {
                        OS_REPORT_1(OS_API_INFO,
                                    "gapi_domainParticipant_create_topic", 0,
                                    "<%s> is already defined but invalid type",
                                    topic_name);
                        newTopic= NULL;
                    }
                } else {
                    OS_REPORT_1(OS_API_INFO,
                                "gapi_domainParticipant_create_topic", 0,
                                "<%s> is already defined but incompatible QoS",
                                topic_name);
                    newTopic= NULL;
                }
                gapi_free(found_qos);
            } else {
                OS_REPORT_1(OS_API_INFO,
                            "gapi_domainParticipant_create_topic", 0,
                            "<%s> is already defined but is not a Topic",
                            topic_name);
                newTopic= NULL;
            }
        } else {
            newTopic = _TopicNew(topic_name, registered_type_name,
                                 typeSupport, qos, a_listener, mask, participant, &context);
        }
    } else if (topic_name) {
        OS_REPORT_1(OS_API_INFO,
                    "gapi_domainParticipant_create_topic", 0,
                    "for topic <%s> lookup typeSupport failed ",
                    topic_name);
    } else {
        OS_REPORT(OS_API_INFO,
                    "gapi_domainParticipant_create_topic", 0,
                    "lookup typeSupport failed topic_name was not defined");
    }

    if ( newTopic ) {
        if ( addTopicDescription(participant, _TopicDescription(newTopic)) ) {
            _TopicIncRef(newTopic);
            _ENTITY_REGISTER_OBJECT(_Entity(participant), (_Object)newTopic);
        } else {
            OS_REPORT_1(OS_API_INFO,
                        "gapi_domainParticipant_create_topic", 0,
                        "register topic <%s> in participant failed ",
                        topic_name);
            _TopicFree (newTopic);
            newTopic = NULL;
        }
    }

    _EntityRelease(description);
    _EntityRelease(participant);

    if ( newTopic ) {
        gapi_object statusHandle;
        statusHandle = _EntityHandle(_Entity(newTopic)->status);
        result = (gapi_topic)_EntityRelease(newTopic);
    }

    return result;
}


/*     ReturnCode_t
 *     delete_topic(
 *         in Topic a_topic);
 */
gapi_returnCode_t
gapi_domainParticipant_delete_topic (
    gapi_domainParticipant _this,
    const gapi_topic a_topic)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;
    _Topic topic = NULL;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        topic = gapi_topicClaimNB(a_topic, NULL);
        if ( topic == NULL ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if ( topic != NULL ) {
        gapi_setIter topicIter;

        topicIter = gapi_setFirst(participant->topicDescriptionSet);
        if ( topicIter ) {
            gapi_boolean found = FALSE;
            _Topic tp;

            tp = (_Topic) gapi_setIterObject(topicIter);
            while ( !found && tp ) {
                if ( tp == topic ) {
                    found = TRUE;
                } else {
                    gapi_setIterNext(topicIter);
                    tp = (_Topic) gapi_setIterObject(topicIter);
                }
            }

            if ( found ) {
                if ( _TopicPrepareDelete (topic) ) {
                    gapi_long count = _TopicDecRef(topic);
                    if ( count == 0 ) {
                        gapi_setRemove(participant->topicDescriptionSet, (gapi_object)topic);
                        _TopicFree(topic);
                        topic = NULL;
                    } else if ( count < 0 ) {
                        /* invalid refcount value */
                        result = GAPI_RETCODE_ERROR;
                    }
                } else {
                    /* topic is stil in use */
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                /* topic not created by this participant */
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            gapi_setIterFree(topicIter);
        } else {
            /* Iterator could not be allocated */
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    }

    _EntityRelease(topic);
    _EntityRelease(participant);

    return result;
}

/*     Topic
 *     find_topic(
 *         in string topic_name,
 *         in Duration_t timeout);
 */
gapi_topic
gapi_domainParticipant_find_topic (
    gapi_domainParticipant _this,
    const gapi_char *topic_name,
    const gapi_duration_t *timeout)
{
    _DomainParticipant participant;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Topic topic = NULL;
    c_iter topicList;
    v_duration vDuration;
    u_topic uTopic = NULL;
    char *typeName = NULL;
    const char *regTypeName = NULL;
    _TypeSupport typeSupport = NULL;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_FIND_TOPIC);

    participant = gapi_domainParticipantClaim(_this, NULL);

    if ( participant &&
         _Entity(participant)->enabled &&
         topic_name &&
         gapi_validDuration(timeout) ) {
        _TopicDescription topicDescription;

        /* first check if the topic is already defined for this participant */
        topicDescription = _DomainParticipantFindTopicDescription(participant, topic_name);
        if (topicDescription != NULL) {
            if (_ObjectGetKind(_Object(topicDescription)) == OBJECT_KIND_TOPIC) {
                topic = _TopicFromTopic(_Topic(topicDescription),
                                        participant,
                                        &context);
            }
        } else {
            kernelCopyInDuration(timeout, &vDuration);

            topicList = u_participantFindTopic (U_PARTICIPANT_GET(participant), topic_name, vDuration);

            if (topicList) {
                uTopic = u_topic(c_iterTakeFirst(topicList));
                c_iterFree (topicList);
            }

            if (uTopic) {
                typeName = (char *)u_topicTypeName(uTopic);
            }
            if (typeName) {
                typeSupport = _DomainParticipantFindTypeSupport(participant, typeName);
            }
            if (typeSupport) {
                regTypeName = _DomainParticipantGetRegisteredTypeName(participant, typeSupport);
                if (!kernelCheckTopicKeyList(uTopic,
                         _TypeSupportTypeKeys(typeSupport))) {
                    OS_REPORT_2(OS_API_INFO,
                                "gapi_domainParticipant_find_topic", 21,
                                "incompatible keylist between "
                                "found topic <%s> and "
                                "known typeSupport <%s>",
                                topic_name, regTypeName);
                    u_topicFree(uTopic);
                    uTopic = NULL;
                }
            } else {
                gapi_char *keys = kernelTopicGetKeys(uTopic);
                if (keys) {
                    typeSupport = _TypeSupportNew(typeName, keys,
                                                  NULL, NULL,
                                                  NULL, NULL,
                                                  0, NULL, NULL,
                                                  NULL, NULL, NULL);
                    os_free(keys);
                }

                if (typeSupport) {
                    regTypeName = typeName;
                    result = _TypeSupportGenericCopyInit(typeSupport,
                                                         participant);
                    if (result != GAPI_RETCODE_OK) {
                        gapi_free(_EntityRelease(typeSupport));
                        typeSupport = NULL;
                    }
                }

                if (typeSupport) {
                    result = _DomainParticipantRegisterType(participant,
                                                            typeSupport,
                                                            regTypeName);
                    if (result == GAPI_RETCODE_OK) {
                        _EntityRelease(typeSupport);
                    } else {
                        gapi_free(_EntityRelease(typeSupport));
                        u_topicFree(uTopic);
                        uTopic = NULL;
                    }
                }
            }

            if (uTopic) {
                topic = _TopicFromKernelTopic(uTopic, topic_name,
                                              regTypeName, typeSupport,
                                              participant, &context);
                if ( !topic ) {
                    u_topicFree(uTopic);
                }
            }
        }
        if ( topic ) {
            if ( addTopicDescription(participant,
                                     _TopicDescription(topic)) ) {
                _TopicIncRef(topic);
                _ENTITY_REGISTER_OBJECT(_Entity(participant),
                                        (_Object)topic);
            } else {
               _TopicFree(topic);
               topic = NULL;
            }
        }
    }


    if ( typeName ) {
        os_free (typeName);
    }

    _EntityRelease(participant);

    return _EntityRelease(topic);
}

/*     TopicDescription
 *     lookup_topicdescription(
 *         in string name);
 */
gapi_topicDescription
gapi_domainParticipant_lookup_topicdescription (
    gapi_domainParticipant _this,
    const gapi_char *name)
{
    _DomainParticipant participant;
    _TopicDescription topicDescription = NULL;

    participant = gapi_domainParticipantClaim(_this, NULL);

    if ( participant && _Entity(participant)->enabled && name ) {
        topicDescription = _DomainParticipantFindTopicDescription(participant, name);
    }

    _EntityRelease(participant);

    return (gapi_topicDescription)_EntityHandle(topicDescription);
}

/*     ContentFilteredTopic
 *     create_contentfilteredtopic(
 *         in string name,
 *         in Topic related_topic,
 *         in string filter_expression,
 *         in StringSeq filter_parameters);
 */
gapi_contentFilteredTopic
gapi_domainParticipant_create_contentfilteredtopic (
    gapi_domainParticipant _this,
    const gapi_char *name,
    const gapi_topic related_topic,
    const gapi_char *filter_expression,
    const gapi_stringSeq *filter_parameters)
{
    _DomainParticipant    participant;
    gapi_boolean          licensed;
    _ContentFilteredTopic newTopic    = NULL;
    _Topic                related     = NULL;

    licensed = _DomainParticipantFactoryIsContentSubscriptionAvailable();

    if(licensed == TRUE){

        if ( name && filter_expression && gapi_sequence_is_valid(filter_parameters) ) {
            participant = gapi_domainParticipantClaim(_this, NULL);

            if ( participant != NULL ) {
                _TopicDescription desc = _DomainParticipantFindTopicDescription(participant, name);

                if ( desc == NULL ) {
                    related = gapi_topicClaim(related_topic, NULL);
                    if ( (related != NULL) &&
                         _DomainParticipantTopicDescriptionExists(participant, _TopicDescription(related)) ) {
                        newTopic = _ContentFilteredTopicNew(name, related, filter_expression,
                                                            filter_parameters, participant);
                        if ( newTopic != NULL ) {
                            if ( addTopicDescription(participant, _TopicDescription(newTopic)) ) {
                                _ENTITY_REGISTER_OBJECT(_Entity(participant), (_Object)newTopic);
                            } else {
                                _ContentFilteredTopicFree(newTopic);
                                newTopic = NULL;
                            }
                        }
                        _EntityRelease(related);
                    }
                }
            }
            _EntityRelease(participant);
        }
    }
    return (gapi_contentFilteredTopic)_EntityRelease(newTopic);
}

/*     ReturnCode_t
 *     delete_contentfilteredtopic(
 *         in ContentFilteredTopic a_contentfilteredtopic);
 */
gapi_returnCode_t
gapi_domainParticipant_delete_contentfilteredtopic (
    gapi_domainParticipant _this,
    const gapi_contentFilteredTopic a_contentfilteredtopic)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;
    _ContentFilteredTopic contentfilteredtopic = NULL;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        contentfilteredtopic = gapi_contentFilteredTopicClaim(a_contentfilteredtopic, NULL);
        if ( contentfilteredtopic == NULL ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if ( contentfilteredtopic ) {
        gapi_setIter topicIter;

        topicIter = gapi_setFind(participant->topicDescriptionSet, (gapi_object)contentfilteredtopic);
        if ( topicIter ) {
            if ( gapi_setIterObject(topicIter) ) {
                if ( _ContentFilteredTopicPrepareDelete(contentfilteredtopic) ) {
                    gapi_setIterRemove(topicIter);
                    _ContentFilteredTopicFree(contentfilteredtopic);
                    contentfilteredtopic = NULL;
                } else {
                    /* prepare delete failed */
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                /* topic not created by this participant */
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            gapi_setIterFree(topicIter);
        } else {
            /* Iterator could not be allocated */
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    }

    _EntityRelease(contentfilteredtopic);
    _EntityRelease(participant);

    return result;
}

/*     MultiTopic
 *     create_multitopic(
 *         in string name,
 *         in string type_name,
 *         in string subscription_expression,
 *         in StringSeq expression_parameters);
 */
gapi_multiTopic
gapi_domainParticipant_create_multitopic (
    gapi_domainParticipant _this,
    const gapi_char *name,
    const gapi_char *type_name,
    const gapi_char *subscription_expression,
    const gapi_stringSeq *expression_parameters)
{
    gapi_boolean licensed;

    /* Function will operate indepenedent of the enable flag */

    licensed =  _DomainParticipantFactoryIsContentSubscriptionAvailable();

    if(licensed == TRUE){

    }
    return (gapi_multiTopic)0;
}

/*     ReturnCode_t
 *     delete_multitopic(
 *         in MultiTopic a_multitopic);
 */
gapi_returnCode_t
gapi_domainParticipant_delete_multitopic (
    gapi_domainParticipant _this,
    const gapi_multiTopic a_multitopic)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        result = GAPI_RETCODE_UNSUPPORTED;
    }

    _EntityRelease(participant);

    return result;
}

gapi_returnCode_t
gapi_domainParticipant_delete_historical_data (
    gapi_domainParticipant _this,
    const gapi_string partition_expression,
    const gapi_string topic_expression)
{
    u_result ur;
    gapi_returnCode_t result;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        ur = u_participantDeleteHistoricalData(
                    U_PARTICIPANT_GET(participant),
                    partition_expression,
                    topic_expression);

        if(ur == U_RESULT_OK){
            result = GAPI_RETCODE_OK;
        } else {
            result = GAPI_RETCODE_ERROR;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    _EntityRelease(participant);

    return result;

}


static gapi_returnCode_t
deleteContainedEntities (
    _DomainParticipant _this,
    gapi_deleteEntityAction action,
    void *action_arg)
{
    gapi_setIter iterSet;
    void *userData;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    /* delete all publishers in the publisherSet */
    /* first delete their contained entities and then themself */
    iterSet = gapi_setFirst (_this->publisherSet);
    while ((gapi_setIterObject(iterSet)) && (result == GAPI_RETCODE_OK)) {
        _Publisher publisher = _Publisher(gapi_setIterObject(iterSet));
        result = gapi_publisher_delete_contained_entities(_EntityHandle(publisher), action, action_arg);

        if(result == GAPI_RETCODE_OK){
            _EntityClaimNotBusy(publisher);
            userData = _ObjectGetUserData(_Object(publisher));
            _PublisherPrepareDelete(publisher);
            _PublisherFree(publisher);
            gapi_setIterRemove(iterSet);
            if ( action ) {
                action(userData, action_arg);
            }
        }
    }
    gapi_setIterFree(iterSet);

    /* delete all subsribers in the subscriberSet */
    /* first delete their contained entities and then themself */
    iterSet = gapi_setFirst (_this->subscriberSet);
    while ((gapi_setIterObject(iterSet)) && (result == GAPI_RETCODE_OK)) {
        _Subscriber subscriber = _Subscriber(gapi_setIterObject(iterSet));
        result = gapi_subscriber_delete_contained_entities(_EntityHandle(subscriber), action, action_arg);

        if(result == GAPI_RETCODE_OK){
            _EntityClaimNotBusy(subscriber);
            userData = _ObjectGetUserData(_Object(subscriber));
            _SubscriberPrepareDelete(subscriber);
            _SubscriberFree (subscriber);
            gapi_setIterRemove (iterSet);
            if ( action ) {
                action(userData, action_arg);
            }
        }
    }
    gapi_setIterFree (iterSet);

    /* Delete all ContentFilteredTopics in the topicDescriptionSet */
    /* Call descructors based of their types */
    iterSet = gapi_setFirst (_this->topicDescriptionSet);

    while ((gapi_setIterObject(iterSet)) && (result == GAPI_RETCODE_OK)) {
        _TopicDescription topicDescription = (_TopicDescription)gapi_setIterObject(iterSet);
        _EntityClaimNotBusy(topicDescription);
        userData = _ObjectGetUserData(_Object(topicDescription));
        if (_ObjectGetKind(_Object(topicDescription)) == OBJECT_KIND_CONTENTFILTEREDTOPIC ) {
            _ContentFilteredTopicPrepareDelete(_ContentFilteredTopic(topicDescription));
            _ContentFilteredTopicFree(_ContentFilteredTopic(topicDescription));
            gapi_setIterRemove (iterSet);
            if ( action ) {
                action(userData, action_arg);
            }
        } else {
            _EntityRelease(topicDescription);
            gapi_setIterNext(iterSet);
        }
    }
    gapi_setIterFree (iterSet);

    /* Delete all topicsdescriptions in the topicDescriptionSet */
    /* Call descructors based of their types */
    iterSet = gapi_setFirst (_this->topicDescriptionSet);

    while ((gapi_setIterObject(iterSet)) && (result == GAPI_RETCODE_OK)) {
        _TopicDescription topicDescription = (_TopicDescription)gapi_setIterObject(iterSet);
        _EntityClaimNotBusy(topicDescription);
        userData = _ObjectGetUserData(_Object(topicDescription));
        if ( _ObjectGetKind(_Object(topicDescription)) == OBJECT_KIND_TOPIC ) {
            _TopicPrepareDelete(_Topic(topicDescription));
            _TopicFree(_Topic(topicDescription));
        } else /* plain topicDescription or multi-topic (unimplemented) */{
            _TopicDescriptionFree (topicDescription);
        }
        gapi_setIterRemove (iterSet);
        if ( action ) {
            action(userData, action_arg);
        }
    }
    gapi_setIterFree (iterSet);

    return result;
}


/*     ReturnCode_t
 *     delete_contained_entities();
 */
gapi_returnCode_t
gapi_domainParticipant_delete_contained_entities (
    gapi_domainParticipant _this,
    gapi_deleteEntityAction action,
    void *action_arg)
{
    gapi_returnCode_t result;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        result = deleteContainedEntities(participant, action, action_arg);
        _EntityRelease(participant);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantQos qos);
 *
 * Function will operate indepenedent of the enable flag
 */

gapi_returnCode_t
gapi_domainParticipant_set_qos (
    gapi_domainParticipant _this,
    const gapi_domainParticipantQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_result uResult;
    _DomainParticipant participant;
    v_participantQos participantQos;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_QOS);

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant && qos ) {
        result = gapi_domainParticipantQosIsConsistent(qos, &context);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    if ( result == GAPI_RETCODE_OK ) {
        gapi_domainParticipantQos * existing_qos = gapi_domainParticipantQos__alloc();

        result = gapi_domainParticipantQosCheckMutability(
                     qos,
                     _DomainParticipantGetQos(participant, existing_qos),
                     &context);
        gapi_free(existing_qos);
    }

    if ( result == GAPI_RETCODE_OK ) {
        participantQos = u_participantQosNew(NULL);
        if (participantQos) {
            if ( copyParticipantQosIn(qos, participantQos) ) {
                uResult = u_entitySetQoS(_EntityUEntity(participant),
                                         (v_qos)(participantQos) );
                result = kernelResultToApiResult(uResult);
                if( result == GAPI_RETCODE_OK ) {
                    participant->listenerThreadInfo.scheduling = qos->listener_scheduling;
                }
                u_participantQosFree(participantQos);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantQos qos);
 *
 * Function will operate indepenedent of the enable flag
 */
gapi_returnCode_t
gapi_domainParticipant_get_qos (
    gapi_domainParticipant _this,
    gapi_domainParticipantQos *qos)
{
    _DomainParticipant participant;
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant && qos ) {
        _DomainParticipantGetQos(participant, qos);
    }

    _EntityRelease(participant);
    return result;
}

/*     ReturnCode_t
 *     set_listener(
 *         in DomainParticipantListener a_listener,
 *         in StatusMask mask);
 *
 * Function will operate indepenedent of the enable flag
 */
gapi_returnCode_t
gapi_domainParticipant_set_listener (
    gapi_domainParticipant _this,
    const struct gapi_domainParticipantListener *a_listener,
    const gapi_statusMask mask)
{
    gapi_returnCode_t result = GAPI_RETCODE_ERROR;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        _DomainParticipantStatus status;

        if ( a_listener ) {
            participant->_Listener = *a_listener;
        } else {
            memset(&participant->_Listener, 0, sizeof(participant->_Listener));
        }

        status = _DomainParticipantStatus(_EntityStatus(participant));
        if ( _DomainParticipantStatusSetListener(status, a_listener, mask) ) {
            result = GAPI_RETCODE_OK;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     DomainParticipantListener
 *     get_listener();
 *
 * Function will operate indepenedent of the enable flag
 */
struct gapi_domainParticipantListener
gapi_domainParticipant_get_listener (
    gapi_domainParticipant _this)
{
    _DomainParticipant participant;
    struct gapi_domainParticipantListener listener;

    participant = gapi_domainParticipantClaim(_this, NULL);

    if ( participant != NULL ) {
        listener = participant->_Listener;
    } else {
        memset(&listener, 0, sizeof(listener));
    }

    _EntityRelease(participant);

    return listener;
}

/*     ReturnCode_t
 *     ignore_participant(
 *         in InstanceHandle_t handle);
 */
gapi_returnCode_t
gapi_domainParticipant_ignore_participant (
    gapi_domainParticipant _this,
    const gapi_instanceHandle_t handle)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        if ( _Entity(participant)->enabled ) {
            result = GAPI_RETCODE_UNSUPPORTED;
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     ignore_topic(
 *         in InstanceHandle_t handle);
 */
gapi_returnCode_t
gapi_domainParticipant_ignore_topic (
    gapi_domainParticipant _this,
    const gapi_instanceHandle_t handle)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        if ( _Entity(participant)->enabled ) {
            result = GAPI_RETCODE_UNSUPPORTED;
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     ignore_publication(
 *         in InstanceHandle_t handle);
 */
gapi_returnCode_t
gapi_domainParticipant_ignore_publication (
    gapi_domainParticipant _this,
    const gapi_instanceHandle_t handle)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        if ( _Entity(participant)->enabled ) {
            result = GAPI_RETCODE_UNSUPPORTED;
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     ignore_subscription(
 *         in InstanceHandle_t handle);
 */
gapi_returnCode_t
gapi_domainParticipant_ignore_subscription (
    gapi_domainParticipant _this,
    const gapi_instanceHandle_t handle)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        if ( _Entity(participant)->enabled ) {
            result = GAPI_RETCODE_UNSUPPORTED;
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     DomainId_t
 *     get_domain_id();
 */
gapi_domainId_t
gapi_domainParticipant_get_domain_id (
    gapi_domainParticipant _this)
{
    _DomainParticipant participant;
    gapi_domainId_t domainId = 0;

    participant = gapi_domainParticipantClaim(_this, NULL);

    if ( (participant != NULL) && _Entity(participant)->enabled) {
        if ( participant->_DomainId ) {
            domainId = gapi_string_dup(participant->_DomainId);
        }
    }

    _EntityRelease(participant);

    return domainId;
}

/*     void
 *     assert_liveliness();
 */
gapi_returnCode_t
gapi_domainParticipant_assert_liveliness (
    gapi_domainParticipant _this)
{
    _DomainParticipant participant;
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( (participant != NULL) && _Entity(participant)->enabled) {
        u_participantAssertLiveliness(U_PARTICIPANT_GET(participant));
    }
    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     set_default_publisher_qos(
 *         in PublisherQos qos);
 */
gapi_returnCode_t
gapi_domainParticipant_set_default_publisher_qos (
    gapi_domainParticipant _this,
    const gapi_publisherQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_DEFAULT_PUBLISHER_QOS);

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant && qos ) {
        if ( _Entity(participant)->enabled ) {
            result = gapi_publisherQosIsConsistent(qos, &context);
            if ( result == GAPI_RETCODE_OK ) {
                gapi_publisherQosCopy (qos, &participant->_defPublisherQos);
            }
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     get_default_publisher_qos(
 *         inout PublisherQos qos);
 */
gapi_returnCode_t
gapi_domainParticipant_get_default_publisher_qos (
    gapi_domainParticipant _this,
    gapi_publisherQos *qos)
{
    _DomainParticipant participant;
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant && _Entity(participant)->enabled && qos ) {
        gapi_publisherQosCopy (&participant->_defPublisherQos, qos);
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     set_default_subscriber_qos(
 *         in SubscriberQos qos);
 */
gapi_returnCode_t
gapi_domainParticipant_set_default_subscriber_qos (
    gapi_domainParticipant _this,
    const gapi_subscriberQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_DEFAULT_SUBSCRIBER_QOS);

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant ) {
        if ( qos ) {
            if ( _Entity(participant)->enabled ) {
                result = gapi_subscriberQosIsConsistent(qos, &context);
                if ( result == GAPI_RETCODE_OK ) {
                    gapi_subscriberQosCopy (qos, &participant->_defSubscriberQos);
                }
            } else {
                result = GAPI_RETCODE_NOT_ENABLED;
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     get_default_subscriber_qos(
 *         inout SubscriberQos qos);
 */
gapi_returnCode_t
gapi_domainParticipant_get_default_subscriber_qos (
    gapi_domainParticipant _this,
    gapi_subscriberQos *qos)
{
    _DomainParticipant participant;
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( (participant != NULL) && _Entity(participant)->enabled && qos ) {
        gapi_subscriberQosCopy (&participant->_defSubscriberQos, qos);
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     set_default_topic_qos(
 *         in TopicQos qos);
 */
gapi_returnCode_t
gapi_domainParticipant_set_default_topic_qos (
    gapi_domainParticipant _this,
    const gapi_topicQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant participant;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_DEFAULT_TOPIC_QOS);

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant ) {
        if ( qos ) {
            if ( _Entity(participant)->enabled ) {
                result = gapi_topicQosIsConsistent(qos, &context);
                if ( result == GAPI_RETCODE_OK ) {
                    gapi_topicQosCopy (qos, &participant->_defTopicQos);
                }
            } else {
                result = GAPI_RETCODE_NOT_ENABLED;
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(participant);

    return result;
}

/*     ReturnCode_t
 *     get_default_topic_qos(
 *         inout TopicQos qos);
 */
gapi_returnCode_t
gapi_domainParticipant_get_default_topic_qos (
    gapi_domainParticipant _this,
    gapi_topicQos *qos)
{
    _DomainParticipant participant;
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant && _Entity(participant)->enabled && qos ) {
        gapi_topicQosCopy (&participant->_defTopicQos, qos);
    }
    _EntityRelease(participant);

    return result;
}

gapi_boolean
_DomainParticipantSetListenerInterestOnChildren (
    _DomainParticipant    participant,
    _ListenerInterestInfo info)
{
    gapi_setIter iterSet;
    gapi_mapIter iterMap;
    _Entity      entity;
    gapi_boolean result = TRUE;
    _TopicDescription td;


    assert(participant);

    iterSet = gapi_setFirst (participant->topicDescriptionSet);
    td = (_TopicDescription)gapi_setIterObject(iterSet);
    while (result && td) {
        /* A listener can only be set on a topic, since the topic
           is a domain entity and both the contentfilteredtopic
           and the multitopic are not
         */
        if (_ObjectGetKind(_Object(td)) == OBJECT_KIND_TOPIC) {
            entity = _Entity(td);
            result = _EntitySetListenerInterest(entity, info);
        }
        gapi_setIterNext(iterSet);
        td = (_TopicDescription)gapi_setIterObject(iterSet);
    }
    gapi_setIterFree (iterSet);

    if ( participant->builtinSubscriber ) {
        iterMap = gapi_mapFirst (participant->builtinTopicMap);
        while (result && gapi_mapIterObject(iterMap)) {
            entity = _Entity(gapi_mapIterObject(iterMap));
            result = _EntitySetListenerInterest(entity, info);
            gapi_mapIterNext (iterMap);
        }
        gapi_mapIterFree (iterMap);
        entity = _Entity(participant->builtinSubscriber);
        result = _EntitySetListenerInterest(entity, info);
    }

    iterSet = gapi_setFirst (participant->publisherSet);
    while (result && gapi_setIterObject(iterSet)) {
        entity = _Entity(gapi_setIterObject(iterSet));
        result = _EntitySetListenerInterest(entity, info);
        gapi_setIterNext (iterSet);
    }
    gapi_setIterFree (iterSet);

    iterSet = gapi_setFirst (participant->subscriberSet);
    while (result && gapi_setIterObject(iterSet)) {
        entity = _Entity(gapi_setIterObject(iterSet));
        result = _EntitySetListenerInterest(entity, info);
        gapi_setIterNext (iterSet);
    }
    gapi_setIterFree (iterSet);

    return result;
}

gapi_domainId_t
_DomainParticipantGetDomainId (
    _DomainParticipant participant)
{
    assert(participant);

    return participant->_DomainId;
}

typedef struct {
    c_base base;
} metadescriptionActionArg;

static void
get_type_metadescription_action (
    v_entity e,
    c_voidp argument)
{
    metadescriptionActionArg *arg = (metadescriptionActionArg *)argument;

    arg->base = c_getBase(c_object(e));
}

c_metaObject
_DomainParticipant_get_type_metadescription (
    _DomainParticipant _this,
    const gapi_char *type_name)
{
    c_metaObject typeDescription = NULL;
    metadescriptionActionArg arg;

    arg.base = NULL;
    if (u_entityAction(u_entity(_DomainParticipantUparticipant(_this)),
                        get_type_metadescription_action, (c_voidp)&arg) == U_RESULT_OK ) {
        if (arg.base) {
            typeDescription = c_metaResolve (c_metaObject(arg.base), type_name);
        }
    }

    return typeDescription;
}

/*     gapi_metaDescription
 *     get_type_metadescription (
 *         in string type_name);
 */
gapi_metaDescription
gapi_domainParticipant_get_type_metadescription (
    gapi_domainParticipant _this,
    const gapi_char *type_name)
{
    _DomainParticipant participant;
    c_metaObject typeDescription = NULL;

    participant = gapi_domainParticipantClaim(_this, NULL);
    if ( participant && type_name ) {
        typeDescription = _DomainParticipant_get_type_metadescription(participant,type_name);
    }
    _EntityRelease(participant);

    return typeDescription;
}

/*     gapi_typeSupport
 *     get_typesupport (
 *         in string registered_name);
 */
gapi_typeSupport
gapi_domainParticipant_get_typesupport (
    gapi_domainParticipant _this,
    const gapi_char *type_name)
{
    _DomainParticipant participant;
    gapi_typeSupport typeSupport = NULL;

    participant = gapi_domainParticipantClaim (_this, NULL);
    if (participant) {
        if ( type_name ) {
            typeSupport = (gapi_typeSupport)_EntityHandle(_DomainParticipantFindType (participant, type_name));
        }
    }
    _EntityRelease (participant);

    return typeSupport;
}

/*     gapi_typeSupport
 *     get_typesupport (
 *         in string type_name);
 */
gapi_typeSupport
gapi_domainParticipant_lookup_typesupport (
    gapi_domainParticipant _this,
    const gapi_char *type_name)
{
    _DomainParticipant participant;
    gapi_typeSupport typeSupport = NULL;

    participant = gapi_domainParticipantClaim (_this, NULL);
    if (participant) {
        if ( type_name ) {
            typeSupport = (gapi_typeSupport)_EntityHandle(_DomainParticipantFindType(participant, type_name));
        }
    }
    _EntityRelease (participant);

    return typeSupport;
}

void
_DomainParticipantSetBuiltinDeleteAction (
    _DomainParticipant      participant,
    gapi_deleteEntityAction action,
    void                   *action_arg,
    gapi_boolean            onTypeSupport)
{
    gapi_mapIter iter;

    assert(participant);
    assert(action);

    participant->deleteActionInfo.action   = action;
    participant->deleteActionInfo.argument = action_arg;

    if ( participant->builtinSubscriber ) {
        _EntityClaim(participant->builtinSubscriber);
        _SubscriberSetDeleteAction(participant->builtinSubscriber, action, action_arg);
        _EntityRelease(participant->builtinSubscriber);
    }

    iter = gapi_mapFirst(participant->builtinTopicMap);
    if ( iter ) {
        _Topic topic = (_Topic) gapi_mapIterObject(iter);
        while ( topic ) {
            _EntityClaim(topic);
            _TopicSetDeleteAction(topic, action, action_arg);
            _EntityRelease(topic);
            gapi_mapIterNext(iter);
            topic = (_Topic) gapi_mapIterObject(iter);
        }
        gapi_mapIterFree(iter);
    }

    if ( onTypeSupport ) {
        iter = gapi_mapFirst(participant->typeSupportMap);
        if ( iter ) {
            _TypeSupport typeSupport = (_TypeSupport) gapi_mapIterObject(iter);
            while ( typeSupport ) {
               _EntityClaim(typeSupport);
                _ObjectSetDeleteAction((_Object)typeSupport, action, action_arg);
                _EntityRelease(typeSupport);
                gapi_mapIterNext(iter);
                typeSupport = (_TypeSupport) gapi_mapIterObject(iter);
            }
            gapi_mapIterFree(iter);
        }
    }
}

void
_DomainParticipantGetListenerActionInfo (
    _DomainParticipant participant,
    gapi_listenerThreadAction *startAction,
    gapi_listenerThreadAction *stopAction,
    void                      **actionArg)
{
    assert(participant);
    assert(startAction);
    assert(stopAction);
    assert(actionArg);

    *startAction = participant->listenerThreadInfo.startAction;
    *stopAction  = participant->listenerThreadInfo.stopAction;
    *actionArg   = participant->listenerThreadInfo.actionArg;
}

static void *
listenerEventThread (
    void *arg)
{
    ListenerThreadInfo *info = (ListenerThreadInfo *) arg;
    c_iter  list = NULL;

    if ( info->startAction ) {
        info->startAction(info->actionArg);
    }

    while ( info->running ) {
        gapi_setIter iter;

        os_mutexLock(&info->mutex);
        iter = gapi_setFirst(info->toAddList);
        while ( gapi_setIterObject(iter) ) {
           gapi_object handle = (gapi_object) gapi_setIterObject(iter);
           _Entity entity;

            os_mutexUnlock(&info->mutex);
            entity = gapi_entityClaim(handle, NULL);
            if ( entity ) {
                if ( u_waitsetAttach(info->waitset,
                                     entity->uEntity,
                                     (c_voidp)entity->uEntity) == U_RESULT_OK ) {
                    _EntityNotifyInitialEvents(entity);
                } else {
                    OS_REPORT(OS_ERROR,
                              "listenerEventThread", 0,
                              "u_waitsetAttach failed");
                }
            }
            gapi_entityRelease(handle);
            os_mutexLock(&info->mutex);
            gapi_setIterRemove(iter);
        }
        gapi_setIterFree(iter);
        os_mutexUnlock(&info->mutex);

        u_waitsetWaitEvents(info->waitset,&list);
        if ( list ) {
            u_waitsetEvent event = u_waitsetEvent(c_iterTakeFirst(list));
            while ( event ) {
                u_entity uEntity = event->entity;
                gapi_entity source = (gapi_entity)(u_entityGetUserData(uEntity));

                gapi_entityNotifyEvent(source, event->events);

                u_waitsetEventFree(event);
                event = u_waitsetEvent(c_iterTakeFirst(list));
            }
            c_iterFree(list);
            list = NULL;
        }
    }

    if ( info->stopAction ) {
        info->stopAction(info->actionArg);
    }

    return NULL;
}


static gapi_boolean
startListenerEventThread (
    _DomainParticipant participant)
{
    ListenerThreadInfo *info;
    os_threadAttr osThreadAttr;
    os_result     osResult;
    gapi_boolean  result = FALSE;

    info = &participant->listenerThreadInfo;

    info->running = TRUE;

    osResult = os_threadAttrInit(&osThreadAttr);
    if ( osResult == os_resultSuccess ) {
        /* Set scheduling attributes as specified by the DomainParticipantQos */
        gapi_threadAttrInit (&info->scheduling, &osThreadAttr);
        /* Set stackSize as specified in the configuration */
        if ( info->stackSize != GAPI_DEFAULT_LISTENER_STACKSIZE ) {
            osThreadAttr.stackSize = info->stackSize;
        }
        osResult = os_threadCreate(&info->threadId, "ListenerEventThread", &osThreadAttr,
                                   (void *(*)(void*))listenerEventThread, (void *)info);
        if ( osResult == os_resultSuccess ) {
            result = TRUE;
        } else {
            info->running = FALSE;
            OS_REPORT(OS_ERROR, "startListenerEventThread", 0, "failed to start listener thread");
        }
    } else {
        info->running = FALSE;
        OS_REPORT(OS_ERROR, "startListenerEventThread", 0, "failed to init thread attributes");
    }

    return result;
}

static gapi_boolean
stopListenerEventThread (
    _DomainParticipant participant)
{
    ListenerThreadInfo *info;
    gapi_boolean        result = FALSE;

    info = &participant->listenerThreadInfo;

    if ( info->running ) {
        info->running = FALSE;

        if ( info->waitset ) {
            u_waitsetNotify(info->waitset, NULL);
            os_threadWaitExit(info->threadId, NULL);
            result = TRUE;
        }
    }

    return result;
}

gapi_boolean
_DomainParticipantAddListenerInterest (
    _DomainParticipant participant,
    _Status            status)
{
    ListenerThreadInfo *info;
    gapi_boolean        result = FALSE;

    info = &participant->listenerThreadInfo;

    os_mutexLock(&info->mutex);
    gapi_setAdd(info->toAddList, _EntityHandle(status->entity));
    os_mutexUnlock(&info->mutex);

    if ( info->running ) {
        u_waitsetNotify(info->waitset, NULL);
    } else {
        startListenerEventThread(participant);
    }

    return result;
}


gapi_boolean
_DomainParticipantRemoveListenerInterest (
    _DomainParticipant participant,
    _Status            status)
{
    ListenerThreadInfo *info;
    gapi_boolean        result = FALSE;

    info = &participant->listenerThreadInfo;

    if ( info->waitset ) {
        if ( info->running ) {
            if ( u_waitsetDetach(info->waitset, status->userEntity) == U_RESULT_OK ) {
                result = TRUE;
            }
        }
    }

    return result;
}


/*     ReturnCode_t
 *     get_discovered_participants (
 *         inout InstanceHandleSeq participant_handles);
 */
gapi_returnCode_t
gapi_domainParticipant_get_discovered_participants (
    gapi_domainParticipant _this,
    gapi_instanceHandleSeq  *participant_handles)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

/*     ReturnCode_t
 *     get_discovered_participant_data (
 *         in InstanceHandle_t handle,
 *         inout ParticipantBuiltinTopicData *participant_data);
 */
gapi_returnCode_t
gapi_domainParticipant_get_discovered_participant_data (
    gapi_domainParticipant _this,
    gapi_participantBuiltinTopicData *participant_data,
    gapi_instanceHandle_t  handle)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

/*     ReturnCode_t
 *     get_discovered_topics (
 *         inout InstanceHandleSeq topic_handles);
 */
gapi_returnCode_t
gapi_domainParticipant_get_discovered_topics (
    gapi_domainParticipant _this,
    gapi_instanceHandleSeq  *topic_handles)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

/*     ReturnCode_t
 *     get_discovered_topic_data (
 *         in InstanceHandle_t handle,
 *         inout TopicBuiltinTopicData *topic_data);
 */
gapi_returnCode_t
gapi_domainParticipant_get_discovered_topic_data (
    gapi_domainParticipant _this,
    gapi_topicBuiltinTopicData *topic_data,
    gapi_instanceHandle_t  handle)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

/*     Boolean
 *     contains_entity (
 *         in InstanceHandle_t a_hande);
 */
gapi_boolean
gapi_domainParticipant_contains_entity (
    gapi_domainParticipant _this,
    gapi_instanceHandle_t  a_handle)
{
    gapi_boolean result = FALSE;
    _DomainParticipant participant;

    if ( a_handle ) {
        participant = gapi_domainParticipantClaim(_this, NULL);
        if ( participant ) {
            gapi_setIter iterSet;

            iterSet = gapi_setFirst (participant->publisherSet);
            while ( !result && gapi_setIterObject(iterSet) ) {
                _Publisher publisher = _Publisher(gapi_setIterObject(iterSet));
                result = _EntityHandleEqual(_Entity(publisher), a_handle);
                if ( !result ) {
                    result = _PublisherContainsEntity(publisher, a_handle);
                }
                gapi_setIterNext(iterSet);
            }
            gapi_setIterFree(iterSet);

            if ( !result ) {
                iterSet = gapi_setFirst (participant->subscriberSet);
                while ( !result && gapi_setIterObject(iterSet) ) {
                    _Subscriber subscriber = _Subscriber(gapi_setIterObject(iterSet));
                    result = _EntityHandleEqual(_Entity(subscriber), a_handle);
                    if ( !result ) {
                        result = _SubscriberContainsEntity(subscriber, a_handle);
                    }
                    gapi_setIterNext(iterSet);
                }
                gapi_setIterFree (iterSet);
            }

            if ( !result ) {
                iterSet = gapi_setFirst (participant->topicDescriptionSet);
                while ( !result && gapi_setIterObject(iterSet) ) {
                    _TopicDescription topic = _TopicDescription(gapi_setIterObject(iterSet));
                    result = _EntityHandleEqual(_Entity(topic), a_handle);
                    gapi_setIterNext(iterSet);
                }
                gapi_setIterFree (iterSet);
            }
        }

        _EntityRelease(participant);
    }

    return result;
}

/*     ReturnCode_t
 *     get_current_time (
 *         inout Time_t current_time);
 */
gapi_returnCode_t
gapi_domainParticipant_get_current_time (
    gapi_domainParticipant _this,
    gapi_time_t *current_time)
{
    _DomainParticipant participant;
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim (_this, &result);
    if ( participant ) {
        c_time t = u_timeGet();
        result = kernelCopyOutTime(&t, current_time);
    }
    _EntityRelease(participant);

    return result;
}



void
_DomainParticipantCleanup (
    _DomainParticipant _this)
{
    assert(_this);

    _EntityClaimNotBusy(_this);

    deleteContainedEntities(_this, NULL, NULL);

    _DomainParticipantFree(_this);
}

