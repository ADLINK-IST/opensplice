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
#include "gapi_subscriber.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_domainParticipant.h"
#include "gapi_structured.h"
#include "gapi_typeSupport.h"
#include "gapi_topic.h"
#include "gapi_entity.h"
#include "gapi_topicDescription.h"
#include "gapi_contentFilteredTopic.h"
#include "gapi_dataReader.h"
#include "gapi_builtin.h"
#include "gapi_qos.h"
#include "gapi_objManag.h"
#include "gapi_kernel.h"
#include "gapi_error.h"

#include "os_heap.h"
#include "u_user.h"

static gapi_boolean
copySubscriberQosIn (
    const gapi_subscriberQos *srcQos,
    v_subscriberQos dstQos)
{
    gapi_boolean copied = TRUE;

    assert(srcQos);
    assert(dstQos);

    /* Important note: The sequence related to policies is created on heap and
     * will be copied into the database by the kernel itself, therefore the
     * c_array does not need to be allocated with c_arrayNew, but can be
     * allocated on heap.
     */
    if (dstQos->groupData.value) {
        os_free(dstQos->groupData.value);
        dstQos->groupData.value = NULL;
    }
    dstQos->groupData.size = srcQos->group_data.value._length;
    if (srcQos->group_data.value._length) {
        dstQos->groupData.value = os_malloc (srcQos->group_data.value._length);
        if (dstQos->groupData.value != NULL) {
            memcpy (dstQos->groupData.value,
                    srcQos->group_data.value._buffer,
                    srcQos->group_data.value._length);
        } else {
            copied = FALSE;
        }
    }

    if (copied) {
        dstQos->partition = gapi_stringSeq_to_String(&srcQos->partition.name,",");
        if ( srcQos->partition.name._length > 0 &&  !dstQos->partition) {
            copied = FALSE;
        }
    }

    if (copied) {
        dstQos->presentation.access_scope    =
                srcQos->presentation.access_scope;
        dstQos->presentation.coherent_access =
                srcQos->presentation.coherent_access;
        dstQos->presentation.ordered_access  =
                srcQos->presentation.ordered_access;
        dstQos->entityFactory.autoenable_created_entities =
                srcQos->entity_factory.autoenable_created_entities;

        if ( srcQos->share.enable ) {
            dstQos->share.enable = TRUE;
            dstQos->share.name = gapi_strdup(srcQos->share.name);
        } else {
            dstQos->share.enable = FALSE;
            dstQos->share.name   = NULL;
        }
    }

    return copied;
}

static gapi_boolean
copySubscriberQosOut (
    const v_subscriberQos  srcQos,
    gapi_subscriberQos *dstQos)
{
    assert(srcQos);
    assert(dstQos);

    if ( dstQos->group_data.value._maximum > 0 ) {
        if ( dstQos->group_data.value._release ) {
            gapi_free(dstQos->group_data.value._buffer);
        }
    }

    if ( (srcQos->groupData.size > 0) && srcQos->groupData.value ) {
        dstQos->group_data.value._buffer = gapi_octetSeq_allocbuf(srcQos->groupData.size);
        if ( dstQos->group_data.value._buffer ) {
            dstQos->group_data.value._maximum = srcQos->groupData.size;
            dstQos->group_data.value._length  = srcQos->groupData.size;
            dstQos->group_data.value._release = TRUE;
            memcpy(dstQos->group_data.value._buffer,
                   srcQos->groupData.value,
                   srcQos->groupData.size);
        }
    } else {
            dstQos->group_data.value._maximum = 0;
            dstQos->group_data.value._length  = 0;
            dstQos->group_data.value._release = FALSE;
            dstQos->group_data.value._buffer = NULL;
    }


    gapi_string_to_StringSeq(srcQos->partition,",",&dstQos->partition.name);

    dstQos->presentation.access_scope    =
            srcQos->presentation.access_scope;
    dstQos->presentation.coherent_access =
            srcQos->presentation.coherent_access;
    dstQos->presentation.ordered_access  =
            srcQos->presentation.ordered_access;
    dstQos->entity_factory.autoenable_created_entities =
            srcQos->entityFactory.autoenable_created_entities;

    if ( srcQos->share.enable ) {
        dstQos->share.enable = TRUE;
        dstQos->share.name = gapi_strdup(srcQos->share.name);
    } else {
        dstQos->share.enable = FALSE;
        dstQos->share.name   = NULL;
    }

    return TRUE;
}

