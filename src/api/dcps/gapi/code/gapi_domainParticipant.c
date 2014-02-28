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
#include "gapi_entity.h"
#include "gapi_kernel.h"
#include "gapi_publisher.h"
#include "gapi_subscriber.h"
#include "gapi_builtin.h"
#include "gapi_qos.h"
#include "gapi_map.h"
#include "gapi_structured.h"
#include "gapi_topic.h"
#include "gapi_topicDescription.h"
#include "gapi_contentFilteredTopic.h"
#include "gapi_typeSupport.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_waitSet.h"
#include "gapi_objManag.h"
#include "gapi_error.h"
#include "gapi_scheduler.h"

#include "os.h"
#include "os_report.h"

#include "u_user.h"
#include "u_waitsetEvent.h"

/* kernel includes */
#include "v_event.h"
#include "v_message.h"
#include "v_readerSample.h"

/* value to represent the platform dependent default */
#define GAPI_DEFAULT_LISTENER_STACKSIZE 0

/*
 * Within a domain participant a type support can be registered. The type support can be uniquely identified by a
  * typename (the idl type name) or an alias (user defined name). But the type support should be findable by both names
  * This leads to a typesupport to be stored within a keyed map within the domain participant. But this map will have
  * 2 keys, the alias name and the typename.
  * The combination of these two keys does _not_ make a type support unique, one of them is enough as a unique identifier.
  */
C_CLASS(TypeSupportMapKey);
C_STRUCT(TypeSupportMapKey)
{
    gapi_char* typeAliasName;
    gapi_char* typeName;
};


/*
 * This operation is used as the compare operation for the map of type supports of the participant. This operation
 * will return EQUAL if a key in the map can be matched to the 'searchKey'. These keys are considered equal if:
 * key.typeName equals searchKey.typeName OR
 * key.typeName equals searchKey.typeAliasName OR
 * key.typeAliasName equals searchKey.typeName OR
 * key.typeAliasName equals searchKey.typeAliasName
 *
 * So equality here does NOT mean that both the typeName AND typeAliasName must be equal. See explaination regarding
 * TypeSupportMapKey for more info
 */
gapi_equality
gapi_typeSupportCompare (
    TypeSupportMapKey key,
    TypeSupportMapKey searchKey)
{
    gapi_equality eq;

    /* equality is reached when the typeName or the typeAliasName of the
     * searchKey matches either the typeName or typeAliasName of the key.
     */
    if(searchKey->typeName)
    {
        /* Check if the type name being searched for matches the typeName of the
         * key or if it matches the typeAliasName of the key.
         */
        eq = gapi_stringCompare(searchKey->typeName, key->typeName);
        if(eq != GAPI_EQ)
        {
            eq = gapi_stringCompare(searchKey->typeName, key->typeAliasName);
        }
    } else
    {
        eq = GAPI_NE;
    }
    /* if a typeAliasName was used in the search and we have not yet matched the
     * typeName of the searchKey, then compare the typeAliasName of the
     * searchKey too
     */
    if(searchKey->typeAliasName && eq != GAPI_EQ)
    {
        /* Check if the type alias name being searched for matches the typeName
         * of the key or if it matches the typeAliasName of the key.
         */
        eq = gapi_stringCompare(searchKey->typeAliasName, key->typeName);
        if(eq != GAPI_EQ)
        {
            eq = gapi_stringCompare(searchKey->typeAliasName, key->typeAliasName);
        }
    }/* else the 'eq' value of the searchKey->typeName remains the truth */
    return eq;
}

typedef struct DeleteActionInfo_s {
    gapi_deleteEntityAction action;
    void                    *argument;
} DeleteActionInfo;

typedef enum {
   STOPPED,
   STARTING,
   RUNNING,
   STOPPING
} threadState_t;

typedef struct ListenerThreadInfo_s {
    os_mutex                  mutex;
    os_cond                   cond;
    os_threadId               threadId;
    u_waitset                 waitset;
    threadState_t             threadState;
    gapi_listenerThreadAction startAction;
    gapi_listenerThreadAction stopAction;
    void                      *actionArg;
    c_iter                    toAddList;
    gapi_schedulingQosPolicy  scheduling;
    os_uint32                 stackSize;
} ListenerThreadInfo;

C_STRUCT(_DomainParticipant) {
    C_EXTENDS(_Entity);
    gapi_domainName_t                        _DomainId;
    gapi_publisherQos                      _defPublisherQos;
    gapi_subscriberQos                     _defSubscriberQos;
    gapi_topicQos                          _defTopicQos;
    struct gapi_domainParticipantListener  _Listener;
    _DomainParticipantFactory              _Factory;
    gapi_map                                typeSupportMap;
    void *                                  userData;
    _Subscriber                             builtinSubscriber;
    DeleteActionInfo                        deleteActionInfo;
    ListenerThreadInfo                      listenerThreadInfo;
    gapi_schedulingQosPolicy                watchdogScheduling;
    /* temporary attribute until user layer support becomes available. */
    c_iter                                  contentFilteredTopics;
    gapi_domainId_int_t                     _dId;
};

typedef struct {
    gapi_long         topic_create_count;
    _TopicDescription topic_description;
} topicInfo;

static gapi_boolean
startListenerEventThread (
    _DomainParticipant _this);

static gapi_boolean
stopListenerEventThread (
    _DomainParticipant _this);

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
    _DomainParticipant _this = _DomainParticipantAlloc();

    if ( _this != NULL ) {
        _this->typeSupportMap = gapi_mapNew (gapi_typeSupportCompare, FALSE, FALSE);
        _this->builtinSubscriber = _Subscriber (0);

        if (_this->typeSupportMap == NULL) {
            _EntityDelete(_this);
            _this = NULL;
        }
    } else {
        _EntityDelete(_this);
        _this = NULL;
    }

    return _this;
}


