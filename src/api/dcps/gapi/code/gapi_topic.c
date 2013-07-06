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
#include "gapi_topicDescription.h"
#include "gapi_domainParticipant.h"
#include "gapi_structured.h"
#include "gapi_objManag.h"
#include "gapi_kernel.h"
#include "gapi_topic.h"
#include "gapi_qos.h"
#include "gapi_typeSupport.h"
#include "gapi_dataReader.h"
#include "gapi_error.h"

#include "os_stdlib.h"
#include "os_heap.h"
#include "u_user.h"
#include "u_entity.h"
#include "u_topic.h"
#include "v_kernel.h"
#include "sd_serializerXMLMetadata.h"
#include "sd_serializerXMLTypeinfo.h"
#include "sd_typeInfoParser.h"

#define U_TOPIC_SET(t,e)     _EntitySetUserEntity(_Entity(t), u_entity(e))

C_STRUCT(_Topic) {
    C_EXTENDS(_TopicDescription);
    gapi_long                 _refCount;
    struct gapi_topicListener _Listener;
};

static gapi_boolean
copyTopicQosIn (
    const gapi_topicQos *srcQos,
     v_topicQos dstQos);

static gapi_boolean
copyTopicQosOut (
    const v_topicQos srcQos,
     gapi_topicQos *dstQos);

_Topic
_TopicNew (
    const gapi_char *topicName,
    const gapi_char *typeName,
    const _TypeSupport typesupport,
    const gapi_topicQos *qos,
    const struct gapi_topicListener *a_listener,
    const gapi_statusMask mask,
    const _DomainParticipant participant,
    const gapi_context *context)
{
    _Topic newTopic = NULL;
    v_topicQos topicQos;
    u_participant uParticipant;
    u_topic uTopic;

    assert(topicName);
    assert(typeName);
    assert(typesupport);
    assert(qos);
    assert(participant);

    uParticipant = _DomainParticipantUparticipant(participant);

    if ( gapi_topicQosIsConsistent(qos, context) == GAPI_RETCODE_OK ) {
        newTopic = _TopicAlloc();
    }

    if ( newTopic != NULL ) {
        const char      *prefix = "select * from ";
        unsigned long    len;
        char            *expr;

        len = strlen(prefix) + strlen(topicName) + 1;
        expr = (char *) os_malloc(len);

        if ( expr ) {
            snprintf(expr, len, "%s%s", prefix, topicName);
            if ( _TopicDescriptionInit(_TopicDescription(newTopic),
                                       topicName,
                                       typeName,
                                       expr,
                                       participant) != GAPI_RETCODE_OK ) {
                _EntityDelete(newTopic);
                newTopic = NULL;
            }
            os_free(expr);
        } else {
            _EntityDelete(newTopic);
            newTopic = NULL;
        }
    }

    if ( newTopic != NULL ) {
        if ( a_listener ) {
            newTopic->_Listener = *a_listener;
        }
        newTopic->_refCount = 1;
        topicQos = u_topicQosNew(NULL);
        if ( topicQos != NULL ) {
            copyTopicQosIn(qos, topicQos);
        } else {
            _TopicDescriptionDispose(_TopicDescription(newTopic));
            newTopic = NULL;
        }
    }

    if ( newTopic != NULL ) {
        uTopic = u_topicNew(uParticipant,
                            topicName,
                            _TypeSupportTypeName(typesupport),
                            _TypeSupportTypeKeys(typesupport),
                            topicQos);
        u_topicQosFree(topicQos);
        if ( uTopic != NULL ) {
            U_TOPIC_SET(newTopic, uTopic);
        } else {
            gapi_redefineError(context);
            _TopicDescriptionDispose(_TopicDescription(newTopic));
            newTopic = NULL;
        }
    }
    if ( newTopic != NULL ) {
        _EntityStatus(newTopic) = _StatusNew(_Entity(newTopic),
                                             STATUS_KIND_TOPIC,
                                             (const struct gapi_listener *) a_listener,
                                             mask);
        if (_EntityStatus(newTopic) == NULL ) {
            u_topicFree(U_TOPIC_GET(newTopic));
            _TopicDescriptionDispose(_TopicDescription(newTopic));
            newTopic = NULL;
        }
    }
    return newTopic;
}