_Subscriber
_SubscriberNew (
    u_participant uParticipant,
    const gapi_subscriberQos  *qos,
    const struct gapi_subscriberListener *a_listener,
    const gapi_statusMask mask,
    const _DomainParticipant participant)
{
    _Subscriber newSubscriber;
    v_subscriberQos subscriberQos;
    gapi_long len;

    assert(uParticipant);
    assert(qos);
    assert(participant);

    newSubscriber = _SubscriberAlloc();

    if ( newSubscriber != NULL ) {
        _EntityInit(_Entity(newSubscriber),
                          _Entity(participant));
        gapi_dataReaderQosCopy (&gapi_dataReaderQosDefault,
                                &newSubscriber->_defDataReaderQos);
        if ( a_listener ) {
            newSubscriber->_Listener = *a_listener;
        }
    }

    if  (newSubscriber != NULL ) {
        subscriberQos = u_subscriberQosNew(NULL);
        if ( subscriberQos != NULL ) {
            if ( !copySubscriberQosIn(qos, subscriberQos) ) {
                _EntityDispose(_Entity(newSubscriber));
                newSubscriber = NULL;
            }
        } else {
            _EntityDispose(_Entity(newSubscriber));
            newSubscriber = NULL;
        }
    }

    if ( newSubscriber != NULL) {
        u_subscriber uSubscriber;
        uSubscriber = u_subscriberNew(uParticipant, "subscriber", subscriberQos, FALSE);
        u_subscriberQosFree(subscriberQos);
        if ( uSubscriber != NULL ) {
            U_SUBSCRIBER_SET(newSubscriber, uSubscriber);
        } else {
            _EntityDispose(_Entity(newSubscriber));
            newSubscriber = NULL;
        }
    }

    if ( newSubscriber != NULL) {
        _Status status;

        status = _StatusNew(_Entity(newSubscriber),
                            STATUS_KIND_SUBSCRIBER,
                            (struct gapi_listener *)a_listener, mask);
        if (status) {
            _EntityStatus(newSubscriber) = status;
            len = (gapi_long)qos->partition.name._length;
            if ( qos->partition.name._length == 0UL ) {
                /*
                 * behaviour of the kernel in case of an empty sequence
                 * is that it is related to none of the partitions,
                 * while DCPS expects it to be conected to all partitions.
                 * Therefore this has to be done seperately.
                 */
                u_subscriberSubscribe (U_SUBSCRIBER_GET(newSubscriber), "");
            }
            newSubscriber->builtin = FALSE;
        } else {
            u_subscriberFree(U_SUBSCRIBER_GET(newSubscriber));
            _EntityDispose(_Entity(newSubscriber));
            newSubscriber = NULL;
        }
    }

    return newSubscriber;
}

gapi_returnCode_t
_SubscriberFree (
    _Subscriber subscriber)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Status status;
    u_subscriber s;

    assert(subscriber);

    status = _EntityStatus(subscriber);
    _StatusSetListener(status, NULL, 0);

    _EntityClaim(status);
    _StatusDeinit(status);

    gapi_dataReaderQos_free(&subscriber->_defDataReaderQos);

    s = U_SUBSCRIBER_GET(subscriber);
    _EntityDispose(_Entity(subscriber));
    if (u_subscriberFree(s) != U_RESULT_OK) {
        result = GAPI_RETCODE_ERROR;
    }

    return result;
}

c_long
_SubscriberReaderCount (
    _Subscriber _this)
{
    return u_subscriberReaderCount(U_SUBSCRIBER_GET(_this));
}

u_subscriber
_SubscriberUsubscriber (
    _Subscriber subscriber)
{
    assert(subscriber);
    return U_SUBSCRIBER_GET(subscriber);
}