static void
deallocateParticipant (
    _DomainParticipant _this)
{
    if ( _this->_DomainId ) {
        os_free(_this->_DomainId);
    }
    gapi_publisherQos_free(&_this->_defPublisherQos);
    gapi_subscriberQos_free(&_this->_defSubscriberQos);
    gapi_topicQos_free(&_this->_defTopicQos);
    gapi_mapFree(_this->typeSupportMap);
    _EntityDispose(_Entity(_this));
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
    os_condAttr osCondAttr;
    u_cfElement cfg;
    u_cfData cfg_data;
    c_ulong cfg_value;

    memset(info, 0, sizeof(ListenerThreadInfo));

    osResult = os_mutexAttrInit (&osMutexAttr);
    osMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    osResult = os_condAttrInit (&osCondAttr);
    osCondAttr.scopeAttr = OS_SCOPE_PRIVATE;
    osResult = os_mutexInit (&info->mutex, &osMutexAttr);
    if ( osResult == os_resultSuccess ) {
       osResult = os_condInit (&info->cond, &info->mutex, &osCondAttr);
    }

    if ( osResult == os_resultSuccess ) {
        info->threadId    = OS_THREAD_ID_NONE;
        info->threadState = STOPPED;
        info->waitset     = NULL;
        info->startAction = threadStartAction;
        info->stopAction  = threadStopAction;
        info->actionArg   = actionArg;
        info->scheduling  = qos->listener_scheduling;
        info->toAddList   = NULL;
        info->stackSize   = GAPI_DEFAULT_LISTENER_STACKSIZE;
        info->waitset     = u_waitsetNew(uParticipant);

        if ( info->waitset ) {
            initialized = TRUE;
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
    os_mutexLock(&info->mutex);
    if ( info->waitset ) {
        u_waitsetFree(info->waitset);
        info->waitset = NULL;
    }

    if ( info->toAddList ) {
        c_iterFree(info->toAddList);
        info->toAddList = NULL;
    }
    os_mutexUnlock(&info->mutex);
    os_condDestroy (&info->cond);
    os_mutexDestroy (&info->mutex);
}

/**
 * Determines the name of the process.
 * @param context   The gapi_context from where this function is called (only
 *                  for error-reporting.
 * @return          A heap-allocated string (has to be freed by os_free), or
 *                  NULL if failed. The string will have to form that
 *                  os_procFigureIdentity returns.
 */
#define _GAPI_DP_INITIAL_LEN (32)
static char*
determineProcIdentity (
    const gapi_context *context)
{
    char *result, *realloced;
    os_int32 size;

    result = (char*)os_malloc(_GAPI_DP_INITIAL_LEN);
    if(result){
        size = os_procFigureIdentity(result, _GAPI_DP_INITIAL_LEN);
        if(size >= _GAPI_DP_INITIAL_LEN){
            /* Output was truncated, only realloc once, since the identity is
             * not changing. */
            realloced = (char*)os_realloc(result, size + 1);
            if(realloced){
                result = realloced;
                size = os_procFigureIdentity(result, size + 1);
            } else {
                os_free(result);
                result = NULL;
                gapi_errorReport(context, GAPI_ERRORCODE_OUT_OF_RESOURCES);
            }
        }
        /* No else, since fall-through for second os_procFigureIdentity-call */
        if (size < 0){
            /* An error occurred */
            os_free(result);
            result = NULL;
            gapi_errorReport(context, GAPI_ERRORCODE_ERROR);
        }
    } else {
        gapi_errorReport(context, GAPI_ERRORCODE_OUT_OF_RESOURCES);
    }

    return result;
}
#undef _GAPI_DP_INITIAL_LEN

_DomainParticipant
_DomainParticipantNew (
    gapi_domainName_t                              domainId,
    const gapi_domainParticipantQos             *qos,
    const struct gapi_domainParticipantListener *a_listener,
    const gapi_statusMask                        mask,
    _DomainParticipantFactory                    theFactory,
    gapi_listenerThreadAction                    startAction,
    gapi_listenerThreadAction                    stopAction,
    void                                        *actionArg,
    const gapi_context                          *context,
    gapi_domainId_int_t                          dId,
    const char                                  *name)
{
    _DomainParticipant newParticipant;
    v_participantQos participantQos = NULL;
    u_participant uParticipant;
    char participantId[256];
    char *procIdentity;

    newParticipant = allocateParticipant();

    if (newParticipant != NULL) {
        _EntityInit (_Entity(newParticipant), NULL);

        newParticipant->_DomainId = gapi_strdup(domainId);
        newParticipant->_dId = dId;
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
        newParticipant->contentFilteredTopics = NULL;

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
        if (name == NULL) {
            procIdentity = determineProcIdentity(context);
            if(procIdentity && procIdentity[0] != '\0'){
                if (procIdentity[0] != '<') {
                    /* A name was found, so use that as an identifier */
                    snprintf(participantId, sizeof(participantId), "%s",
                            procIdentity);
                } else {
                    /* Only PID was returned, decorate with some extra info */
                    snprintf(participantId, sizeof(participantId),
                             "DCPS Appl %s "PA_ADDRFMT"_"PA_ADDRFMT, procIdentity,
                             os_threadIdToInteger(os_threadIdSelf()),
                             (PA_ADDRCAST) newParticipant);
                }
            } else {
                /* Memory claim failed or empty string, fill in something useful */
                snprintf(participantId, sizeof(participantId),
                         "DCPS Appl <%d> "PA_ADDRFMT"_"PA_ADDRFMT,
                         os_procIdToInteger(os_procIdSelf()),
                         os_threadIdToInteger(os_threadIdSelf()),
                         (PA_ADDRCAST) newParticipant);
            }
            if(procIdentity){
                os_free(procIdentity);
            }
        }

        uParticipant = u_participantNew (domainId, 1,
                                         name ? name : participantId,
                                         (v_qos)participantQos,
                                         FALSE);
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
                                    uParticipant,
                                    startAction,
                                    stopAction,
                                    actionArg,
                                    qos))
        {
            gapi_errorReport(context, GAPI_ERRORCODE_OUT_OF_RESOURCES);
            u_participantFree(uParticipant);
            deallocateParticipant(newParticipant);
            newParticipant = NULL;
        }
    }

    if (newParticipant != NULL) {
        _Status status;

        status = _StatusNew(_Entity(newParticipant),
                            STATUS_KIND_PARTICIPANT,
                            (struct gapi_listener *)a_listener, mask);
        if (status) {
            _EntityStatus(newParticipant) = status;
        } else {
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
    _DomainParticipant _this)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Status status;
    u_participant p;
    gapi_typeSupport handle;
    c_long tsMapLength;
    c_long i;
    gapi_mapIter iter;
    TypeSupportMapKey key;
    _TypeSupport ts;

    assert (_this);

    status = _EntityStatus(_this);
    _StatusSetListener(status, NULL, 0);

    _EntityClaim(status);
    _StatusDeinit(status);

    tsMapLength = gapi_mapLength(_this->typeSupportMap);
    for(i = 0; i < tsMapLength; i++)
    {
        /* get the first entry of the map, a little later we will remove the
         * found entry which will in turn ensure the next loop another iter
         * object will be first.
         */
        iter = gapi_mapFirst(_this->typeSupportMap);
        assert(iter); /* should be ok, as it should match the length */
        key = (TypeSupportMapKey)gapi_mapIterKey(iter);
        ts = (_TypeSupport)gapi_mapIterObject(iter);
        /* remove the entry from the map */
        gapi_mapRemove(_this->typeSupportMap, (gapi_object)key);
        /* Free all key related memory */
        os_free(key->typeAliasName);
        os_free(key->typeName);
        os_free(key);
        /* Free the type support handle */
        handle = _EntityHandle(ts);
        gapi_free(handle);
        os_free(iter);
    }

    stopListenerEventThread(_this);
    deinitListenerThreadInfo(&_this->listenerThreadInfo);

    gapi_mapFree (_this->typeSupportMap);
    _this->typeSupportMap = NULL;

    gapi_publisherQos_free(&_this->_defPublisherQos);
    gapi_subscriberQos_free(&_this->_defSubscriberQos);
    gapi_topicQos_free(&_this->_defTopicQos);

    if ( _this->_DomainId ) {
        os_free(_this->_DomainId);
    }

    p = U_PARTICIPANT_GET(_this);
    _EntityDispose (_Entity(_this));
    u_participantFree(p);

    return result;
}

gapi_boolean
_DomainParticipantPrepareDelete (
    _DomainParticipant  _this,
    const gapi_context *context)
{
    gapi_boolean result = TRUE;
    c_char *name;
    c_iter entities;
    u_entity e;

    assert (_this);
    assert (_ObjectGetKind(_Object(_this)) == OBJECT_KIND_DOMAINPARTICIPANT);

    if (u_participantTopicCount(U_PARTICIPANT_GET(_this)) > 0) {
        OS_REPORT_1(OS_WARNING,
                    "_DomainParticipantPrepareDelete", 0,
                    "Delete Participant 0x%x failed: Some contained Topics still exists",
                    _this);
        entities = u_participantLookupTopics (U_PARTICIPANT_GET(_this), NULL);
        e = c_iterTakeFirst(entities);
        while (e) {
            name = u_topicName(u_topic(e));
            if (name) {
                OS_REPORT_2(OS_WARNING,
                            "_DomainParticipantPrepareDelete", 0,
                            "Delete Participant 0x%x failed because "
                            "Topic '%s' still exists",
                            _this,
                            name);
                os_free(name);
            } else {
                OS_REPORT_2(OS_WARNING,
                            "_DomainParticipantPrepareDelete", 0,
                            "Delete Participant 0x%x failed because "
                            "Topic 'without a name' still exists",
                            _this,
                            name);
            }
            u_entityFree(e);
            e = c_iterTakeFirst(entities);
        }
        c_iterFree(entities);
        result = FALSE;
    }
    if (u_participantPublisherCount(U_PARTICIPANT_GET(_this)) > 0) {
        OS_REPORT_1(OS_WARNING,
                    "_DomainParticipantPrepareDelete", 0,
                    "Delete Participant 0x%x failed: "
                    "Some contained Publishers still exists",
                    _this);
        entities = u_participantLookupPublishers(U_PARTICIPANT_GET(_this));
        e = c_iterTakeFirst(entities);
        while (e) {
            OS_REPORT_2(OS_WARNING,
                        "_DomainParticipantPrepareDelete", 0,
                        "Delete Participant 0x%x failed because "
                        "Publisher 0x%x still exists",
                        _this, e);
            u_entityFree(e);
            e = c_iterTakeFirst(entities);
        }
        c_iterFree(entities);
        result = FALSE;
    }
    if (u_participantSubscriberCount(U_PARTICIPANT_GET(_this)) > 0) {
        OS_REPORT_1(OS_WARNING,
                    "_DomainParticipantPrepareDelete", 0,
                    "Delete Participant 0x%x failed: "
                    "Some contained Subscribers still exists",
                    _this);
        entities = u_participantLookupSubscribers(U_PARTICIPANT_GET(_this));
        e = c_iterTakeFirst(entities);
        while (e) {
            OS_REPORT_2(OS_WARNING,
                        "_DomainParticipantPrepareDelete", 0,
                        "Delete Participant 0x%x failed because "
                        "Subscriber 0x%x still exists",
                        _this, e);
            u_entityFree(e);
            e = c_iterTakeFirst(entities);
        }
        c_iterFree(entities);
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
    _DomainParticipant _this,
    const char *registry_name)
{
    _TypeSupport typeSupport = NULL;
    gapi_mapIter iter;
    C_STRUCT(TypeSupportMapKey) key;

    assert(_this);
    assert(registry_name);

    key.typeAliasName = (gapi_char*)registry_name;
    key.typeName = NULL;
    iter = gapi_mapFind (_this->typeSupportMap, (gapi_object)&key);
    if (iter) {
        typeSupport = (_TypeSupport)gapi_mapIterObject (iter);
        gapi_mapIterFree (iter);
    }
    return typeSupport;
}

u_participant
_DomainParticipantUparticipant (
    _DomainParticipant _this)
{
    u_participant upart = NULL;

    assert(_this);

    upart = U_PARTICIPANT_GET(_this);
    return upart;
}

_Topic
_DomainParticipantFindBuiltinTopic (
    _DomainParticipant _this,
    const gapi_char *topic_name)
{
    c_iter topics;
    _Topic topic = NULL;
    gapi_topic handle;
    u_entity t;

    assert(_this);
    assert(topic_name);

    topics = u_participantFindTopic(U_PARTICIPANT_GET(_this), topic_name, C_TIME_ZERO);
    if (topics) {
        t = c_iterTakeFirst(topics);
        if (t) {
            handle = u_entityGetUserData(t);
            if (handle) {
                topic = _TopicFromHandle(handle);
            } else {
                topic = _TopicFromUserTopic(u_topic(t),_this,NULL);
                if (topic == NULL) {
                    OS_REPORT_1(OS_WARNING,
                                "_DomainParticipantFindBuiltinTopic", 0,
                                "Failed to create GAPI Entity for Topic '%s'.",
                                topic_name);
                }
            }
            u_entityFree(t);
            /* There may be more topics returned that must be freed again.
             */
            t = c_iterTakeFirst(topics);
            while (t) {
                u_entityFree(t);
                t = c_iterTakeFirst(topics);
            }
        }
        c_iterFree(topics);
    }
    return topic;
}

/* precondition: participant must be locked */  /* Need a MACRO to guard this! */
gapi_returnCode_t
_DomainParticipantRegisterType (
    _DomainParticipant _this,
    _TypeSupport typeSupport,
    const gapi_char *registryName)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_context context;
    TypeSupportMapKey key;

    assert(_this);
    assert(typeSupport);
    assert(registryName);

    GAPI_CONTEXT_SET(context, _EntityHandle(_this), GAPI_METHOD_REGISTER_TYPE);
    key = os_malloc(C_SIZEOF(TypeSupportMapKey));
    if(key)
    {
        key->typeAliasName = gapi_strdup(registryName);
        if(key->typeAliasName)
        {
            key->typeName = gapi_strdup(_TypeSupportTypeName(typeSupport));
            if(key->typeName)
            {
                result = gapi_mapAdd(_this->typeSupportMap,
                            (gapi_object)key,
                            (gapi_object)typeSupport);
                if(result != GAPI_RETCODE_OK)
                {
                    os_free(key->typeName);
                    os_free(key->typeAliasName);
                    os_free(key);
                }
            } else
            {
                os_free(key->typeAliasName);
                os_free(key);
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
                OS_REPORT(OS_ERROR,
                    "_DomainParticipantRegisterType", 0,
                    "Unable to duplicate the type name of the typesupport. "
                    "Not enough heap memory available.");
            }
        } else
        {
            os_free(key);
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
            OS_REPORT(OS_ERROR,
                    "_DomainParticipantRegisterType", 0,
                    "Unable to duplicate the registry name of the typesupport. "
                    "Not enough heap memory available.");
        }
    } else
    {
        result = GAPI_RETCODE_OUT_OF_RESOURCES;
        OS_REPORT(OS_ERROR,
                    "_DomainParticipantRegisterType", 0,
                    "Unable to allocate a key holder struct to insert the "
                    "typesupport in the map of typesupports. Not enough heap "
                    "memory available.");
    }
    return result;
}

gapi_boolean
_DomainParticipantContainsTypeSupport (
    _DomainParticipant _this,
    _TypeSupport       typeSupport)
{
    gapi_mapIter iter;
    _TypeSupport ts;
    gapi_boolean contains = FALSE;

    assert(_this);
    assert(typeSupport);

    iter = gapi_mapFirst (_this->typeSupportMap);
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
    _DomainParticipant _this,
    _TypeSupport       typeSupport)
{
    _TypeSupport registered = NULL;
    _TypeSupport ts;
    gapi_mapIter iter;

    assert(_this);
    assert(typeSupport);

    iter = gapi_mapFirst (_this->typeSupportMap);
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
    _DomainParticipant _this,
    _TypeSupport       typeSupport)
{
    gapi_mapIter iter;
    _TypeSupport t;
    const gapi_char *name = NULL;
    TypeSupportMapKey key;

    iter = gapi_mapFirst(_this->typeSupportMap);
    if ( iter ) {
        t = (_TypeSupport)gapi_mapIterObject(iter);
        while ( !name && t ) {
            if ( t == typeSupport ) {
                key = (TypeSupportMapKey) gapi_mapIterKey(iter);
                name = key->typeAliasName;
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
    _DomainParticipant _this,
    const gapi_char *topic_name)
{
    c_iter topics;
    u_topic topic;
    gapi_topicDescription handle;
    _TopicDescription topicDescription = NULL;

    assert(_this);
    assert(topic_name);

    topics = u_participantFindTopic(U_PARTICIPANT_GET(_this),
                                    topic_name,
                                    C_TIME_ZERO);
    topic = c_iterTakeFirst(topics);
    if (topic) {
        handle = u_entityGetUserData(u_entity(topic));
        if (handle) {
            OS_REPORT_2(OS_WARNING,
                        "_DomainParticipantFindTopicDescription", 0,
                        "The newly created User layer Topic '%s' has an unexpected handle 0x%x",
                        topic_name, handle);
        }
        /* create a new gapi _Topic for the new user layer topic. */
        topicDescription = _TopicDescription(_TopicFromUserTopic(topic,_this,NULL));
        _EntityRelease(topicDescription);
        /* A reference is hold by the new gapi _Topic (topicDescription) */
        u_entityFree(u_entity(topic));
        /* There may be more topics returned that must be freed again.
         */
        topic = c_iterTakeFirst(topics);
        while (topic) {
            u_entityFree(u_entity(topic));
            topic = c_iterTakeFirst(topics);
        }
    }
    c_iterFree(topics);

    return topicDescription;
}

static c_equality
compareTopicName (
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    gapi_string name;
    c_equality equality = C_NE;
    _TopicDescription topicDescription = (_TopicDescription) o;
    const gapi_char *topic_name = (const gapi_char *) arg;

    name = _TopicDescriptionGetName(topicDescription);
    if (name && topic_name) {
        if (strcmp(name, topic_name) == 0) {
            equality = C_EQ;
        }
    }
    gapi_free(name);
    return equality;
}

/* precondition: participant must be locked */
_TopicDescription
_DomainParticipantLookupTopicDescription (
    _DomainParticipant _this,
    const gapi_char *topic_name)
{
    c_iter topics;
    u_topic topic;
    gapi_topicDescription handle;
    _TopicDescription topicDescription = NULL;

    assert(_this);
    assert(topic_name);

    topicDescription = (_TopicDescription) c_iterResolve(
            _this->contentFilteredTopics,
            compareTopicName,
            (void *) topic_name);
    if (topicDescription == NULL) {
        topics = u_participantLookupTopics(U_PARTICIPANT_GET(_this),
                                           topic_name);
        topic = c_iterTakeFirst(topics);
        if (topic) {
            handle = u_entityGetUserData(u_entity(topic));
            if (handle) {
                topicDescription = _TopicDescriptionFromHandle(handle);
            } else {
                OS_REPORT_1(OS_WARNING,
                            "_DomainParticipantLookupTopicDescription", 0,
                            "Found User layer Topic '%s' without a "
                            "reference to a gapi topic.",
                            topic_name);
            }
            u_entityFree(u_entity(topic));
            /* There may be more topics returned that must be freed again.
             */
            topic = c_iterTakeFirst(topics);
            while (topic) {
                u_entityFree(u_entity(topic));
                topic = c_iterTakeFirst(topics);
            }
        }
        c_iterFree(topics);
    }
    return topicDescription;
}

gapi_boolean
_DomainParticipantTopicDescriptionExists (
    _DomainParticipant _this,
    _TopicDescription  topicDescription)
{
    gapi_boolean      result = FALSE;

    assert(_this);
    assert(topicDescription);

    result = u_participantContainsTopic(U_PARTICIPANT_GET(_this),
                                        U_TOPIC_GET(topicDescription));
    return result;
}

gapi_domainParticipantQos *
_DomainParticipantGetQos (
    _DomainParticipant _this,
    gapi_domainParticipantQos * qos)
{
    v_participantQos participantQos;
    u_participant uParticipant;

    assert(_this);

    uParticipant = U_PARTICIPANT_GET(_this);

    if ( u_entityQoS(u_entity(uParticipant), (v_qos*)&participantQos) == U_RESULT_OK ) {
        copyParticipantQosOut(participantQos,  qos);
        u_participantQosFree(participantQos);

        qos->watchdog_scheduling = _this->watchdogScheduling;
        qos->listener_scheduling = _this->listenerThreadInfo.scheduling;
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
    gapi_returnCode_t result;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_PUBLISHER);

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        if ( qos == GAPI_PUBLISHER_QOS_DEFAULT ) {
            qos = &participant->_defPublisherQos;
        }
        if ( gapi_publisherQosIsConsistent(qos, &context) == GAPI_RETCODE_OK ) {
            publisher = _PublisherNew(U_PARTICIPANT_GET(participant),
                                      qos,
                                      a_listener,
                                      mask,
                                      participant);
            if ( publisher != NULL ) {
                _ENTITY_REGISTER_OBJECT(_Entity(participant), (_Object)publisher);
            }
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_create_publisher", 0,
                        "Given QoS Policy is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_create_publisher", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

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
    c_bool contains;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant ) {
        publisher = gapi_publisherClaimNB(p, &result);

        if ( publisher ) {
            contains = u_participantContainsPublisher(U_PARTICIPANT_GET(participant),
                                                      U_PUBLISHER_GET(publisher));
            if (contains) {
                if (_PublisherWriterCount(publisher) > 0) {
                    OS_REPORT_1(OS_WARNING,
                                "gapi_domainParticipant_delete_publisher", 0,
                                "Operation failed: %d DataWriters exists",
                                _PublisherWriterCount(publisher));
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
                if (result == GAPI_RETCODE_OK) {
                    result = _PublisherFree(publisher);
                    publisher = NULL;
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                OS_REPORT(OS_WARNING,
                          "gapi_domainParticipant_delete_publisher", 0,
                          "Operation failed: Publisher is not a contained entity.");
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            _EntityRelease(publisher);
        } else {
            /* publisher is not valid */
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_delete_publisher", 0,
                        "Given Publisher is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_delete_publisher", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_SUBSCRIBER);

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        if (qos == GAPI_SUBSCRIBER_QOS_DEFAULT) {
            qos = &participant->_defSubscriberQos;
        }
        result = gapi_subscriberQosIsConsistent(qos, &context);
        if ( result == GAPI_RETCODE_OK ) {
            subscriber = _SubscriberNew(U_PARTICIPANT_GET(participant),
                                        qos,
                                        a_listener,mask,
                                        participant);
            if (subscriber != NULL) {
                _ENTITY_REGISTER_OBJECT(_Entity(participant), (_Object)subscriber);
            }
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_create_subscriber", 0,
                        "Given QoS Policy is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_create_subscriber", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

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
    c_bool contains;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant ) {
        subscriber = gapi_subscriberClaimNB(s, &result);
        if ( subscriber ) {
            contains = u_participantContainsSubscriber(U_PARTICIPANT_GET(participant),
                                                       U_SUBSCRIBER_GET(subscriber));
            if (contains) {
                if (subscriber == participant->builtinSubscriber) {
                    participant->builtinSubscriber = NULL;
                    _SubscriberDeleteContainedEntities(subscriber);
                } else if (_SubscriberReaderCount(subscriber) > 0) {
                    OS_REPORT_1(OS_WARNING,
                                "gapi_domainParticipant_delete_subscriber", 0,
                                "Operation failed: %d DataReaders exists",
                                _SubscriberReaderCount(subscriber));
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
                if (result == GAPI_RETCODE_OK) {
                    result = _SubscriberFree(subscriber);
                    subscriber = NULL;
                }
            } else {
                OS_REPORT(OS_WARNING,
                          "gapi_domainParticipant_delete_subscriber", 0,
                          "Operation failed: Subscriber is not a contained entity.");
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            _EntityRelease(subscriber);
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_delete_subscriber", 0,
                        "Given Subscriber is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_delete_subscriber", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        if ( _EntityEnabled(participant)) {
            if ( participant->builtinSubscriber == NULL ) {
                participant->builtinSubscriber = _BuiltinSubscriberNew (
                    U_PARTICIPANT_GET(participant), participant->_Factory, participant);
                _ENTITY_REGISTER_OBJECT(_Entity(participant),
                                        (_Object)participant->builtinSubscriber);
                _EntityRelease(participant->builtinSubscriber);
            }
            subscriber = participant->builtinSubscriber;
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_builtin_subscriber", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

    return (gapi_subscriber)_EntityHandle(subscriber);
}

_Subscriber
_DomainParticipantGetBuiltinSubscriber (
    _DomainParticipant _this)
{
    assert(_this);

    return _this->builtinSubscriber;
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
    _TypeSupport       typeSupport = NULL;
    gapi_string        typeName    = NULL;
    gapi_topic         topic       = NULL;
    gapi_context       context;
    gapi_returnCode_t result;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_TOPIC);

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        if ( (topic_name != NULL) && (registered_type_name != NULL) ) {
            if ( qos == GAPI_TOPIC_QOS_DEFAULT ) {
                qos = &participant->_defTopicQos;
            }
            typeSupport = _DomainParticipantFindType(participant,
                                                     (const char*)registered_type_name);
            if ( typeSupport != NULL ) {
                typeName = _TypeSupportTypeName(typeSupport);
            }
        }
    } else {
        OS_REPORT_1(OS_API_INFO,
                    "gapi_domainParticipant_create_topic", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

    if ( typeSupport != NULL ) {
        newTopic = _TopicNew(topic_name, registered_type_name,
                             typeSupport, qos, a_listener, mask,
                             participant, &context);
        if (newTopic) {
            _ENTITY_REGISTER_OBJECT(_Entity(participant), (_Object)newTopic);
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

    _EntityRelease(participant);

    if ( newTopic ) {
        gapi_object statusHandle;
        statusHandle = _EntityHandle(_Entity(newTopic)->status);
        topic = (gapi_topic)_EntityRelease(newTopic);
    }

    return topic;
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
    c_bool contains;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant != NULL ) {
        topic = gapi_topicClaimNB(a_topic, &result);
        if ( topic != NULL ) {
            contains = u_participantContainsTopic(U_PARTICIPANT_GET(participant),
                                                  U_TOPIC_GET(topic));
            if (contains) {
                if ( _TopicPrepareDelete (topic) ) {
                    /* The following ref count usage is not bullet proof.
                     * Will be solved by future handle server.
                     */
                    c_long count = _TopicRefCount(topic);
                    _TopicFree(topic);
                    if (count == 1) {
                        topic = NULL;
                    }
                } else {
                    /* topic is stil in use */
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            _EntityRelease(topic);
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_delete_topic", 0,
                        "Given Topic is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_delete_topic", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    _DomainParticipant participant = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Topic topic = NULL;
    c_iter topicList;
    v_duration vDuration;
    u_topic uTopic = NULL;
    char *typeName = NULL;
    gapi_context context;
    u_participant uParticipant;
    _TypeSupport typeSupport;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_FIND_TOPIC);

    if (!topic_name || !gapi_validDuration(timeout)) {
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_find_topic", 0,
                    "Given topic name or timeout is invalid.",
                    gapi_retcode_image(result));
    } else {
        participant = gapi_domainParticipantClaim(_this, &result);
    }
    if ( result == GAPI_RETCODE_OK ) {
         if (_EntityEnabled(participant)) {
            kernelCopyInDuration(timeout, &vDuration);
            uParticipant = _DomainParticipantUparticipant(participant);
            /* release the participant lock because with an infinite timeout not releasing
             * the participant results in a deadlock because no other thread can access the participant object */
            _EntityRelease(participant);
            topicList = u_participantFindTopic (uParticipant,
                                                topic_name, vDuration);

            if (topicList) {
                u_topic t;
                uTopic = c_iterTakeFirst(topicList);
                /* OSPL-103: It's allowed to have a topic without the
                   corresponding type support. It's however not allowed to have
                   a topic with a faulty type support (AKA two different types
                   under the same name). */
                typeName = (char *)u_topicTypeName(uTopic);
                typeSupport = _DomainParticipantFindType (participant, typeName);
                if (typeSupport) {
                    /* type_name verified if reached. Meta data matches, verify
                       key list matches too or disregard uTopic. */
                    c_string keys;
                    keys = u_topicGetTopicKeys (uTopic);
                    if (keys) {
                        if (strcmp (typeSupport->type_keys, keys) != 0) {
                            OS_REPORT_2 (OS_API_INFO,
                                "gapi_domainParticipant_find_topic", 0,
                                "keys in topic %s do not match keys defined in type %s",
                                topic_name, typeName);
                            u_topicFree (uTopic);
                            uTopic = NULL;
                        }
                        c_free (keys);
                    }
                    /* don't free typeSupport here */
                }

                t = c_iterTakeFirst(topicList);
                while(t) {
                    u_topicFree(t);
                    t = c_iterTakeFirst(topicList);
                }
                c_iterFree(topicList);
            }

            participant = gapi_domainParticipantClaim(_this, &result);
            if ( result == GAPI_RETCODE_OK ) {
                if (uTopic && typeName) {
                    topic = _TopicFromKernelTopic(uTopic, topic_name,
                                                  typeName,
                                                  participant, &context);
                    if ( !topic ) {
                        u_topicFree(uTopic);
                    }
                }
            }  else {
                 OS_REPORT_1(OS_WARNING,
                             "gapi_domainParticipant_find_topic", 0,
                             "Given DomainParticipant is invalid: result = %s",
                             gapi_retcode_image(result));
            }
            if ( topic ) {
                _ENTITY_REGISTER_OBJECT(_Entity(participant),
                                        (_Object)topic);
            }
        } else {
            OS_REPORT(OS_WARNING,
                      "gapi_domainParticipant_find_topic", 0,
                      "Given DomainParticipant is not enabled.");
        }
        if ( typeName ) {
            os_free (typeName);
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_find_topic", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant ) {
        if ( _EntityEnabled(participant) && name ) {
            topicDescription = _DomainParticipantLookupTopicDescription(participant, name);
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_lookup_topicdescription", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    _DomainParticipant p;
    gapi_boolean licensed;
    _ContentFilteredTopic newTopic = NULL;
    _Topic related = NULL;
    _Topic found = NULL;
    c_iter topics;
    u_topic topic;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    licensed = _DomainParticipantFactoryIsContentSubscriptionAvailable();
    if (licensed != TRUE) {
        OS_REPORT(OS_WARNING,
                  "gapi_domainParticipant_create_contentfilteredtopic", 0,
                  "Cannot create ContentFilteredTopic: No license.");
        return NULL;
    }
    if (name == NULL) {
        OS_REPORT(OS_WARNING,
                  "gapi_domainParticipant_create_contentfilteredtopic", 0,
                  "Given name is invalid. name = NULL");
        return NULL;
    }
    if (filter_expression == NULL) {
        OS_REPORT(OS_WARNING,
                  "gapi_domainParticipant_create_contentfilteredtopic", 0,
                  "Given filter expression is invalid. expression = NULL");
        return NULL;
    }
    if (!gapi_sequence_is_valid(filter_parameters))
    {
        OS_REPORT(OS_WARNING,
                  "gapi_domainParticipant_create_contentfilteredtopic", 0,
                  "Given parameter sequence is invalid.");
        return NULL;
    }

    p = gapi_domainParticipantClaim(_this, &result);
    if ( p != NULL ) {
        found = (_Topic) c_iterResolve(p->contentFilteredTopics,
                                       compareTopicName,
                                       (void *) name);
        if (found == NULL) {
            topics = u_participantLookupTopics(U_PARTICIPANT_GET(p), name);
            topic = c_iterTakeFirst(topics);
            if (topic == NULL) {
                related = gapi_topicClaim(related_topic, &result);
                if (related != NULL) {
                    newTopic = _ContentFilteredTopicNew(name,
                                                        related,
                                                        filter_expression,
                                                        filter_parameters,
                                                        p);
                    if ( newTopic != NULL ) {
                        p->contentFilteredTopics =
                            c_iterInsert(p->contentFilteredTopics, newTopic);
                        _ENTITY_REGISTER_OBJECT(_Entity(p),
                                                (_Object)newTopic);
                    }
                    _EntityRelease(related);
                } else {
                    OS_REPORT_1(OS_WARNING,
                                "gapi_domainParticipant_create_contentfilteredtopic", 0,
                                "Cannot resolve related topic: result = %s",
                                 gapi_retcode_image(result));
                }
            }
            while (topic) {
                u_entityFree(u_entity(topic));
                topic = c_iterTakeFirst(topics);
            }
            c_iterFree(topics);
        } else {
            OS_REPORT(OS_WARNING,
                      "gapi_domainParticipant_create_contentfilteredtopic", 0,
                      "Given ContentFilteredTopic name already exists.");
        }
        _EntityRelease(p);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_create_contentfilteredtopic", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
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
    _ContentFilteredTopic topic = NULL;
    _ContentFilteredTopic found;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant != NULL ) {
        topic = gapi_contentFilteredTopicClaim(a_contentfilteredtopic, &result);
        if ( topic != NULL ) {
            if ( _ContentFilteredTopicPrepareDelete(topic) ) {
                found = c_iterTake(participant->contentFilteredTopics, topic);
                if (found == topic) {
                    _ContentFilteredTopicFree(topic);
                    topic = NULL;
                } else {
                    OS_REPORT(OS_WARNING,
                              "gapi_domainParticipant_delete_contentfilteredtopic", 0,
                              "Given ContentFilteredTopic is invalid");
                    result = GAPI_RETCODE_BAD_PARAMETER;
                }
            } else {
                /* topic is stil in use */
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            _EntityRelease(topic);
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_delete_contentfilteredtopic", 0,
                        "Given ContentFilteredTopic is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_delete_contentfilteredtopic", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_delete_multitopic", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_delete_historical_data", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
    return result;

}

static c_bool
collect_entities (
    c_object o,
    c_voidp arg)
{
    u_entity entity = (u_entity)o;
    c_iter *iter = (c_iter *)arg;
    u_participant p;
    u_subscriber s;
    c_bool builtin = FALSE;

    if (u_entityKind(entity) == U_SUBSCRIBER) {
        p = u_entityParticipant(entity);
        s = u_subscriber(entity);
        builtin = u_participantIsBuiltinSubscriber(p,s);
    }
    if (!builtin) {
        *iter = c_iterInsert(*iter,entity);
    }
    return TRUE;
}

gapi_returnCode_t
_DomainParticipantDeleteContainedEntitiesNoClaim (
    _DomainParticipant _this)
{
    gapi_publisher handle;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Publisher publisher;
    _Subscriber subscriber;
    _Topic topic;
    c_iter entities;
    u_entity entity;
    gapi_context context;
    /* u_result uResult; Should be added to test results from user-layer calls. */

    if(_this)
    {
        GAPI_CONTEXT_SET(context, _EntityHandle(_this), GAPI_METHOD_DELETE_CONTAINED_ENTITIES);
#if 1
/* This part can be replaced by the '#else' part but needs bugfixes (only user entities are deleted). */
        entities = NULL;
        u_participantWalkSubscribers(U_PARTICIPANT_GET(_this),
                                     (u_subscriberAction)collect_entities,&entities);
        entity = c_iterTakeFirst(entities);
        while ((entity) && (result == GAPI_RETCODE_OK)) {
            handle = u_entityGetUserData(entity);
            if (handle) {
                result = gapi_subscriber_delete_contained_entities(handle);
                if (result == GAPI_RETCODE_OK){
                    subscriber = gapi_subscriberClaimNB(handle,&result);
                    if (subscriber) {
                        _SubscriberFree(subscriber);
                    }
                } else if (result == GAPI_RETCODE_ALREADY_DELETED) {
                    result = GAPI_RETCODE_OK;
                }
            } else {
                OS_REPORT_1(OS_INFO,
                            "_DomainParticipantDeleteContainedEntitiesNoClaim", 0,
                            "Found User layer Subscriber 0x%x has no valid "
                            "GAPI Subscriber handle (NULL)",
                            entity);
            }
            entity = c_iterTakeFirst(entities);
        }
        c_iterFree(entities);

        entities = NULL;
        u_participantWalkPublishers(U_PARTICIPANT_GET(_this),
                                    (u_publisherAction)collect_entities,&entities);
        entity = c_iterTakeFirst(entities);
        while ((entity) && (result == GAPI_RETCODE_OK)) {
            handle = u_entityGetUserData(entity);
            if (handle) {
                result = gapi_publisher_delete_contained_entities(handle);
                if (result == GAPI_RETCODE_OK) {
                    publisher = gapi_publisherClaimNB(handle,&result);
                    if (publisher) {
                        _PublisherFree(publisher);
                    }
                } else if (result == GAPI_RETCODE_ALREADY_DELETED) {
                    result = GAPI_RETCODE_OK;
                }
            } else {
                OS_REPORT_1(OS_INFO,
                            "_DomainParticipantDeleteContainedEntitiesNoClaim", 0,
                            "Found User layer Publisher 0x%x has no valid "
                            "GAPI Publisher handle (NULL)",
                            entity);
            }
            entity = c_iterTakeFirst(entities);
        }
        c_iterFree(entities);

        entities = NULL;
        u_participantWalkTopics(U_PARTICIPANT_GET(_this),
                                (u_topicAction)collect_entities,&entities);
        entity = c_iterTakeFirst(entities);
        while ((entity) && (result == GAPI_RETCODE_OK)) {
            handle = u_entityGetUserData(entity);
            if (handle) {
                topic = _Topic(gapi_entityClaimNB(handle,&result));
                if (result == GAPI_RETCODE_OK) {
                    c_long count = _TopicRefCount(topic);
                    switch (_ObjectGetKind(_Object(topic))) {
                    case OBJECT_KIND_TOPIC:
                        result = _TopicFree(topic);
                    break;
                    case OBJECT_KIND_CONTENTFILTEREDTOPIC:
                        _ContentFilteredTopicFree(_ContentFilteredTopic(topic));
                    break;
                    default:
                        assert(FALSE);
                        result = GAPI_RETCODE_BAD_PARAMETER;
                    break;
                    }
                    if ((result == GAPI_RETCODE_OK) && (count > 1)) {
                        _EntityRelease(_Entity(topic));
                    }
                } else if (result == GAPI_RETCODE_ALREADY_DELETED) {
                    result = GAPI_RETCODE_OK;
                }
            } else {
                OS_REPORT_1(OS_INFO,
                            "_DomainParticipantDeleteContainedEntitiesNoClaim", 0,
                            "Found User layer Topic 0x%x has no valid "
                            "GAPI Topic handle (NULL)",
                            entity);
            }
            entity = c_iterTakeFirst(entities);
        }
        c_iterFree(entities);
#else
        uResult = u_participantDeleteContainedEntities(U_PARTICIPANT_GET(_this));
        result = kernelResultToApiResult(uResult);
#endif
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}


/*     ReturnCode_t
 *     delete_contained_entities();
 */
gapi_returnCode_t
gapi_domainParticipant_delete_contained_entities (
    gapi_domainParticipant _this)
{
    gapi_returnCode_t result;
    _DomainParticipant participant;

    participant = gapi_domainParticipantClaim(_this, &result);
    if (participant) {
        result = _DomainParticipantDeleteContainedEntitiesNoClaim(participant);
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_delete_contained_entities", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
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
    _DomainParticipant participant = NULL;
    v_participantQos participantQos;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_QOS);

    if (qos) {
        participant = gapi_domainParticipantClaim(_this, &result);
        if (participant) {
            result = gapi_domainParticipantQosIsConsistent(qos, &context);
            if (result != GAPI_RETCODE_OK) {
                OS_REPORT_1(OS_WARNING,
                            "gapi_domainParticipant_set_qos", 0,
                            "Given QoS Policy is invalid: result = %s",
                            gapi_retcode_image(result));
            }
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_set_qos", 0,
                        "Given DomainParticipant is invalid: result = %s",
                        gapi_retcode_image(result));
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_set_qos", 0,
                    "Given QoS Policy is invalid: result = %s",
                    gapi_retcode_image(result));
    }

    if ( result == GAPI_RETCODE_OK ) {
        gapi_domainParticipantQos *existing_qos = gapi_domainParticipantQos__alloc();

        if (existing_qos) {
            result = gapi_domainParticipantQosCheckMutability(
                         qos,
                         _DomainParticipantGetQos(participant, existing_qos),
                         &context);
            gapi_free(existing_qos);
            if (result != GAPI_RETCODE_OK) {
                OS_REPORT_1(OS_WARNING,
                            "gapi_domainParticipant_set_qos", 0,
                            "Given QoS Policy is invalid: result = %s",
                            gapi_retcode_image(result));
            }
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
            OS_REPORT_1(OS_ERROR,
                        "gapi_domainParticipant_set_qos", 0,
                        "Operation failed: result = %s",
                        gapi_retcode_image(result));
        }
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
                } else {
                    OS_REPORT_1(OS_WARNING,
                                "gapi_domainParticipant_set_qos", 0,
                                "Operation u_entitySetQoS failed: result = %s",
                                gapi_retcode_image(result));
                }
                u_participantQosFree(participantQos);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
                OS_REPORT_1(OS_ERROR,
                            "gapi_domainParticipant_set_qos", 0,
                            "Operation failed: result = %s",
                             gapi_retcode_image(result));
            }
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
            OS_REPORT_1(OS_ERROR,
                        "gapi_domainParticipant_set_qos", 0,
                        "Operation failed: result = %s",
                         gapi_retcode_image(result));
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

    if (qos) {
        participant = gapi_domainParticipantClaim(_this, &result);
        if (participant) {
            _DomainParticipantGetQos(participant, qos);
            _EntityRelease(participant);
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_get_qos", 0,
                        "Given DomainParticipant is invalid: result = %s",
                        gapi_retcode_image(result));
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_qos", 0,
                    "Given QoS Policy is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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

    if (participant) {
        _Status status;

        if ( a_listener ) {
            participant->_Listener = *a_listener;
        } else {
            memset(&participant->_Listener, 0, sizeof(participant->_Listener));
        }

        status = _EntityStatus(participant);
        if ( _StatusSetListener(status,
                                (struct gapi_listener *)a_listener,
                                mask) )
        {
            result = GAPI_RETCODE_OK;
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_set_listener", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    gapi_returnCode_t result = GAPI_RETCODE_ERROR;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL ) {
        listener = participant->_Listener;
        _EntityRelease(participant);
    } else {
        memset(&listener, 0, sizeof(listener));
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_listener", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
        if ( _EntityEnabled(participant) ) {
            result = GAPI_RETCODE_UNSUPPORTED;
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_ignore_participant", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
        if ( _EntityEnabled(participant) ) {
            result = GAPI_RETCODE_UNSUPPORTED;
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_ignore_topic", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
        if ( _EntityEnabled(participant) ) {
            result = GAPI_RETCODE_UNSUPPORTED;
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_ignore_publication", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
        if ( _EntityEnabled(participant) ) {
            result = GAPI_RETCODE_UNSUPPORTED;
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_ignore_subscription", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
    return result;
}

/*     gapi_domainId_int_t
 *     get_domain_id_w_id();
 */
gapi_domainId_int_t
gapi_domainParticipant_get_domain_id (
    gapi_domainParticipant _this)
{
    _DomainParticipant participant;
    gapi_domainId_int_t domainId = INVALID_DOMAIN_ID;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL) {
        if (_EntityEnabled(participant)) {
            if (participant->_dId == INVALID_DOMAIN_ID) {
                domainId = u_userGetDomainIdFromEnvUri();
            } else {
                domainId = participant->_dId;
            }
        } else {
            OS_REPORT(OS_WARNING,
                      "gapi_domainParticipant_get_domain_id", 0,
                      "Given DomainParticipant is not enabled.");
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_domain_id", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
    return domainId;
}

/*     DomainId_t
 *     get_domain_id_as_str();
 *
 *     Note : the caller must free the returned value
 */
gapi_domainName_t
gapi_domainParticipant_get_domain_id_as_str (
    gapi_domainParticipant _this)
{
    _DomainParticipant participant;
    gapi_domainName_t domainId = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    participant = gapi_domainParticipantClaim(_this, &result);

    if ( participant != NULL) {
        if (_EntityEnabled(participant)) {
            if ( participant->_DomainId ) {
                domainId = gapi_string_dup(participant->_DomainId);
            }
        } else {
            OS_REPORT(OS_WARNING,
                      "gapi_domainParticipant_get_domain_id_as_str", 0,
                      "Given DomainParticipant is not enabled.");
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_domain_id_as_str", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    u_result uResult;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant != NULL) {
        if (_EntityEnabled(participant)) {
            uResult = u_participantAssertLiveliness(U_PARTICIPANT_GET(participant));
            result = kernelResultToApiResult(uResult);
        } else {
            OS_REPORT(OS_WARNING,
                      "gapi_domainParticipant_assert_liveliness", 0,
                      "Given DomainParticipant is not enabled.");
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_assert_liveliness", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    if (result == GAPI_RETCODE_OK) {
        if (qos == GAPI_PUBLISHER_QOS_DEFAULT) {
            qos = &gapi_publisherQosDefault;
        }
        result = gapi_publisherQosIsConsistent(qos, &context);
        if (result == GAPI_RETCODE_OK) {
            gapi_publisherQosCopy (qos, &participant->_defPublisherQos);
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_set_default_publisher_qos", 0,
                        "Given QoS Policy is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_set_default_publisher_qos", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    if (result == GAPI_RETCODE_OK) {
        if (qos) {
            gapi_publisherQosCopy (&participant->_defPublisherQos, qos);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_get_default_publisher_qos", 0,
                        "Given QoS Policy is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_default_publisher_qos", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

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
    if (result == GAPI_RETCODE_OK) {
        if (qos == GAPI_SUBSCRIBER_QOS_DEFAULT) {
            qos = &gapi_subscriberQosDefault;
        }
        result = gapi_subscriberQosIsConsistent(qos, &context);
        if (result == GAPI_RETCODE_OK) {
            gapi_subscriberQosCopy (qos, &participant->_defSubscriberQos);
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_set_default_subscriber_qos", 0,
                        "Given QoS Policy is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_set_default_subscriber_qos", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

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
    if (result == GAPI_RETCODE_OK) {
        if (qos) {
            gapi_subscriberQosCopy (&participant->_defSubscriberQos, qos);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_get_default_subscriber_qos", 0,
                        "Given QoS Policy is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_default_subscriber_qos", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

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
    if (result == GAPI_RETCODE_OK) {
        if (qos == GAPI_TOPIC_QOS_DEFAULT) {
            qos = &gapi_topicQosDefault;
        }
        result = gapi_topicQosIsConsistent(qos, &context);
        if (result == GAPI_RETCODE_OK) {
            gapi_topicQosCopy (qos, &participant->_defTopicQos);
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_set_default_topic_qos", 0,
                        "Given QoS Policy is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_set_default_topic_qos", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

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
    if (result == GAPI_RETCODE_OK) {
        if (qos) {
            gapi_topicQosCopy (&participant->_defTopicQos, qos);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_get_default_topic_qos", 0,
                        "Given QoS Policy is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_default_topic_qos", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

    return result;
}

gapi_domainName_t
_DomainParticipantGetDomainId (
    _DomainParticipant _this)
{
    assert(_this);

    return _this->_DomainId;
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
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim(_this, &result);
    if ( participant ) {
        if ( type_name ) {
            typeDescription = _DomainParticipant_get_type_metadescription(participant,type_name);
        } else {
            OS_REPORT(OS_WARNING,
                      "gapi_domainParticipant_get_type_metadescription", 0,
                      "Given type name = <NULL>");
        }
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_type_metadescription", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim (_this, &result);
    if (participant) {
        if ( type_name ) {
            typeSupport = (gapi_typeSupport)_EntityHandle(_DomainParticipantFindType (participant, type_name));
        } else {
            OS_REPORT(OS_WARNING,
                      "gapi_domainParticipant_get_typesupport", 0,
                      "Given type name = <NULL>");
        }
        _EntityRelease (participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_typesupport", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
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
    gapi_returnCode_t result;

    participant = gapi_domainParticipantClaim (_this, &result);
    if (participant) {
        if ( type_name ) {
            typeSupport = (gapi_typeSupport)_EntityHandle(_DomainParticipantFindType(participant, type_name));
        } else {
            OS_REPORT(OS_WARNING,
                      "gapi_domainParticipant_lookup_typesupport", 0,
                      "Given type name = <NULL>");
        }
        _EntityRelease (participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_lookup_typesupport", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }

    return typeSupport;
}

void
_DomainParticipantGetListenerActionInfo (
    _DomainParticipant _this,
    gapi_listenerThreadAction *startAction,
    gapi_listenerThreadAction *stopAction,
    void                      **actionArg)
{
    assert(_this);
    assert(startAction);
    assert(stopAction);
    assert(actionArg);

    *startAction = _this->listenerThreadInfo.startAction;
    *stopAction  = _this->listenerThreadInfo.stopAction;
    *actionArg   = _this->listenerThreadInfo.actionArg;
}

static void *
listenerEventThread (
    void *arg)
{
    ListenerThreadInfo *info = (ListenerThreadInfo *) arg;
    c_iter list = NULL;
    gapi_object handle;
    u_result result;

    if ( info->startAction ) {
        info->startAction(info->actionArg);
    }

    os_mutexLock(&info->mutex);

    if ( info->threadState == STARTING )
    {
       info->threadState = RUNNING;
       os_condBroadcast(&info->cond);
    }

    while ( info->threadState == RUNNING ) {
        handle = c_iterTakeFirst(info->toAddList);
        while (handle) {
           _Entity entity;
            /* ES: The info->mutex can not be claimed before the gapi entity
             * can be claimed as this violates the locking strategy within
             * the gapi. So we will unlock the mutex while we have the entity
             * claim.
             */
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
            handle = c_iterTakeFirst(info->toAddList);
        }
        os_mutexUnlock(&info->mutex);

        result = u_waitsetWaitEvents(info->waitset,&list);
        if(result == U_RESULT_OK || result == U_RESULT_TIMEOUT)
        {
            if ( list ) {
                u_waitsetEvent event = u_waitsetEvent(c_iterTakeFirst(list));
                while ( event) {
                    if(!(event->events & V_EVENT_TRIGGER))
                    {
                        u_entity uEntity = event->entity;
                        gapi_entity source = (gapi_entity)(u_entityGetUserData(uEntity));
                        gapi_entityNotifyEvent(source, event->events);
                    }
                    u_waitsetEventFree(event);
                    event = u_waitsetEvent(c_iterTakeFirst(list));
                }
                c_iterFree(list);
                list = NULL;
            }
        } else
        {
            OS_REPORT_1(OS_ERROR,
                      "listenerEventThread", 0,
                      "u_waitsetWaitEvents failed with result code %d", result);
            info->threadState = STOPPING;
        }
        os_mutexLock(&info->mutex);
    }
    info->threadState = STOPPED;
    os_condBroadcast(&info->cond);
    os_mutexUnlock(&info->mutex);

    if ( info->stopAction ) {
        info->stopAction(info->actionArg);
    }

    return NULL;
}


static gapi_boolean
startListenerEventThread (
    _DomainParticipant _this)
{
    ListenerThreadInfo *info;
    os_threadAttr osThreadAttr;
    os_result     osResult;
    gapi_boolean  result = FALSE;

    info = &_this->listenerThreadInfo;

    info->threadState = STARTING;
    os_condBroadcast(&info->cond);

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
            OS_REPORT(OS_ERROR, "startListenerEventThread", 0,
                      "failed to start listener thread");
        }
    } else {
        OS_REPORT(OS_ERROR, "startListenerEventThread", 0,
                  "failed to init thread attributes");
    }

    return result;
}

static gapi_boolean
stopListenerEventThread (
    _DomainParticipant _this)
{
    ListenerThreadInfo *info;
    gapi_boolean        result = FALSE;
    gapi_returnCode_t gResult = GAPI_RETCODE_OK;
    os_result waitResult;

    info = &_this->listenerThreadInfo;

    os_mutexLock(&info->mutex);
    switch ( info->threadState )
    {
       case STARTING:
       {
          /* Someone is stopping the listener before its really started */
          info->threadState = STOPPING;
          os_condBroadcast(&info->cond);
          do
          {
             waitResult = os_condWait(&info->cond, &info->mutex );
             if (waitResult == os_resultFail)
             {
                OS_REPORT(OS_CRITICAL, "stopListenerEventThread", 0,
                          "os_condWait failed - initial state STARTING");
                os_mutexUnlock(&info->mutex);
                return result;
             }
          } while ( info->threadState != STOPPED );
          os_mutexUnlock(&info->mutex);

          result = TRUE;
          break;
       }
       case RUNNING:
       {
          info->threadState = STOPPING;
          os_condBroadcast(&info->cond);
          u_waitsetNotify(info->waitset, NULL);
          os_mutexUnlock(&info->mutex);
          _EntityRelease(_this);
          os_threadWaitExit(info->threadId, NULL);
          (void)gapi_domainParticipantClaimNB(_ObjectToHandle(_Object(_this)),
                                        &gResult);
          if(gResult != GAPI_RETCODE_OK)
          {
             OS_REPORT(OS_WARNING, "stopListenerEventThread", 0,
                       "failed to reclaim participant");
          }
          result = TRUE;
          break;
       }
       case STOPPING:
       {
          do
          {
             waitResult = os_condWait(&info->cond, &info->mutex );
             if (waitResult == os_resultFail)
             {
                OS_REPORT(OS_CRITICAL, "stopListenerEventThread", 0,
                          "os_condWait failed - initial state STOPPING");
                os_mutexUnlock(&info->mutex);
                return result;
             }
          } while ( info->threadState != STOPPED );
          result = TRUE;
          os_mutexUnlock(&info->mutex);
          break;
       }
       default:
       {
          os_mutexUnlock(&info->mutex);
       }
    }

    return result;
}

gapi_boolean
_DomainParticipantAddListenerInterest (
    _DomainParticipant _this,
    _Status            status)
{
    ListenerThreadInfo *info;
    gapi_boolean        result = FALSE;
    os_result waitResult;

    info = &_this->listenerThreadInfo;

    os_mutexLock(&info->mutex);

    switch ( info->threadState )
    {
       case RUNNING:
       case STARTING:
       {
          info->toAddList = c_iterInsert(info->toAddList,
                                         _EntityHandle(status->entity));
          u_waitsetNotify(info->waitset, NULL);
          break;
       }
       case STOPPING:
       case STOPPED:
       {
          while ( info->threadState == STOPPING )
          {
             waitResult = os_condWait(&info->cond, &info->mutex );
             if (waitResult == os_resultFail)
             {
                OS_REPORT(OS_CRITICAL, "_DomainParticipantAddListenerInterest", 0,
                          "os_condWait failed - initial state STOPPING / STOPPED");
                os_mutexUnlock(&info->mutex);
                return result;
             }
          };
          info->toAddList = c_iterInsert(info->toAddList,
                                         _EntityHandle(status->entity));
          startListenerEventThread(_this);
          break;
       }
    }

    os_mutexUnlock(&info->mutex);

    return result;
}


gapi_boolean
_DomainParticipantRemoveListenerInterest (
    _DomainParticipant _this,
    _Status            status)
{
    ListenerThreadInfo *info;
    gapi_boolean        result = FALSE;

    info = &_this->listenerThreadInfo;


    os_mutexLock(&info->mutex);
    if ( info->waitset ) {

        if ( info->threadState == RUNNING ) {
            if ( u_waitsetDetach(info->waitset, status->userEntity) == U_RESULT_OK ) {
                result = TRUE;
            }
        }
    }
    os_mutexUnlock(&info->mutex);
    return result;
}


/*     ReturnCode_t
 *     get_discovered_participants (
 *         inout InstanceHandleSeq participant_handles);
 */
gapi_returnCode_t
gapi_domainParticipant_get_discovered_participants (
    gapi_domainParticipant _this,
    gapi_ReaderInstanceAction action,
    c_voidp arg)
{
    gapi_subscriber s;
    u_subscriber us;
    c_iter readers;
    u_dataReader r;
    _Entity entity;
    u_result uResult;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    s = gapi_domainParticipant_get_builtin_subscriber (_this);
    if (s) {
        entity = gapi_entityClaim(s, NULL);
        if (entity) {
            us =   U_SUBSCRIBER_GET(entity);
            _EntityRelease(entity);
            if (us) {
                readers = u_subscriberLookupReaders(us,"DCPSParticipant");
                if (readers) {
                    r = c_iterTakeFirst(readers);
                    if (r) {
                        uResult = u_dataReaderWalkInstances(r,action,arg);
                        result = kernelResultToApiResult(uResult);
                    } else {
                        result = GAPI_RETCODE_ERROR;
                        OS_REPORT(OS_ERROR,
                                  "gapi_domainParticipant_get_discovered_participants", 0,
                                  "iterTakeFirst for reader returned NULL");
                    }
                } else {
                    result = GAPI_RETCODE_ERROR;
                    OS_REPORT(OS_ERROR,
                              "gapi_domainParticipant_get_discovered_participants", 0,
                              "no reader found for the builtin subscriber");
                  }
            } else {
                result = GAPI_RETCODE_ERROR;
                OS_REPORT(OS_ERROR,
                          "gapi_domainParticipant_get_discovered_participants", 0,
                          "no valid subscriber entity");
            }
        } else {
            result = GAPI_RETCODE_ERROR;
            OS_REPORT(OS_ERROR,
                      "gapi_domainParticipant_get_discovered_participants", 0,
                      "gapi_entityClaim failed on builtin subscriber");
        }
    } else {
        result = GAPI_RETCODE_ERROR;
        OS_REPORT(OS_ERROR,
                  "gapi_domainParticipant_get_discovered_participants", 0,
                  "failed to get builtin subscriber");
    }


    return result;
}


struct copyDiscoveredDataArg
{
    c_voidp to;
    _DataReader reader;
    gapi_readerAction action;
};

static v_actionResult
copyDiscoveredData(
    c_object from,
    c_voidp to)
{
    v_message msg;
    c_voidp sample;
    v_actionResult result = 0;
    struct copyDiscoveredDataArg *arg = (struct copyDiscoveredDataArg *) to;

    if (from) {
        msg = v_message(C_REFGET(v_readerSample(from), arg->reader->messageOffset));
        sample = C_DISPLACE(msg, arg->reader->userdataOffset);
        arg->action(sample, arg->to);
    }
    return result;
}


/*     ReturnCode_t
 *     get_discovered_participant_data (
 *         in InstanceHandle_t handle,
 *         inout ParticipantBuiltinTopicData *participant_data);
 */
gapi_returnCode_t
gapi_domainParticipant_get_discovered_participant_data (
    gapi_domainParticipant _this,
    c_voidp participant_data,
    gapi_instanceHandle_t  handle,
    gapi_readerAction action

    )
{
    gapi_subscriber s;
    gapi_dataReader reader;
    u_dataReader r;
    _DataReader _reader;
    u_result uResult;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    s = gapi_domainParticipant_get_builtin_subscriber (_this);
    if (s) {
        reader = gapi_subscriber_lookup_datareader(s, "DCPSParticipant");
        if (reader) {
            _reader = gapi_dataReaderClaim(reader, NULL);
            if (_reader) {
                struct copyDiscoveredDataArg arg;

                arg.to = participant_data;
                arg.reader = _reader;
                arg.action = action;
                r =  U_DATAREADER_GET(_reader);
                uResult = u_readerReadInstance(u_reader(r),
                        handle,
                        copyDiscoveredData,
                        &arg);
                _EntityRelease(_reader);
                result = kernelResultToApiResult(uResult);
            } else {
                result = GAPI_RETCODE_ERROR;
                OS_REPORT(OS_ERROR,
                          "gapi_domainParticipant_get_discovered_participant_data", 0,
                          "gapi_entityClaim failed on builtin reader");
            }
        } else {
           result = GAPI_RETCODE_ERROR;
           OS_REPORT(OS_ERROR,
                     "gapi_domainParticipant_get_discovered_participant_data", 0,
                     "no reader found for the builtin subscriber");
        }
    } else {
        result = GAPI_RETCODE_ERROR;
        OS_REPORT(OS_ERROR,
                "gapi_domainParticipant_get_discovered_participant_data", 0,
                "no valid subscriber entity");
    }


   return result;
}

/*     ReturnCode_t
 *     get_discovered_topics (
 *         inout InstanceHandleSeq topic_handles);
 */
gapi_returnCode_t
gapi_domainParticipant_get_discovered_topics (
    gapi_domainParticipant _this,
    gapi_ReaderInstanceAction action,
    c_voidp arg)
{
    gapi_subscriber s;
    u_subscriber us;
    c_iter readers;
    u_dataReader r;
    _Entity entity;
    u_result uResult;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    s = gapi_domainParticipant_get_builtin_subscriber (_this);
    if (s) {
       entity = gapi_entityClaim(s, NULL);
       if (entity) {
           us =   U_SUBSCRIBER_GET(entity);
           _EntityRelease(entity);
           if (us) {
               readers = u_subscriberLookupReaders(us,"DCPSTopic");
               if (readers) {
                   r = c_iterTakeFirst(readers);
                   if (r) {
                       uResult = u_dataReaderWalkInstances(r,action,arg);
                       result = kernelResultToApiResult(uResult);
                   } else {
                       result = GAPI_RETCODE_ERROR;
                       OS_REPORT(OS_ERROR,
                                 "gapi_domainParticipant_get_discovered_topics", 0,
                                 "iterTakeFirst for reader returned NULL");
                   }
               } else {
                   result = GAPI_RETCODE_ERROR;
                   OS_REPORT(OS_ERROR,
                             "gapi_domainParticipant_get_discovered_topics", 0,
                             "no reader found for the builtin subscriber");
                 }
           } else {
               result = GAPI_RETCODE_ERROR;
               OS_REPORT(OS_ERROR,
                         "gapi_domainParticipant_get_discovered_topics", 0,
                         "no valid subscriber entity");
           }
       } else {
           result = GAPI_RETCODE_ERROR;
           OS_REPORT(OS_ERROR,
                     "gapi_domainParticipant_get_discovered_topics", 0,
                     "gapi_entityClaim failed on builtin subscriber");
       }
    } else {
       result = GAPI_RETCODE_ERROR;
       OS_REPORT(OS_ERROR,
                 "gapi_domainParticipant_get_discovered_topics", 0,
                 "failed to get builtin subscriber");
    }


    return result;
}
struct copyDiscoveredTopicDataArg
{
    c_voidp to;
    _DataReader reader;
    gapi_readerAction action;
};

static v_actionResult
copyDiscoveredTopicData(
    c_object from,
    c_voidp to)
{
    v_message msg;
    c_voidp sample;
    v_actionResult result = 0;
    struct copyDiscoveredTopicDataArg *arg = (struct copyDiscoveredTopicDataArg *) to;

    if (from) {
        msg = v_message(C_REFGET(v_readerSample(from), arg->reader->messageOffset));
        sample = C_DISPLACE(msg, arg->reader->userdataOffset);
        arg->action(sample, arg->to);
    }
    return result;
}
/*     ReturnCode_t
 *     get_discovered_topic_data (
 *         in InstanceHandle_t handle,
 *         inout TopicBuiltinTopicData *topic_data);
 */
gapi_returnCode_t
gapi_domainParticipant_get_discovered_topic_data (
    gapi_domainParticipant _this,
    c_voidp topic_data,
    gapi_instanceHandle_t  handle,
    gapi_readerAction action)
{
    gapi_subscriber s;
    gapi_dataReader reader;
    u_dataReader r;
    _DataReader _reader;
    u_result uResult;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    s = gapi_domainParticipant_get_builtin_subscriber (_this);
    if (s) {
        reader = gapi_subscriber_lookup_datareader(s, "DCPSTopic");
        if (reader) {
            _reader = gapi_dataReaderClaim(reader, NULL);
            if (_reader) {
                struct copyDiscoveredTopicDataArg arg;

                arg.to = topic_data;
                arg.reader = _reader;
                arg.action = action;
                r =  U_DATAREADER_GET(_reader);
                uResult = u_readerReadInstance(u_reader(r),
                        handle,
                        copyDiscoveredTopicData,
                        &arg);
                _EntityRelease(_reader);
                result = kernelResultToApiResult(uResult);
            } else {
                result = GAPI_RETCODE_ERROR;
                OS_REPORT(OS_ERROR,
                          "gapi_domainParticipant_get_discovered_topic_data", 0,
                          "gapi_entityClaim failed on builtin reader");
            }
        } else {
           result = GAPI_RETCODE_ERROR;
           OS_REPORT(OS_ERROR,
                     "gapi_domainParticipant_get_discovered_topic_data", 0,
                     "no reader found for the builtin subscriber");
        }
    } else {
        result = GAPI_RETCODE_ERROR;
        OS_REPORT(OS_ERROR,
                "gapi_domainParticipant_get_discovered_topic_data", 0,
                "no valid subscriber entity");
    }


   return result;
}

struct check_handle_arg {
    gapi_instanceHandle_t handle;
    gapi_boolean result;
};

static c_bool
publisher_check_handle(
    u_publisher publisher,
    struct check_handle_arg *arg)
{
    gapi_entity handle;
    _Entity e;

    assert(publisher);
    assert(arg);

    if (!arg->result) {
        handle = u_entityGetUserData(u_entity(publisher));
        e = _Entity(gapi_objectPeekUnchecked(handle));
        if (e) {
            arg->result = _EntityHandleEqual(e,arg->handle);
            if ( !arg->result ) {
                arg->result = _PublisherContainsEntity(_Publisher(e), arg->handle);
            }
        }
    }
    return !arg->result;
}

static c_bool
subscriber_check_handle(
    u_subscriber subscriber,
    struct check_handle_arg *arg)
{
    gapi_entity handle;
    _Entity e;

    assert(subscriber);
    assert(arg);

    if (!arg->result) {
        handle = u_entityGetUserData(u_entity(subscriber));
        e = _Entity(gapi_objectPeekUnchecked(handle));
        if (e) {
            arg->result = _EntityHandleEqual(e,arg->handle);
            if ( !arg->result ) {
                arg->result = _SubscriberContainsEntity(_Subscriber(e), arg->handle);
            }
        }
    }
    return !arg->result;
}

static c_bool
topic_check_handle(
    u_topic topic,
    struct check_handle_arg *arg)
{
    gapi_entity handle;
    _Entity e;

    assert(topic);
    assert(arg);

    if (!arg->result) {
        handle = u_entityGetUserData(u_entity(topic));
        e = _Entity(gapi_objectPeekUnchecked(handle));
        if (e) {
            arg->result = _EntityHandleEqual(e,arg->handle);
        }
    }
    return !arg->result;
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
    gapi_boolean contains = FALSE;
    _DomainParticipant participant;
    struct check_handle_arg arg;
    gapi_returnCode_t result;

    if ( a_handle ) {
        participant = gapi_domainParticipantClaim(_this, &result);
        if ( participant ) {
            arg.handle = a_handle;
            arg.result = FALSE;

            if (!arg.result) {
                u_participantWalkPublishers(U_PARTICIPANT_GET(participant),
                                            (u_publisherAction)publisher_check_handle,
                                            (c_voidp)&arg);
            }
            if (!arg.result) {
                u_participantWalkSubscribers(U_PARTICIPANT_GET(participant),
                                             (u_subscriberAction)subscriber_check_handle,
                                             (c_voidp)&arg);
            }
            if ( !arg.result ) {
                u_participantWalkTopics(U_PARTICIPANT_GET(participant),
                                        (u_topicAction)topic_check_handle,
                                        (c_voidp)&arg);
            }
            contains = arg.result;
        } else {
            OS_REPORT_1(OS_WARNING,
                        "gapi_domainParticipant_contains_entity", 0,
                        "Given DomainParticipant is invalid: result = %s",
                        gapi_retcode_image(result));
        }
        _EntityRelease(participant);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_contains_entity", 0,
                    "Given Entity is invalid: result = %s",
                    gapi_retcode_image(result));
    }
    return contains;
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
        _EntityRelease(participant);
    } else {
        OS_REPORT_1(OS_WARNING,
                    "gapi_domainParticipant_get_current_time", 0,
                    "Given DomainParticipant is invalid: result = %s",
                    gapi_retcode_image(result));
    }
    return result;
}



void
_DomainParticipantCleanup (
    _DomainParticipant _this)
{
    u_result uResult;

    assert(_this);

    _EntityClaimNotBusy(_this);
    if(!os_serviceGetSingleProcess())
    {
        _DomainParticipantDeleteContainedEntitiesNoClaim(_this);
    } else
    {
        uResult = u_participantDeleteContainedEntities(U_PARTICIPANT_GET(_this));
        if(uResult != U_RESULT_OK)
        {
            OS_REPORT_1(OS_WARNING,
                        "_DomainParticipantCleanup", 0,
                        "Failed to delete the user layer participant. Result = %s.",
                        gapi_retcode_image(kernelResultToApiResult(uResult)));
        }
    }
    _DomainParticipantFree(_this);

}