_Topic
_TopicFromKernelTopic (
    u_topic uTopic,
    const gapi_char *topicName,
    const gapi_char *typeName,
    const _DomainParticipant participant,
    const gapi_context *context)
{
    _Topic newTopic = NULL;
    u_participant uParticipant;

    assert(uTopic);
    assert(topicName);
    assert(typeName);
    assert(participant);

    uParticipant = _DomainParticipantUparticipant(participant);

    newTopic = _TopicAlloc();

    if ( newTopic != NULL ) {
        const char      *prefix = "select * from ";
        unsigned long    len;
        char            *stmt;

        len = strlen(prefix) + strlen(topicName) + 1;
        stmt = (char *) os_malloc(len);

        if ( stmt ) {
            snprintf(stmt, len, "%s%s", prefix, topicName);
            if ( _TopicDescriptionInit(_TopicDescription(newTopic),
                                       topicName,
                                       typeName,
                                       stmt,
                                       participant) == GAPI_RETCODE_OK ) {
                U_TOPIC_SET(newTopic, uTopic);
                newTopic->_refCount = 1;
            } else {
                _EntityDelete(newTopic);
                newTopic = NULL;
            }
            os_free(stmt);
        } else {
            _EntityDelete(newTopic);
            newTopic = NULL;
        }
    }
    if ( newTopic ) {
        _EntityStatus(newTopic) = _StatusNew(_Entity(newTopic),
                                             STATUS_KIND_TOPIC,
                                             NULL,0);
        if (_EntityStatus(newTopic) == NULL ) {
            _TopicDescriptionDispose(_TopicDescription(newTopic));
            newTopic = NULL;
        }
    }
    return newTopic;
}

_Topic
_TopicFromTopic (
    _Topic topic,
    const _DomainParticipant participant,
    const gapi_context *context)
{
    _Topic newTopic = NULL;
    _TopicDescription descr;
    u_participant uParticipant;
    u_topic uTopic;
    _TypeSupport typeSupport;
    v_topicQos topicQos;
    gapi_char *topicName;
    gapi_char *typeName;

    assert(topic);
    assert(participant);

    uParticipant = _DomainParticipantUparticipant(participant);
    descr = _TopicDescription(topic);

    topicName = descr->topic_name;
    typeName  = descr->type_name;

    typeSupport = _DomainParticipantFindType(participant, typeName);

    if ( typeSupport ) {
        newTopic = _TopicAlloc();
    }

    if ( newTopic != NULL ) {
        const char      *prefix = "select * from ";
        unsigned long    len;
        char            *stmt;

        len = strlen(prefix) + strlen(topicName) + 1;
        stmt = (char *) os_malloc(len);

        if ( stmt ) {
            snprintf(stmt, len, "%s%s", prefix, topicName);
            if ( _TopicDescriptionInit(_TopicDescription(newTopic),
                                       topicName,
                                       typeName,
                                       stmt,
                                       participant) == GAPI_RETCODE_OK )
            {
                newTopic->_refCount = 1;
            } else {
                _EntityDelete(newTopic);
                newTopic = NULL;
            }
            os_free(stmt);
        } else {
            _EntityDelete(newTopic);
            newTopic = NULL;
        }
    }

    if ( newTopic != NULL ) {
        /* obtain the v_qos of the existing topic */

        if ( u_entityQoS(u_entity(U_TOPIC_GET(topic)),
                         (v_qos*)&topicQos) != U_RESULT_OK )
        {
            _TopicDescriptionDispose(_TopicDescription(newTopic));
            newTopic = NULL;
        }
    }
    if ( newTopic != NULL ) {
        uTopic = u_topicNew(uParticipant,
                            topicName,
                            _TypeSupportTypeName(typeSupport),
                            _TypeSupportTypeKeys(typeSupport),
                            topicQos);
        u_topicQosFree(topicQos);
        if ( uTopic != NULL ) {
            U_TOPIC_SET(newTopic, uTopic);
        } else {
            gapi_redefineError(context);
            _TopicDescriptionDispose(_TopicDescription(newTopic));
            newTopic = NULL;
        }
    }
    if ( newTopic ) {
        _EntityStatus(newTopic) = _StatusNew(_Entity(newTopic),
                                             STATUS_KIND_TOPIC,
                                             NULL,0);
        if ( _EntityStatus(newTopic) == NULL ) {
            u_topicFree(U_TOPIC_GET(newTopic));
            _TopicDescriptionDispose(_TopicDescription(newTopic));
            newTopic = NULL;
        }
    }
    return newTopic;
}