gapi_subscriberQos *
_SubscriberGetQos (
    _Subscriber subscriber,
    gapi_subscriberQos *qos)
{
    v_subscriberQos subscriberQos;
    u_subscriber uSubscriber;

    assert(subscriber);

    uSubscriber = U_SUBSCRIBER_GET(subscriber);

    if ( u_entityQoS(u_entity(uSubscriber), (v_qos*)&subscriberQos) == U_RESULT_OK ) {
        copySubscriberQosOut(subscriberQos,  qos);
        u_subscriberQosFree(subscriberQos);
    }

    return qos;
}

gapi_dataReader
gapi_subscriber_create_datareader (
    gapi_subscriber _this,
    const gapi_topicDescription a_topic,
    const gapi_dataReaderQos *qos,
    const struct gapi_dataReaderListener *a_listener,
    const gapi_statusMask mask)
{
    _Subscriber         subscriber;
    _DataReader         datareader       = NULL;
    gapi_dataReader     result           = NULL;
    _TopicDescription   topicDescription = NULL;
    gapi_dataReaderQos *readerQos;
    gapi_context        context;
    gapi_topicQos* topicQos;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_DATAREADER);

    subscriber = gapi_subscriberClaim(_this, NULL);

    if ( subscriber ) {
        if ( !subscriber->builtin ) {
            topicDescription = _TopicDescriptionFromHandle(a_topic);
        }
    }

    if ( topicDescription ) {
        if ( qos == GAPI_DATAREADER_QOS_DEFAULT ) {
            readerQos = &subscriber->_defDataReaderQos;
        } else if ( qos == GAPI_DATAREADER_QOS_USE_TOPIC_QOS ) {
            _Topic topic = NULL;
            switch(_ObjectGetKind(_Object(topicDescription))) {
            case OBJECT_KIND_TOPIC:
                topic = _Topic(topicDescription);
            break;
            case OBJECT_KIND_CONTENTFILTEREDTOPIC:
                topic = _ContentFilteredTopicGetRelatedTopic(_ContentFilteredTopic(topicDescription));
            break;
            default:
                topic = NULL;
            break;
            }
            if ( topic ) {
                topicQos = gapi_topicQos__alloc();
                readerQos = gapi_dataReaderQos__alloc();
                gapi_dataReaderQosCopy(&subscriber->_defDataReaderQos, readerQos);
                _TopicGetQos(topic, topicQos);
                gapi_mergeTopicQosWithDataReaderQos(topicQos,readerQos);
                gapi_free(topicQos);
            } else {
                readerQos = (gapi_dataReaderQos *)qos;
            }
        } else {
            readerQos = (gapi_dataReaderQos *)qos;
        }

        if (  gapi_dataReaderQosIsConsistent(readerQos, &context) == GAPI_RETCODE_OK ) {
            gapi_char *typeName;
            gapi_char *topicName;
            _DomainParticipant participant;
            _TypeSupport typeSupport;

            /* find topic with the participant for consistency */
            typeName  = _TopicDescriptionGetTypeName(topicDescription);
            topicName = _TopicDescriptionGetName(topicDescription);
            participant = _EntityParticipant(_Entity(subscriber));


            /* find type support for the data type to find
             * data reader create function.
             */
            typeSupport = _DomainParticipantFindType(participant, typeName);
            if(typeSupport)
            {          
                /* if create function is NULL, take default from data reader */
                datareader = _DataReaderNew(topicDescription,
                                            typeSupport,
                                            readerQos,
                                            a_listener,
                                            mask,
                                            subscriber);
                if ( datareader ) {
                    _ENTITY_REGISTER_OBJECT(_Entity(subscriber),
                                            (_Object)datareader);
                }
            }else{
                OS_REPORT_1(OS_WARNING,
                            "gapi_subscriber_create_datareader", 0,
                            "TypeSupport %s not found !",
                            typeName);
            }
            
            gapi_free(typeName);
            gapi_free(topicName);
        }

        if (qos == GAPI_DATAREADER_QOS_USE_TOPIC_QOS) {
            gapi_free(readerQos);
        }
    }

    _EntityRelease(subscriber);

    if ( datareader ) {
        gapi_object statusHandle;
        statusHandle = _EntityHandle(_Entity(datareader)->status);
        result = (gapi_dataReader)_EntityRelease(datareader);
    }

    return result;
}

