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
#include "gapi_subscriber.h"
#include "gapi_domainParticipant.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_map.h"
#include "gapi_domainEntity.h"
#include "gapi_structured.h"
#include "gapi_typeSupport.h"
#include "gapi_topic.h"
#include "gapi_topicDescription.h"
#include "gapi_contentFilteredTopic.h"
#include "gapi_dataReader.h"
#include "gapi_builtin.h"
#include "gapi_qos.h"
#include "gapi_subscriberStatus.h"
#include "gapi_objManag.h"
#include "gapi_kernel.h"
#include "gapi_error.h"

#include "os_heap.h"
#include "u_user.h"

static v_subscriberQos
newSubscriberQos (
    void)
{
    v_subscriberQos subscriberQos;

    subscriberQos = u_subscriberQosNew(NULL);
    if ( subscriberQos ) {
        if ( subscriberQos->partition ) {
            os_free(subscriberQos->partition);
            subscriberQos->partition = NULL;
        }
    }
    return subscriberQos;
}

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
    v_participantQos participantQos;
    v_subscriberQos subscriberQos;
    c_bool enable = TRUE;
    gapi_long len;

    assert(uParticipant);
    assert(qos);
    assert(participant);

    newSubscriber = _SubscriberAlloc();

    if ( newSubscriber != NULL ) {
        if ( u_entityQoS(u_entity(uParticipant), (v_qos*)&participantQos) == U_RESULT_OK ) {
            enable = participantQos->entityFactory.autoenable_created_entities;
            u_participantQosFree(participantQos);
        }
        _DomainEntityInit(_DomainEntity(newSubscriber), participant, _Entity(participant), enable);
        gapi_dataReaderQosCopy (&gapi_dataReaderQosDefault, &newSubscriber->_defDataReaderQos);
        if ( a_listener ) {
            newSubscriber->_Listener = *a_listener;
        }
        newSubscriber->dataReaderSet = gapi_setNew (gapi_objectRefCompare);
        if (newSubscriber->dataReaderSet == NULL) {
            _DomainEntityDispose(_DomainEntity(newSubscriber));
            os_free (newSubscriber);
            newSubscriber = NULL;
        }
    }

    if  (newSubscriber != NULL ) {
        subscriberQos = newSubscriberQos ();
        if ( subscriberQos != NULL ) {
            if ( !copySubscriberQosIn(qos, subscriberQos) ) {
                _DomainEntityDispose(_DomainEntity(newSubscriber));
                newSubscriber = NULL;
            }
        } else {
            _DomainEntityDispose(_DomainEntity(newSubscriber));
            newSubscriber = NULL;
        }
    }

    if ( newSubscriber != NULL) {
        u_subscriber uSubscriber;

        uSubscriber = u_subscriberNew(uParticipant, "subscriber", subscriberQos, enable);
        u_subscriberQosFree(subscriberQos);
        if ( uSubscriber != NULL ) {
            U_SUBSCRIBER_SET(newSubscriber, uSubscriber);
        } else {
            gapi_setFree(newSubscriber->dataReaderSet);
            _DomainEntityDispose(_DomainEntity(newSubscriber));
            newSubscriber = NULL;
        }
    }

    if ( newSubscriber != NULL) {
        _EntityStatus(newSubscriber) = _Status(_SubscriberStatusNew(newSubscriber, a_listener,mask));
        if ( _EntityStatus(newSubscriber) != NULL ) {
            len = (gapi_long)qos->partition.name._length;
            if ( qos->partition.name._length == 0UL ) {
                /*
                 * behaviour of the kernel in case of an empty sequence is that it is related no
                 * none of the partitions, while DCPS expects it to be conected to all partitions.
                 * Therefore this has to be done seperately.
                 */
                u_subscriberSubscribe (U_SUBSCRIBER_GET(newSubscriber), "");
            }
            newSubscriber->builtin = FALSE;
        } else {
             gapi_setFree(newSubscriber->dataReaderSet);
            _DomainEntityDispose(_DomainEntity(newSubscriber));
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

    assert(subscriber);

    _SubscriberStatusSetListener(_SubscriberStatus(_Entity(subscriber)->status), NULL, 0);

    _SubscriberStatusFree(_SubscriberStatus(_Entity(subscriber)->status));

    _EntityFreeStatusCondition(_Entity(subscriber));

    if (u_subscriberFree(U_SUBSCRIBER_GET(subscriber)) != U_RESULT_OK) {
        result = GAPI_RETCODE_ERROR;
    }
    gapi_setFree (subscriber->dataReaderSet);
    subscriber->dataReaderSet = NULL;

    gapi_dataReaderQos_free(&subscriber->_defDataReaderQos);

    _DomainEntityDispose(_DomainEntity(subscriber));

    return result;
}

gapi_boolean
_SubscriberPrepareDelete (
    _Subscriber subscriber)
{
    gapi_boolean result;

    assert(subscriber);

    if (gapi_setIsEmpty (subscriber->dataReaderSet)) {
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

u_subscriber
_SubscriberUsubscriber (
    _Subscriber subscriber)
{
    assert(subscriber);
    return U_SUBSCRIBER_GET(subscriber);
}

_DomainParticipant
_SubscriberParticipant (
    _Subscriber subscriber)
{
    assert(subscriber);
    return _DomainEntityParticipant(_DomainEntity(subscriber));
}

void
_SubscriberSetDeleteAction (
    _Subscriber subscriber,
    gapi_deleteEntityAction action,
    void *argument)
{
    gapi_setIter iter;
    _DataReader reader;

    _ObjectSetDeleteAction(_Object(subscriber), action, argument);
    iter = gapi_setFirst(subscriber->dataReaderSet);
    if ( iter ) {
        reader = (_DataReader)gapi_setIterObject(iter);
        while ( reader ) {
            _EntityClaim(reader);
            _DataReaderSetDeleteAction(reader, action, argument);
            _EntityRelease(reader);
            gapi_setIterNext(iter);
            reader = _DataReader(gapi_setIterObject(iter));
        }
        gapi_setIterFree(iter);
    }
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

            /* find topic with the participant for consistency */
            typeName  = _TopicDescriptionGetTypeName(topicDescription);
            topicName = _TopicDescriptionGetName(topicDescription);
            participant = _DomainEntityParticipant(_DomainEntity(subscriber));

            if ( _DomainParticipantTopicDescriptionExists(participant,
                                                          topicDescription))
            {
                _TypeSupport typeSupport;

                /* find type support for the data type to find
                 * data reader create function.
                 */
                typeSupport = _DomainParticipantFindType(participant, typeName);
                /* if create function is NULL, take default from data reader */
                if (_TypeSupportGetDataReader(typeSupport) == NULL) {
                    datareader = _DataReaderNew(topicDescription,
                                                typeSupport,
                                                readerQos,
                                                a_listener,
                                                mask,
                                                subscriber);
                } else {
                    datareader =
                        (_DataReader)_TypeSupportGetDataReader(typeSupport)(
                                         a_topic,
                                         readerQos,
                                         a_listener,
                                         mask,
                                         _this);
                }
                if ( datareader ) {
                    gapi_setAdd (subscriber->dataReaderSet,
                                 (gapi_object)datareader);
                    _ENTITY_REGISTER_OBJECT(_Entity(subscriber),
                                            (_Object)datareader);
                }
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

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_DATAREADER);

    subscriber = gapi_subscriberClaimNB(_this, &result);

    if ( subscriber ) {
        if ( !subscriber->builtin ) {
            datareader = gapi_dataReaderClaimNB(a_datareader, NULL);
            if ( datareader == NULL ) {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }
        }
    }

    if ( datareader ) {
        gapi_setIter dataReaderIter = gapi_setFind(subscriber->dataReaderSet,
                                                   (gapi_object)datareader);
        if ( dataReaderIter != NULL ) {
            if ( gapi_setIterObject(dataReaderIter) ) {
                if ( _DataReaderPrepareDelete (datareader, &context) ) {
                    gapi_setRemove(subscriber->dataReaderSet,
                                   (gapi_object)datareader);
                    _DataReaderFree(datareader);
                    datareader = NULL;
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            gapi_setIterFree(dataReaderIter);
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
        _EntityRelease(datareader);
    }

    _EntityRelease(subscriber);

     return result;
}

gapi_returnCode_t
gapi_subscriber_delete_contained_entities (
    gapi_subscriber _this,
    gapi_deleteEntityAction action,
    void *action_arg)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Subscriber subscriber = (_Subscriber)_this;
    void *userData;
    gapi_context context;
    gapi_setIter iterSet;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_CONTAINED_ENTITIES);

    subscriber = gapi_subscriberClaim(_this, &result);

    if ( subscriber ) {
        iterSet = gapi_setFirst (subscriber->dataReaderSet);

        while ((gapi_setIterObject(iterSet)) && (result == GAPI_RETCODE_OK)) {
            _DataReader datareader = _DataReader(gapi_setIterObject(iterSet));
            result = gapi_dataReader_delete_contained_entities(
                          _EntityHandle(datareader),
                           action,
                           action_arg);

            if(result == GAPI_RETCODE_OK){
                _EntityClaimNotBusy(datareader);
                userData = _ObjectGetUserData(_Object(datareader));
                if ( _DataReaderPrepareDelete(datareader, &context) ) {
                    _DataReaderFree(datareader);
                } else if (result == GAPI_RETCODE_OK) {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
                gapi_setIterRemove(iterSet);
                if ( action ) {
                    action(userData, action_arg);
                }
            }
        }
        gapi_setIterFree (iterSet);
        _EntityRelease(subscriber);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}

_DataReader
_SubscriberLookupDatareader (
    _Subscriber subscriber,
    const gapi_char *topicName)
{
    _DataReader  dataReader = NULL;
    gapi_setIter iter;

    assert(subscriber);
    assert(topicName);

    iter = gapi_setFirst(subscriber->dataReaderSet);
    while ( (dataReader == NULL) && (gapi_setIterObject(iter) != NULL) ) {
        _DataReader dr = _DataReader(gapi_setIterObject(iter));

        assert(dr->topicDescription);
        assert(dr->topicDescription->topic_name);
        if ( strcmp(dr->topicDescription->topic_name, topicName) == 0 ) {
            dataReader = dr;
        } else {
            gapi_setIterNext (iter);
        }
    }
    gapi_setIterFree(iter);

    return dataReader;
}

gapi_dataReader
gapi_subscriber_lookup_datareader (
    gapi_subscriber _this,
    const gapi_char *topic_name)
{
    _Subscriber subscriber = (_Subscriber)_this;
    _DataReader dataReader = NULL;

    subscriber = gapi_subscriberClaim(_this, NULL);

    if ( subscriber && topic_name ) {
        gapi_setIter       iter;

        iter = gapi_setFirst(subscriber->dataReaderSet);
        while ( (dataReader == NULL) && (gapi_setIterObject(iter) != NULL) ) {
            _DataReader dr = _DataReader(gapi_setIterObject(iter));
            gapi_string name;

            assert(dr->topicDescription);
            name = _TopicDescriptionGetName(dr->topicDescription);
            assert(name);
            if ( strcmp(name, topic_name) == 0 ) {
                dataReader = dr;
            } else {
                gapi_setIterNext (iter);
            }
            gapi_free(name);
        }
        gapi_setIterFree(iter);
    }
    _EntityRelease(subscriber);

    return (gapi_dataReader)_EntityHandle(dataReader);
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

gapi_returnCode_t
gapi_subscriber_notify_datareaders (
    gapi_subscriber _this)
{
    gapi_returnCode_t result;
    _Subscriber subscriber;

    subscriber = gapi_subscriberClaim(_this, &result);
    if ( subscriber ) {
        gapi_setIter iter;

        iter = gapi_setFirst(subscriber->dataReaderSet);
        if ( iter ) {
            _DataReader  dataReader;
            dataReader = (_DataReader)gapi_setIterObject(iter);
            while ( dataReader ) {
                _DataReaderTriggerNotify(dataReader);
                gapi_setIterNext(iter);
                dataReader = (_DataReader)gapi_setIterObject(iter);
            }
            gapi_setIterFree(iter);
        }
    }

    _EntityRelease(subscriber);
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

    if ((result == GAPI_RETCODE_OK ) && (_Entity(subscriber)->enabled)) {
        gapi_subscriberQos * existing_qos = gapi_subscriberQos__alloc();

        result = gapi_subscriberQosCheckMutability(qos,
                                                   _SubscriberGetQos(subscriber,existing_qos),
                                                   &context);
        gapi_free(existing_qos);
    }



    if ( result == GAPI_RETCODE_OK ) {
        subscriberQos = newSubscriberQos();
        if (subscriberQos) {
            if ( copySubscriberQosIn(qos, subscriberQos) ) {
                uResult = u_entitySetQoS(_EntityUEntity(subscriber),(v_qos)(subscriberQos) );
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

    subscriber = gapi_subscriberClaim(_this, &result);
    if ( subscriber && qos ) {
        _SubscriberGetQos(subscriber, qos);
    }

    _EntityRelease(subscriber);
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
        _SubscriberStatus status;

        if ( a_listener ) {
            subscriber->_Listener = *a_listener;
        } else {
            memset(&subscriber->_Listener, 0, sizeof(subscriber->_Listener));
        }

        status = _SubscriberStatus(_EntityStatus(subscriber));
        if ( _SubscriberStatusSetListener(status, a_listener, mask) ) {
            result = GAPI_RETCODE_OK;
        }
    }

    _EntityRelease(subscriber);

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
    } else {
        memset(&listener, 0, sizeof(listener));
    }
    _EntityRelease(subscriber);

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
        participant = _DomainEntityParticipant(_DomainEntity(subscriber));
    }

    _EntityRelease(subscriber);

    return (gapi_domainParticipant)_EntityHandle(participant);
}

gapi_returnCode_t
gapi_subscriber_set_default_datareader_qos (
    gapi_subscriber _this,
    const gapi_dataReaderQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Subscriber subscriber = (_Subscriber)_this;
    gapi_context        context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_DEFAULT_DATAREADER_QOS);

    subscriber = gapi_subscriberClaim(_this, &result);

    if ( subscriber ) {
        if ( qos ) {
            result = gapi_dataReaderQosIsConsistent(qos, &context);
            if ( result == GAPI_RETCODE_OK ) {
                gapi_dataReaderQosCopy (qos, &subscriber->_defDataReaderQos);
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(subscriber);

    return result;
}

gapi_returnCode_t
gapi_subscriber_get_default_datareader_qos (
    gapi_subscriber _this,
    gapi_dataReaderQos *qos)
{
    _Subscriber subscriber;
    gapi_returnCode_t result;

    subscriber = gapi_subscriberClaim(_this, &result);

    if ( subscriber && qos ) {
        gapi_dataReaderQosCopy (&subscriber->_defDataReaderQos, qos);
    }
    _EntityRelease(subscriber);
    return result;
}

gapi_returnCode_t
gapi_subscriber_copy_from_topic_qos (
    gapi_subscriber _this,
    gapi_dataReaderQos *a_datareader_qos,
    const gapi_topicQos *a_topic_qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Subscriber subscriber = (_Subscriber)_this;

    subscriber = gapi_subscriberClaim(_this, &result);

    if ( (subscriber != NULL) ) {
        if ((a_topic_qos      != GAPI_TOPIC_QOS_DEFAULT) &&
            (a_datareader_qos != GAPI_DATAREADER_QOS_DEFAULT)) {
            gapi_mergeTopicQosWithDataReaderQos(a_topic_qos, a_datareader_qos);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }
    _EntityRelease(subscriber);

    return result;
}

gapi_boolean
_SubscriberSetListenerInterestOnChildren (
    _Subscriber subscriber,
    _ListenerInterestInfo info)
{
    gapi_setIter iterSet;
    _Entity      entity;
    gapi_boolean result = TRUE;

    assert(subscriber);

    iterSet = gapi_setFirst(subscriber->dataReaderSet);
    while (result && gapi_setIterObject(iterSet)) {
        entity = _Entity(gapi_setIterObject(iterSet));
        result = _EntitySetListenerInterest(entity, info);
        gapi_setIterNext(iterSet);
    }
    gapi_setIterFree (iterSet);

    return result;
}

gapi_boolean
_SubscriberContainsEntity (
    _Subscriber subscriber,
    gapi_instanceHandle_t handle)
{
    gapi_boolean result = FALSE;
    gapi_setIter iterSet;

    assert(subscriber);

    _EntityClaim(subscriber);

    iterSet = gapi_setFirst(subscriber->dataReaderSet);
    while ( !result && gapi_setIterObject(iterSet) ) {
        _DataReader reader = _DataReader(gapi_setIterObject(iterSet));
        result = _EntityHandleEqual(_Entity(reader), handle);
        gapi_setIterNext(iterSet);
    }
    gapi_setIterFree (iterSet);

    _EntityRelease(subscriber);

    return result;
}

void
_SubscriberOnDataOnReaders (
    _Subscriber _this)
{
    gapi_listener_DataOnReadersListener callback;
    gapi_object target;
    gapi_object source;
    _Entity entity;
    _Status status;
    c_voidp listenerData;

    if (_this) {
        status = _Entity(_this)->status;
        target = _StatusFindTarget(status, GAPI_DATA_ON_READERS_STATUS);
        if (target) {
            source = _EntityHandle(_this);
            if ( target != source ) {
                entity = gapi_entityClaim(target, NULL);
                status = entity->status;
            } else {
                entity = NULL;
            }

            callback = status->callbackInfo.on_data_on_readers;
            listenerData = status->callbackInfo.listenerData;

            _EntitySetBusy(_this);
            _EntityRelease(_this);

            if (entity) {
                _EntitySetBusy(entity);
                _EntityRelease(entity);
                callback(listenerData, source);
                gapi_objectClearBusy(target);
            } else {
                callback(listenerData, source);
            }

            gapi_objectClearBusy(source);
            gapi_entityClaim(source, NULL);
        }
    }
}

void
_SubscriberNotifyListener(
    _Subscriber _this,
    gapi_statusMask triggerMask)
{
    if ( triggerMask & GAPI_DATA_ON_READERS_STATUS ) {
        _SubscriberOnDataOnReaders(_this);
    }
}