_Topic
_TopicFromUserTopic (
    u_topic uTopic,
    const _DomainParticipant participant,
    const gapi_context *context)
{
    _Topic newTopic = NULL;
    u_participant uParticipant;
    _TypeSupport typeSupport;
    gapi_char *topicName;
    gapi_char *typeName;

    assert(uTopic);
    assert(participant);

    uParticipant = _DomainParticipantUparticipant(participant);

    topicName = u_topicName(uTopic);
    typeName  = u_topicTypeName(uTopic);

    typeSupport = _DomainParticipantFindType(participant, typeName);

    if ( typeSupport ) {
        newTopic = _TopicAlloc();
    }

    if ( newTopic != NULL ) {
        const char      *prefix = "select * from ";
        unsigned long    len;
        char            *stmt;

        len = strlen(prefix) + strlen(topicName) + 1;
        stmt = (char *) os_malloc(len);

        if ( stmt ) {
            snprintf(stmt, len, "%s%s", prefix, topicName);
            if ( _TopicDescriptionInit(_TopicDescription(newTopic),
                                       topicName,
                                       typeName,
                                       stmt,
                                       participant) == GAPI_RETCODE_OK )
            {
                newTopic->_refCount = 1;
            } else {
                _EntityDelete(newTopic);
                newTopic = NULL;
            }
            os_free(stmt);
        } else {
            _EntityDelete(newTopic);
            newTopic = NULL;
        }
    }

    if ( newTopic != NULL ) {
        U_TOPIC_SET(newTopic, uTopic);
#if 1 /* existing user layer topic so we should not try to set user data as
       * it should heve been set already or is going to be set soon.
       */
        if (u_entityGetUserData(u_entity(uTopic)) == NULL) {
            c_voidp prev;
            prev = u_entitySetUserData(u_entity(uTopic),newTopic);
            assert(prev == NULL);
        } else {
            OS_REPORT_1(OS_ERROR,
                        "_TopicFromUserTopic", 0,
                        "Set user layer topic handle rejected, handle 0x%x already assigned.",
                        u_entityGetUserData(u_entity(uTopic)));
        }
#endif
    }
    if ( newTopic ) {
        _EntityStatus(newTopic) = _StatusNew(_Entity(newTopic),
                                             STATUS_KIND_TOPIC,
                                             NULL,0);
        if ( _EntityStatus(newTopic) == NULL ) {
            _TopicDescriptionDispose(_TopicDescription(newTopic));
            newTopic = NULL;
        }
    }
    return newTopic;
}

gapi_returnCode_t
_TopicFree (
    _Topic topic)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Status status;
    u_topic t;
    c_long count;

    assert (topic);

    count = _TopicDecRef(topic);
    if (count == 0) {
        status = _EntityStatus(topic);
        _StatusSetListener(status,NULL,0);
        _EntityClaim(status);
        _StatusDeinit(status);
        t = U_TOPIC_GET(topic);
        _TopicDescriptionDispose (_TopicDescription(topic));
        u_entitySetUserData(u_entity(t),NULL);
        u_topicFree(t);
    } else if (count < 0) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

gapi_long
_TopicIncRef (
    _Topic topic)
{
    assert(topic);
    assert(topic->_refCount > 0);
    topic->_refCount++;
    return topic->_refCount;
}