gapi_returnCode_t
gapi_subscriber_delete_datareader (
    gapi_subscriber _this,
    const gapi_dataReader a_datareader)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Subscriber subscriber;
    _DataReader datareader = NULL;
    gapi_context context;
    c_bool contains;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_DATAREADER);

    subscriber = gapi_subscriberClaimNB(_this, &result);

    if ( subscriber ) {
        if ( !subscriber->builtin ) {
            datareader = gapi_dataReaderClaimNB(a_datareader, NULL);
            if ( datareader ) {
                contains = u_subscriberContainsReader(U_SUBSCRIBER_GET(subscriber),
                                                      U_READER_GET(datareader));
                if (contains) {
                    if ( _DataReaderPrepareDelete (datareader, &context) ) {
                        _DataReaderFree(datareader);
                        datareader = NULL;
                    } else {
                        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                    }
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
                _EntityRelease(datareader);
            } else {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }
        }
        _EntityRelease(subscriber);
    }
    return result;
}

gapi_returnCode_t
_SubscriberDeleteContainedEntities (
    _Subscriber _this)
{
    gapi_dataReader handle;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader dataReader;
    c_iter readers;
    u_dataReader r;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _EntityHandle(_this), GAPI_METHOD_DELETE_CONTAINED_ENTITIES);

    if ( _this ) {
        readers = u_subscriberLookupReaders(U_SUBSCRIBER_GET(_this),NULL);

        r = c_iterTakeFirst(readers);
        while (r) {
            handle = u_entityGetUserData(u_entity(r));
            result = gapi_dataReader_delete_contained_entities(handle);
            if (result == GAPI_RETCODE_OK) {
                dataReader = gapi_dataReaderClaimNB(handle,&result);
                if (dataReader) {
                    if ( _DataReaderPrepareDelete(dataReader, &context) ) {
                        _DataReaderFree(dataReader);
                    } else if (result == GAPI_RETCODE_OK) {
                        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                    }
                }
            } else if (result == GAPI_RETCODE_ALREADY_DELETED) {
                result = GAPI_RETCODE_OK;
            }
            r = c_iterTakeFirst(readers);
        }
        c_iterFree(readers);
    }
    return result;
}

gapi_returnCode_t
gapi_subscriber_delete_contained_entities (
    gapi_subscriber _this)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Subscriber subscriber;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_CONTAINED_ENTITIES);

    subscriber = gapi_subscriberClaim(_this, &result);

    if (subscriber) {
        result = _SubscriberDeleteContainedEntities(subscriber);
        _EntityRelease(subscriber);
    }

    return result;
}

gapi_dataReader
gapi_subscriber_lookup_datareader (
    gapi_subscriber _this,
    const gapi_char *topic_name)
{
    _Subscriber subscriber;
    gapi_dataReader handle = NULL;
    u_dataReader found;
    c_iter iter;

    subscriber = gapi_subscriberClaim(_this, NULL);

    if (subscriber) {
        iter = u_subscriberLookupReaders(U_SUBSCRIBER_GET(subscriber),
                                         topic_name);
        if (iter) {
            found = c_iterTakeFirst(iter);
            if (found) {
                handle = u_entityGetUserData(u_entity(found));
            }
            c_iterFree(iter);
        }
        _EntityRelease(subscriber);
    }
    return handle;
}


gapi_dataReaderSeq *
allocDataReaderSeq (
    gapi_unsigned_long len)
{
    gapi_dataReaderSeq *list;

    list = gapi_dataReaderSeq__alloc();
    if ( list && (len > 0) ) {
        list->_buffer = gapi_dataReaderSeq_allocbuf(len);
        if ( list->_buffer ) {
            list->_maximum = len;
            list->_release = TRUE;
        } else {
            gapi_free(list);
            list = NULL;
        }
    }
    return list;
}


gapi_returnCode_t
gapi_subscriber_get_datareaders (
    gapi_subscriber _this,
    gapi_dataReaderSeq *readers,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

static c_bool
trigger_reader(
    u_dataReader reader,
    c_voidp arg)
{
    gapi_dataReader handle;
    _DataReader dataReader;

    assert(reader);

    handle = u_entityGetUserData(u_entity(reader));
    dataReader = gapi_dataReaderClaim(handle,NULL);
    if (dataReader) {
        _DataReaderTriggerNotify(dataReader);
        _EntityRelease(dataReader);
    }
    return TRUE;
}

gapi_returnCode_t
gapi_subscriber_notify_datareaders (
    gapi_subscriber _this)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Subscriber subscriber;

    subscriber = gapi_subscriberClaim(_this, &result);
    if ( subscriber ) {
        u_subscriberWalkReaders(U_SUBSCRIBER_GET(subscriber),
                                (u_readerAction)trigger_reader,NULL);
        _EntityRelease(subscriber);
    }
    return result;
}


gapi_returnCode_t
gapi_subscriber_set_qos (
    gapi_subscriber _this,
    const gapi_subscriberQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_result uResult;
    _Subscriber subscriber;
    v_subscriberQos subscriberQos;
    gapi_context        context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_QOS);

    subscriber = gapi_subscriberClaim(_this, &result);

    if ( subscriber && qos ) {
        result = gapi_subscriberQosIsConsistent(qos, &context);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    if ((result == GAPI_RETCODE_OK ) && (_EntityEnabled(subscriber))) {
        gapi_subscriberQos * existing_qos = gapi_subscriberQos__alloc();

        result = gapi_subscriberQosCheckMutability(qos,
                                                   _SubscriberGetQos(subscriber,
                                                                     existing_qos),
                                                   &context);
        gapi_free(existing_qos);
    }



    if ( result == GAPI_RETCODE_OK ) {
        subscriberQos = u_subscriberQosNew(NULL);
        if (subscriberQos) {
            if ( copySubscriberQosIn(qos, subscriberQos) ) {
                uResult = u_entitySetQoS(_EntityUEntity(subscriber),
                                         (v_qos)(subscriberQos) );
                result = kernelResultToApiResult(uResult);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
            u_subscriberQosFree(subscriberQos);
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    }

    _EntityRelease(subscriber);

    return result;
}

gapi_returnCode_t
gapi_subscriber_get_qos (
    gapi_subscriber _this,
    gapi_subscriberQos *qos)
{
    _Subscriber subscriber;
    gapi_returnCode_t result;

    if (qos) {
        subscriber = gapi_subscriberClaim(_this, &result);
        if (subscriber) {
            _SubscriberGetQos(subscriber, qos);
            _EntityRelease(subscriber);
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}

gapi_returnCode_t
gapi_subscriber_set_listener (
    gapi_subscriber _this,
    const struct gapi_subscriberListener *a_listener,
    const gapi_statusMask mask)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Subscriber subscriber = (_Subscriber)_this;

    subscriber = gapi_subscriberClaim(_this, &result);

    if ( (subscriber != NULL) ) {
        _Status status;

        if ( a_listener ) {
            subscriber->_Listener = *a_listener;
        } else {
            memset(&subscriber->_Listener, 0, sizeof(subscriber->_Listener));
        }

        status = _EntityStatus(subscriber);
        if ( _StatusSetListener(status,
                                (struct gapi_listener *)a_listener,
                                mask) )
        {
            result = GAPI_RETCODE_OK;
        }
        _EntityRelease(subscriber);
    }
    return result;
}


struct gapi_subscriberListener
gapi_subscriber_get_listener (
    gapi_subscriber _this)
{
    _Subscriber subscriber;
    struct gapi_subscriberListener listener;

    subscriber = gapi_subscriberClaim(_this, NULL);

    if ( subscriber != NULL) {
        listener = subscriber->_Listener;
        _EntityRelease(subscriber);
    } else {
        memset(&listener, 0, sizeof(listener));
    }
    return listener;
}

gapi_returnCode_t
gapi_subscriber_begin_access (
    gapi_subscriber _this)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_subscriber_end_access (
    gapi_subscriber _this)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_domainParticipant
gapi_subscriber_get_participant (
    gapi_subscriber _this)
{
    _Subscriber subscriber;
    _DomainParticipant participant = NULL;

    subscriber = gapi_subscriberClaim(_this, NULL);

    if ( subscriber != NULL) {
        participant = _EntityParticipant(_Entity(subscriber));
        _EntityRelease(subscriber);
    }

    return (gapi_domainParticipant)_EntityHandle(participant);
}

gapi_returnCode_t
gapi_subscriber_set_default_datareader_qos (
    gapi_subscriber _this,
    const gapi_dataReaderQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Subscriber subscriber = (_Subscriber)_this;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_DEFAULT_DATAREADER_QOS);

    subscriber = gapi_subscriberClaim(_this, &result);
    if (result == GAPI_RETCODE_OK) {
        if (qos == GAPI_SUBSCRIBER_QOS_DEFAULT) {
            qos = &gapi_dataReaderQosDefault;
        }
        result = gapi_dataReaderQosIsConsistent(qos, &context);
        if (result == GAPI_RETCODE_OK) {
            gapi_dataReaderQosCopy (qos, &subscriber->_defDataReaderQos);
        }
        _EntityRelease(subscriber);
    }

    return result;
}

gapi_returnCode_t
gapi_subscriber_get_default_datareader_qos (
    gapi_subscriber _this,
    gapi_dataReaderQos *qos)
{
    _Subscriber subscriber;
    gapi_returnCode_t result;

    if (qos) {
        subscriber = gapi_subscriberClaim(_this, &result);
        if (result == GAPI_RETCODE_OK) {
            gapi_dataReaderQosCopy (&subscriber->_defDataReaderQos, qos);
            _EntityRelease(subscriber);
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

gapi_returnCode_t
gapi_subscriber_copy_from_topic_qos (
    gapi_subscriber _this,
    gapi_dataReaderQos *a_datareader_qos,
    const gapi_topicQos *a_topic_qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    if ((a_topic_qos      != GAPI_TOPIC_QOS_DEFAULT) &&
        (a_datareader_qos != GAPI_DATAREADER_QOS_DEFAULT))
    {
        gapi_mergeTopicQosWithDataReaderQos(a_topic_qos, a_datareader_qos);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

struct check_handle_arg {
    gapi_instanceHandle_t handle;
    gapi_boolean result;
};

static c_bool
check_handle(
    u_dataReader dataReader,
    struct check_handle_arg *arg)
{
    gapi_dataReader handle;
    _Entity e;

    assert(dataReader);
    assert(arg);

    if (!arg->result) {
        handle = u_entityGetUserData(u_entity(dataReader));
        e = _Entity(gapi_objectPeekUnchecked(handle));
        if (e) {
            arg->result = _EntityHandleEqual(e,arg->handle);
        }
    }
    return !arg->result;
}

gapi_boolean
_SubscriberContainsEntity (
    _Subscriber _this,
    gapi_instanceHandle_t handle)
{
    struct check_handle_arg arg;

    assert(_this);

    _EntityClaim(_this);

    arg.handle = handle;
    arg.result = FALSE;

    u_subscriberWalkReaders(U_SUBSCRIBER_GET(_this),
                           (u_readerAction)check_handle,
                           (c_voidp)&arg);

    _EntityRelease(_this);

    return arg.result;
}

void
_SubscriberNotifyListener(
    _Subscriber _this,
    gapi_statusMask triggerMask)
{
    gapi_object source;
    _Status status;

    if ( _this && (triggerMask & GAPI_DATA_ON_READERS_STATUS) ) {
        status = _EntityStatus(_this);
        source = _EntityHandle(_this);
        _StatusNotifyDataOnReaders(status, source);
    }
}