gapi_long
_TopicDecRef (
    _Topic topic)
{
    assert(topic);
    assert(topic->_refCount > 0);
    topic->_refCount--;
    return topic->_refCount;
}

gapi_long
_TopicRefCount (
    _Topic topic)
{
    assert(topic);
    if (topic) {
        return topic->_refCount;
    } else {
        return 0;
    }
}

gapi_topicQos *
_TopicGetQos (
    _Topic topic,
    gapi_topicQos *qos)
{
    v_topicQos topicQos;
    u_topic uTopic;
    u_result result;

    assert(topic);

    uTopic = U_TOPIC_GET(topic);
    result = u_entityQoS(u_entity(uTopic),(v_qos*)&topicQos);
    if (result == U_RESULT_OK) {
        copyTopicQosOut(topicQos,  qos);
        u_topicQosFree(topicQos);
    } else {
        OS_REPORT_1(OS_ERROR,
                    "Failure during topic QoS get", 0,
                    "u_entityQoS return with result %d", result);
    }
    return qos;
}

u_topic
_TopicUtopic (
    _Topic topic)
{
    assert(topic);
    assert(topic->_refCount > 0);

    return U_TOPIC_GET(topic);
}

gapi_returnCode_t
gapi_topic_set_qos (
    gapi_topic _this,
    const gapi_topicQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_result uResult;
    _Topic topic;
    v_topicQos topicQos;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_QOS);

    topic = gapi_topicClaim(_this, &result);

    if ( topic && qos ) {
        result = gapi_topicQosIsConsistent(qos, &context);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    if ((result == GAPI_RETCODE_OK ) && (_EntityEnabled(topic))) {
        gapi_topicQos *existing_qos = gapi_topicQos__alloc();

        result = gapi_topicQosCheckMutability(qos,
                                              _TopicGetQos(topic,
                                                           existing_qos),
                                              &context);
        gapi_free(existing_qos);
    }

    if ( result == GAPI_RETCODE_OK ) {
        topicQos = u_topicQosNew(NULL);
        if (topicQos) {
            if ( copyTopicQosIn(qos, topicQos) ) {
                uResult = u_entitySetQoS(_EntityUEntity(topic),
                                         (v_qos)(topicQos));
                result = kernelResultToApiResult(uResult);
                u_topicQosFree(topicQos);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    }

    _EntityRelease(topic);

    return result;
}

gapi_returnCode_t
gapi_topic_get_qos (
    gapi_topic _this,
    gapi_topicQos *qos)
{
    _Topic topic;
    gapi_returnCode_t result;

    topic = gapi_topicClaim(_this, &result);
    if ( topic && qos ) {
        _TopicGetQos(topic,qos);
    }
    _EntityRelease(topic);
    return result;
}

gapi_returnCode_t
gapi_topic_dispose_all_data (
    gapi_topic _this)
{
    gapi_returnCode_t result;
    _Topic topic;
    u_topic uTopic;

    topic = gapi_topicClaim(_this, &result);
    if ( topic )
    {
        u_result uResult;

        uTopic = U_TOPIC_GET(topic);
        uResult = u_topicDisposeAllData( uTopic );
        result = kernelResultToApiResult(uResult);
    }

   _EntityRelease(topic);
   return result;
}

static v_result
copyInconsistentTopicStatus (
    c_voidp info,
    c_voidp arg)
{
    struct v_inconsistentTopicInfo *from;
    gapi_inconsistentTopicStatus *to;

    from = (struct v_inconsistentTopicInfo *)info;
    to = (gapi_inconsistentTopicStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    return V_RESULT_OK;
}

static v_result
copyAllDataDisposedTopicStatus (
    c_voidp info,
    c_voidp arg)
{
    struct v_allDataDisposedInfo *from;
    gapi_allDataDisposedTopicStatus *to;

    from = (struct v_allDataDisposedInfo *)info;
    to = (gapi_allDataDisposedTopicStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    return V_RESULT_OK;
}

static gapi_returnCode_t
_TopicGetInconsistentTopicStatus (
    _Topic _this,
    gapi_inconsistentTopicStatus *status)
{
    u_result uResult;
    uResult = u_topicGetInconsistentTopicStatus(
                  U_TOPIC_GET(_this),
                  TRUE,
                  copyInconsistentTopicStatus,
                  status);
    return kernelResultToApiResult(uResult);
}

static gapi_returnCode_t
_TopicPeekInconsistentTopicStatus (
    _Topic _this,
    gapi_inconsistentTopicStatus *status)
{
    u_result uResult;
    uResult = u_topicGetInconsistentTopicStatus(
                  U_TOPIC_GET(_this),
                  FALSE,
                  copyInconsistentTopicStatus,
                  status);
    return kernelResultToApiResult(uResult);
}

static gapi_returnCode_t
_TopicGetAllDataDisposedTopicStatus (
    _Topic _this,
    gapi_allDataDisposedTopicStatus *status)
{
    u_result uResult;
    uResult = u_topicGetAllDataDisposedStatus(
                  U_TOPIC_GET(_this),
                  TRUE,
                  copyAllDataDisposedTopicStatus,
                  status);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_topic_get_inconsistent_topic_status (
    gapi_topic _this,
    gapi_inconsistentTopicStatus *status)
{
    _Topic topic;
    gapi_returnCode_t result;

    topic = gapi_topicClaim(_this, &result);

    if (topic != NULL) {
        if (_EntityEnabled(topic) ) {
            result = _TopicGetInconsistentTopicStatus (
                         topic, status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(topic);

    return result;
}

gapi_returnCode_t
gapi_topic_get_all_data_disposed_topic_status (
    gapi_topic _this,
    gapi_allDataDisposedTopicStatus *status)
{
    _Topic topic;
    gapi_returnCode_t result;

    topic = gapi_topicClaim(_this, &result);

    if (topic != NULL) {
        if (_EntityEnabled(topic) ) {
            result = _TopicGetAllDataDisposedTopicStatus (
                         topic, status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(topic);

    return result;
}

gapi_returnCode_t
gapi_topic_set_listener (
    gapi_topic _this,
    const struct gapi_topicListener *a_listener,
    const gapi_statusMask mask)
{
    gapi_returnCode_t result = GAPI_RETCODE_ERROR;
    _Topic topic;

    topic = gapi_topicClaim(_this, &result);

    if ( topic != NULL ) {
        if ( _EntityEnabled(topic) ) {
            _Status status;

            if ( a_listener ) {
                topic->_Listener = *a_listener;
            } else {
                memset(&topic->_Listener, 0, sizeof(topic->_Listener));
            }

            status = _EntityStatus(topic);
            if ( _StatusSetListener(status,
                                    (struct gapi_listener *)a_listener,
                                    mask) )
            {
                result = GAPI_RETCODE_OK;
            }
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(topic);

    return result;
}

struct gapi_topicListener
gapi_topic_get_listener (
    gapi_topic _this)
{
    _Topic topic;
    struct gapi_topicListener listener;

   topic = gapi_topicClaim(_this, NULL);

    if ( topic != NULL ) {
        listener = topic->_Listener;
    } else {
        memset(&listener, 0, sizeof(listener));
    }

    _EntityRelease(topic);

    return listener;
}

static gapi_boolean
copyTopicQosIn (
    const gapi_topicQos *srcQos,
    v_topicQos dstQos)
{
    gapi_boolean copied = FALSE;

    assert(srcQos);
    assert(dstQos);

    if (srcQos->topic_data.value._length) {
        dstQos->topicData.size = srcQos->topic_data.value._length;
        dstQos->topicData.value = os_malloc (srcQos->topic_data.value._length);
    }
    if ((srcQos->topic_data.value._length == 0) || dstQos->topicData.value) {

        kernelCopyInDuration(&srcQos->deadline.period,
                             &dstQos->deadline.period);

        dstQos->durability.kind =
                srcQos->durability.kind;
        dstQos->durabilityService.history_kind =
                srcQos->durability_service.history_kind;
        dstQos->durabilityService.history_depth =
                srcQos->durability_service.history_depth;
        dstQos->durabilityService.max_samples =
                srcQos->durability_service.max_samples;
        dstQos->durabilityService.max_instances =
                srcQos->durability_service.max_instances;
        dstQos->durabilityService.max_samples_per_instance =
                srcQos->durability_service.max_samples_per_instance;

        kernelCopyInDuration(&srcQos->durability_service.service_cleanup_delay,
                             &dstQos->durabilityService.service_cleanup_delay);

        dstQos->history.kind = srcQos->history.kind;
        dstQos->history.depth = srcQos->history.depth;

        kernelCopyInDuration(&srcQos->latency_budget.duration,
                             &dstQos->latency.duration);

        kernelCopyInDuration(&srcQos->lifespan.duration,
                             &dstQos->lifespan.duration);

        dstQos->liveliness.kind =
                srcQos->liveliness.kind;

        kernelCopyInDuration(&srcQos->liveliness.lease_duration,
                             &dstQos->liveliness.lease_duration);

        dstQos->orderby.kind =
                srcQos->destination_order.kind;
        dstQos->ownership.kind =
                srcQos->ownership.kind;
        dstQos->reliability.kind =
                srcQos->reliability.kind;

        kernelCopyInDuration(&srcQos->reliability.max_blocking_time,
                             &dstQos->reliability.max_blocking_time);

        dstQos->reliability.synchronous =
                srcQos->reliability.synchronous;

        dstQos->resource.max_samples =
                srcQos->resource_limits.max_samples;
        dstQos->resource.max_instances =
                srcQos->resource_limits.max_instances;
        dstQos->resource.max_samples_per_instance =
                srcQos->resource_limits.max_samples_per_instance;
        dstQos->transport.value =
                srcQos->transport_priority.value;

        if (srcQos->topic_data.value._length) {
            memcpy (dstQos->topicData.value,
                    srcQos->topic_data.value._buffer,
                    srcQos->topic_data.value._length);
        }
        copied = TRUE;
    }
    return copied;
}

static gapi_boolean
copyTopicQosOut (
    const v_topicQos srcQos,
    gapi_topicQos   *dstQos)
{
    assert(srcQos);
    assert(dstQos);

    kernelCopyOutDuration(&srcQos->deadline.period,
                          &dstQos->deadline.period);

    dstQos->durability.kind =
            srcQos->durability.kind;

    dstQos->durability_service.history_kind =
            srcQos->durabilityService.history_kind;
    dstQos->durability_service.history_depth =
            srcQos->durabilityService.history_depth;
    dstQos->durability_service.max_samples =
            srcQos->durabilityService.max_samples;
    dstQos->durability_service.max_instances =
            srcQos->durabilityService.max_instances;
    dstQos->durability_service.max_samples_per_instance =
            srcQos->durabilityService.max_samples_per_instance;

    kernelCopyOutDuration(&srcQos->durabilityService.service_cleanup_delay,
                          &dstQos->durability_service.service_cleanup_delay);

    dstQos->history.kind =
            srcQos->history.kind;
    dstQos->history.depth =
            srcQos->history.depth;

    kernelCopyOutDuration(&srcQos->latency.duration,
                          &dstQos->latency_budget.duration);

    kernelCopyOutDuration(&srcQos->lifespan.duration,
                          &dstQos->lifespan.duration);

    dstQos->liveliness.kind =
            srcQos->liveliness.kind;

    kernelCopyOutDuration(&srcQos->liveliness.lease_duration,
                          &dstQos->liveliness.lease_duration);

    dstQos->destination_order.kind =
            srcQos->orderby.kind;
    dstQos->ownership.kind =
            srcQos->ownership.kind;
    dstQos->reliability.kind =
            srcQos->reliability.kind;

    kernelCopyOutDuration(&srcQos->reliability.max_blocking_time,
                          &dstQos->reliability.max_blocking_time);

    dstQos->reliability.synchronous =
            srcQos->reliability.synchronous;

    dstQos->resource_limits.max_samples =
            srcQos->resource.max_samples;
    dstQos->resource_limits.max_instances =
            srcQos->resource.max_instances;
    dstQos->resource_limits.max_samples_per_instance =
            srcQos->resource.max_samples_per_instance;
    dstQos->transport_priority.value =
            srcQos->transport.value;

    if ( dstQos->topic_data.value._maximum > 0 ) {
        if ( dstQos->topic_data.value._release ) {
            gapi_free(dstQos->topic_data.value._buffer);
        }
    }
    if ( (srcQos->topicData.size > 0) && srcQos->topicData.value ) {
        dstQos->topic_data.value._buffer = gapi_octetSeq_allocbuf(srcQos->topicData.size);
        if ( dstQos->topic_data.value._buffer ) {
            dstQos->topic_data.value._maximum = srcQos->topicData.size;
            dstQos->topic_data.value._length  = srcQos->topicData.size;
            dstQos->topic_data.value._release = TRUE;
            memcpy(dstQos->topic_data.value._buffer,
                   srcQos->topicData.value,
                   srcQos->topicData.size);
        }
    } else {
        dstQos->topic_data.value._maximum = 0;
        dstQos->topic_data.value._length  = 0;
        dstQos->topic_data.value._release = FALSE;
        dstQos->topic_data.value._buffer = NULL;
    }

    return TRUE;
}

void
_TopicNotifyListener(
    _Topic _this,
    gapi_statusMask triggerMask)
{
    gapi_object source = _EntityHandle(_this);
    _Status status = _EntityStatus(_this);
    gapi_returnCode_t result;
    if ( triggerMask & GAPI_INCONSISTENT_TOPIC_STATUS ) {
        gapi_inconsistentTopicStatus info;

        result = _TopicPeekInconsistentTopicStatus (_this, &info);
        if (result == GAPI_RETCODE_OK) {
            _StatusNotifyInconsistentTopic(status, source, &info);
        }
    }

    if ( triggerMask & GAPI_ALL_DATA_DISPOSED_STATUS ) {
        gapi_allDataDisposedTopicStatus info;

        result = _TopicGetAllDataDisposedTopicStatus(_this, &info);
        if (result == GAPI_RETCODE_OK) {
            _StatusNotifyAllDataDisposed(status, source);
        }
    }
}

gapi_string
gapi_topic_get_metadescription (
    gapi_topic _this)
{
    gapi_returnCode_t result;
    _Topic topic;
    u_topic uTopic;

    gapi_string metadescription = NULL;
    gapi_string description = NULL;
    sd_serializer serializer;
    sd_serializedData serData;
    c_base base;
    c_type type = NULL;

    topic = gapi_topicClaim(_this, &result);
    if ( topic && result == GAPI_RETCODE_OK )
    {
        uTopic = U_TOPIC_GET(topic);
        type = u_topicGetUserType( uTopic );
        if ( type ) {
            base = c_getBase(type);
            if (base) {
                serializer = sd_serializerXMLTypeinfoNew(base, FALSE);
                if ( serializer ) {
                    serData = sd_serializerSerialize(serializer, (c_object)type);
                    if ( serData ) {
                        description = sd_serializerToString(serializer, serData);
                        if ( description ) {
                            metadescription = gapi_string_dup(description);
                            os_free(description);
                        }
                        sd_serializedDataFree(serData);
                    }
                    sd_serializerFree(serializer);
                }
            }
            c_free(type);
        }
    }
   _EntityRelease(topic);
   return metadescription;
}

gapi_string
gapi_topic_get_keylist (
    gapi_topic _this)
{
    gapi_returnCode_t result;
    _Topic topic;
    u_topic uTopic;

    gapi_string keylist = NULL;
    gapi_string keys = NULL;

    topic = gapi_topicClaim(_this, &result);
    if ( topic && result == GAPI_RETCODE_OK )
    {
        uTopic = U_TOPIC_GET(topic);
        keys = u_topicGetTopicKeys( uTopic );
        if (keys) {
            keylist = gapi_string_dup(keys);
            c_free(keys);
        }
    }
   _EntityRelease(topic);
   return keylist;
}

