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
#include "gapi_publisher.h"
#include "gapi_domainParticipant.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_dataWriter.h"
#include "gapi_topicDescription.h"
#include "gapi_topic.h"
#include "gapi_kernel.h"
#include "gapi_set.h"
#include "gapi_domainEntity.h"
#include "gapi_structured.h"
#include "gapi_typeSupport.h"
#include "gapi_qos.h"
#include "gapi_publisherStatus.h"
#include "gapi_objManag.h"
#include "gapi_error.h"

#include "os_heap.h"
#include "u_user.h"

#define U_PUBLISHER_SET(p,e)     _EntitySetUserEntity(_Entity(p), u_entity(e))


C_STRUCT(_Publisher) {
    C_EXTENDS(_DomainEntity);
    gapi_dataWriterQos             _defDataWriterQos;
    struct gapi_publisherListener  _Listener;
    gapi_set                       dataWriterSet;
};

static v_publisherQos
newPublisherQos (
    void)
{
    v_publisherQos publisherQos;

    publisherQos = u_publisherQosNew(NULL);
    if ( publisherQos ) {
        if ( publisherQos->partition ) {
            os_free(publisherQos->partition);
            publisherQos->partition = NULL;
        }
    }
    return publisherQos;
}

static gapi_boolean
copyPublisherQosIn (
    const gapi_publisherQos *srcQos,
    v_publisherQos dstQos)
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
    }

    return copied;
}

static gapi_boolean
copyPublisherQosOut (
    const v_publisherQos  srcQos,
    gapi_publisherQos *dstQos)
{
    assert(srcQos);
    assert(dstQos);
    
    if ( dstQos->group_data.value._maximum > 0 ) {
        if ( dstQos->group_data.value._release ) {
            gapi_free(dstQos->group_data.value._buffer);
        }
    }

    if ( (srcQos->groupData.size > 0) && srcQos->groupData.value ) {
        dstQos->group_data.value._buffer =
                gapi_octetSeq_allocbuf(srcQos->groupData.size);
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
        
    return TRUE;
}

_Publisher
_PublisherNew (
    u_participant uParticipant,
    const gapi_publisherQos *qos,
    const struct gapi_publisherListener *a_listener,
    const gapi_statusMask mask,
    const _DomainParticipant participant)
{
    v_participantQos participantQos;
    v_publisherQos publisherQos;
    _Publisher newPublisher;
    c_bool enable = TRUE;

    assert(uParticipant);
    assert(qos);
    assert(participant);

    newPublisher = _PublisherAlloc();

    if ( newPublisher ) {
        if ( u_entityQoS(u_entity(uParticipant), (v_qos*)&participantQos) == U_RESULT_OK ) {
            enable = participantQos->entityFactory.autoenable_created_entities;
            u_participantQosFree(participantQos);
        }
        _DomainEntityInit (_DomainEntity(newPublisher),
                           participant,
                           _Entity(participant),
                           enable);
        gapi_dataWriterQosCopy (&gapi_dataWriterQosDefault,
                                &newPublisher->_defDataWriterQos);
        if ( a_listener ) {
            newPublisher->_Listener = *a_listener;
        }
        newPublisher->dataWriterSet = gapi_setNew (gapi_objectRefCompare);
        if ( newPublisher->dataWriterSet == NULL ) {
            _DomainEntityDispose(_DomainEntity(newPublisher));
            newPublisher = NULL;
        }
    }

    if ( newPublisher ) {
        publisherQos = newPublisherQos ();
        if ( publisherQos ) {
            if ( !copyPublisherQosIn(qos, publisherQos)) {
                _DomainEntityDispose(_DomainEntity(newPublisher));
                newPublisher = NULL;
            }
        } else {
            gapi_setFree(newPublisher->dataWriterSet);
            _DomainEntityDispose(_DomainEntity(newPublisher));
            newPublisher = NULL;
        }
    }
        
    if ( newPublisher  ) {        
        u_publisher uPublisher;
        
        uPublisher = u_publisherNew (uParticipant,
                                     "publisher",
                                     publisherQos,
                                     enable);
        u_publisherQosFree(publisherQos);
        if ( uPublisher ) {
            U_PUBLISHER_SET(newPublisher, uPublisher);
        } else {
            gapi_setFree(newPublisher->dataWriterSet);
            _DomainEntityDispose(_DomainEntity(newPublisher));
            newPublisher = NULL;
        }
    }

    if ( newPublisher ) {        
        _EntityStatus(newPublisher) = _Status(_PublisherStatusNew(newPublisher, a_listener,mask));
        if ( _EntityStatus(newPublisher) ) {
            if ( qos->partition.name._length == 0 ) {
                /* 
                 * behaviour of the kernel in case of an empty sequence is that it is related no
                 * none of the partitions, while DCPS expects it to be conected to all partitions.
                 * Therefore this has to be done seperately.
                 */
                u_publisherPublish (U_PUBLISHER_GET(newPublisher), "");
            }
        } else {
            u_publisherFree(U_PUBLISHER_GET(newPublisher));
            gapi_setFree(newPublisher->dataWriterSet);
            _DomainEntityDispose(_DomainEntity(newPublisher));
            newPublisher = NULL;
         }
    }
         
    return newPublisher;
}

gapi_returnCode_t
_PublisherFree (
    _Publisher _this)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    assert(_this);

    _PublisherStatusSetListener(_PublisherStatus(_Entity(_this)->status), NULL, 0);

    _PublisherStatusFree(_PublisherStatus(_Entity(_this)->status));

    _EntityFreeStatusCondition(_Entity(_this));

    if (u_publisherFree(U_PUBLISHER_GET(_this)) != U_RESULT_OK) {
        result = GAPI_RETCODE_ERROR;
    }
    
    gapi_setFree (_this->dataWriterSet);
    _this->dataWriterSet = NULL;
    
    gapi_dataWriterQos_free(&_this->_defDataWriterQos);
    
    _DomainEntityDispose (_DomainEntity(_this));

    return result;
}

gapi_boolean
_PublisherPrepareDelete (
    _Publisher _this)
{
    gapi_boolean result;

    assert(_this);

    if ( gapi_setIsEmpty (_this->dataWriterSet) ) {
        result = TRUE;
    } else {
        result = FALSE;
    }

    return result;
}

u_publisher
_PublisherUpublisher (
    _Publisher _this)
{
    assert(_this);
    return U_PUBLISHER_GET(_this);
}

gapi_publisherQos *
_PublisherGetQos (
    _Publisher _this,
    gapi_publisherQos *qos)
{
    v_publisherQos publisherQos;
    u_publisher uPublisher;
    
    assert(_this);

    uPublisher = U_PUBLISHER_GET(_this);
        
    if ( u_entityQoS(u_entity(uPublisher), (v_qos*)&publisherQos) == U_RESULT_OK ) {
        copyPublisherQosOut(publisherQos,  qos);
        u_publisherQosFree(publisherQos);
    }

    return qos;
}

gapi_dataWriter
gapi_publisher_create_datawriter (
    gapi_publisher _this,
    const gapi_topic a_topic,
    const gapi_dataWriterQos *qos,
    const struct gapi_dataWriterListener *a_listener,
    const gapi_statusMask mask)
{
    _Publisher publisher;
    _DataWriter datawriter = NULL;
    gapi_dataWriter result = NULL;
    _Topic          topic  = NULL;
    gapi_dataWriterQos *writerQos;
    gapi_context context;
    gapi_topicQos* topicQos;
    gapi_returnCode_t rc;
    
    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_DATAWRITER);

    publisher = gapi_publisherClaim(_this, NULL);

    if ( publisher ) {
        topic = _TopicFromHandle(a_topic);
    } 

    if ( topic ) { 
        if ( qos == GAPI_DATAWRITER_QOS_DEFAULT ) {
            writerQos = &publisher->_defDataWriterQos;
        } else if ( qos == GAPI_DATAWRITER_QOS_USE_TOPIC_QOS ) {
            topicQos = gapi_topicQos__alloc();
            writerQos = gapi_dataWriterQos__alloc();
            gapi_dataWriterQosCopy(&publisher->_defDataWriterQos, writerQos);
            _TopicGetQos(topic, topicQos);
            gapi_mergeTopicQosWithDataWriterQos(topicQos,writerQos);
            gapi_free(topicQos);
        } else {
            writerQos = (gapi_dataWriterQos *)qos;
        }

        if ( gapi_dataWriterQosIsConsistent(writerQos, &context) == GAPI_RETCODE_OK ) {
            gapi_char *typeName;
            gapi_char *topicName;
            _DomainParticipant participant;
            _TypeSupport typeSupport;
            
            /* find topic with the participant for consistency */
            typeName  = _TopicGetTypeName(topic);
            topicName = _TopicGetName(topic);
            participant = _DomainEntityParticipant(_DomainEntity(publisher));

            /* find type support for the data type to find data writer create function */
            typeSupport = _DomainParticipantFindType(participant, typeName);
            /* if create function is NULL, take default from data writer */
            if ( _TypeSupportGetDataWriter(typeSupport) == NULL ) {
                datawriter = _DataWriterNew(topic, typeSupport, writerQos, a_listener, mask, publisher);
            } else {
                result = _TypeSupportGetDataWriter(typeSupport)(a_topic, writerQos, a_listener, mask, _this);
                datawriter = gapi_dataWriterClaim(_this, &rc);
            }
            if ( datawriter ) {
                gapi_setAdd(publisher->dataWriterSet, (gapi_object)datawriter);
                _ENTITY_REGISTER_OBJECT(_Entity(publisher), (_Object)datawriter);
            }
            gapi_free(typeName);
            gapi_free(topicName);
        }

        if (qos == GAPI_DATAWRITER_QOS_USE_TOPIC_QOS) {
            gapi_free(writerQos);
        }
    }

    _EntityRelease(publisher);

    if ( datawriter ) {
        gapi_object statusHandle;
        statusHandle = _EntityHandle(_Entity(datawriter)->status);
        result = (gapi_dataWriter)_EntityRelease(datawriter);
    }
    
    return result;
}

gapi_returnCode_t
gapi_publisher_delete_datawriter (
    gapi_publisher _this,
    const gapi_dataWriter a_datawriter)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Publisher publisher;
    _DataWriter datawriter = NULL;

    publisher = gapi_publisherClaim(_this, &result);

    if ( publisher ) {
        datawriter = gapi_dataWriterClaimNB(a_datawriter, NULL);
        if ( datawriter == NULL ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if ( datawriter ) {
        gapi_setIter dataWriterIter = gapi_setFind(publisher->dataWriterSet,
                                                   (gapi_object)datawriter);
        if ( dataWriterIter != NULL ) {
            if ( gapi_setIterObject(dataWriterIter) ) {
                if ( _DataWriterPrepareDelete(datawriter) ) {
                    gapi_setRemove(publisher->dataWriterSet,
                                   (gapi_object)datawriter);
                    _DataWriterFree(datawriter);
                    datawriter = NULL;
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            gapi_setIterFree(dataWriterIter);
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
        _EntityRelease(datawriter);
    }
    _EntityRelease(publisher);

    return result;
}

gapi_dataWriter
gapi_publisher_lookup_datawriter (
    gapi_publisher _this,
    const gapi_char *topic_name)
{
    _Publisher publisher;
    _DataWriter dataWriter = NULL;

    publisher  = gapi_publisherClaim(_this, NULL);

    if ( publisher && topic_name ) {
        gapi_setIter iter;
        
        iter = gapi_setFirst(publisher->dataWriterSet);
        while ( (dataWriter == NULL) && (gapi_setIterObject(iter) != NULL) ) {
            _DataWriter dw = _DataWriter(gapi_setIterObject(iter));
            gapi_string name;
           
            assert(dw->topic);
            name = _TopicGetName(dw->topic);
            assert(name);
            if ( strcmp(name, topic_name) == 0 ) {
                dataWriter = dw;
            } else {
                gapi_setIterNext(iter);
            }
            gapi_free(name);
        }
        gapi_setIterFree(iter);
    }

    _EntityRelease(publisher);
    
    return (gapi_dataWriter)_EntityHandle(dataWriter);
}

gapi_returnCode_t
gapi_publisher_delete_contained_entities (
    gapi_publisher _this,
    gapi_deleteEntityAction action,
    void *action_arg)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Publisher publisher;
    void *userData;
    gapi_setIter iterSet;
    
    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher != NULL ) {
        /* delete all datawriters in the datawriterSet */
        iterSet = gapi_setFirst (publisher->dataWriterSet);
        
        while ((gapi_setIterObject(iterSet)) && (result == GAPI_RETCODE_OK)) {
            _DataWriter dataWriter = _DataWriter(gapi_setIterObject(iterSet));
            _EntityClaimNotBusy(dataWriter);
            userData = _ObjectGetUserData(_Object(dataWriter));                
            _DataWriterPrepareDelete(dataWriter);
            _DataWriterFree(dataWriter);
            gapi_setIterRemove (iterSet);
            if ( action ) {
                action(userData, action_arg);
            }
        }
        gapi_setIterFree (iterSet);
        _EntityRelease(publisher);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

gapi_returnCode_t
gapi_publisher_set_qos (
    gapi_publisher _this,
    const gapi_publisherQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_result uResult;
    _Publisher publisher;
    v_publisherQos publisherQos;
    gapi_context context;
    gapi_publisherQos *existing_qos;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_QOS);
    
    publisher = gapi_publisherClaim(_this, &result);

    if ( publisher && qos ) {
        result = gapi_publisherQosIsConsistent(qos, &context);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    
    if ((result == GAPI_RETCODE_OK ) && (_Entity(publisher)->enabled)) {
        existing_qos = gapi_publisherQos__alloc();

        result = gapi_publisherQosCheckMutability(qos,
                                                  _PublisherGetQos(publisher,
                                                                   existing_qos),
                                                  &context);
        gapi_free(existing_qos);
    }
    
    if ( result == GAPI_RETCODE_OK ) {
        publisherQos = newPublisherQos();
        if (publisherQos) {
            if ( copyPublisherQosIn(qos, publisherQos) ) {
                uResult = u_entitySetQoS(_EntityUEntity(publisher),
                                         (v_qos)(publisherQos) );
                result = kernelResultToApiResult(uResult);
                u_publisherQosFree(publisherQos);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    }

    _EntityRelease(publisher);

    return result;
}

gapi_returnCode_t

gapi_publisher_get_qos (
    gapi_publisher _this,
    gapi_publisherQos *qos)
{
    _Publisher publisher;
    gapi_returnCode_t result;

    publisher = gapi_publisherClaim(_this, &result);
    if ( publisher && qos ) {
        _PublisherGetQos(publisher, qos);
    }

    _EntityRelease(publisher);
    return result ;
}

gapi_returnCode_t
gapi_publisher_set_listener (
    gapi_publisher _this,
    const struct gapi_publisherListener *a_listener,
    const gapi_statusMask mask)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Publisher publisher;

    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher != NULL ) {
        _PublisherStatus status;

        if ( a_listener ) {
            publisher->_Listener = *a_listener;
        } else {
            memset(&publisher->_Listener, 0, sizeof(publisher->_Listener));
        }

        status = _PublisherStatus(_EntityStatus(publisher));
        if ( _PublisherStatusSetListener(status, a_listener, mask) ) {
            result = GAPI_RETCODE_OK;
        }
    }
    _EntityRelease(publisher);

    return result;
}

struct gapi_publisherListener
gapi_publisher_get_listener (
    gapi_publisher _this)
{
    _Publisher publisher;
    struct gapi_publisherListener listener;
    
    publisher  = gapi_publisherClaim(_this, NULL);

    if ( publisher != NULL ) {
        listener = publisher->_Listener;
    } else {
        memset(&listener, 0, sizeof(listener));
    }
    _EntityRelease(publisher);

    return listener;
}


gapi_returnCode_t
gapi_publisher_suspend_publications (
    gapi_publisher _this)
{
    _Publisher publisher;
    gapi_returnCode_t result = GAPI_RETCODE_BAD_PARAMETER;
    u_result uResult;

    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher != NULL ) {
        if ( _Entity(publisher)->enabled) {
            uResult = u_publisherSuspend(U_PUBLISHER_GET(publisher));
            result = kernelResultToApiResult(uResult);
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(publisher);

    return result;
}

gapi_returnCode_t
gapi_publisher_resume_publications (
    gapi_publisher _this)
{
    _Publisher publisher;
    gapi_returnCode_t result = GAPI_RETCODE_BAD_PARAMETER;
    u_result uResult;

    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher != NULL ) {
        if ( _Entity(publisher)->enabled) {
            uResult = u_publisherResume(U_PUBLISHER_GET(publisher));
            result = kernelResultToApiResult(uResult);
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }
    _EntityRelease(publisher);

    return result;
}

gapi_returnCode_t
gapi_publisher_begin_coherent_changes (
    gapi_publisher _this)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_publisher_end_coherent_changes (
    gapi_publisher _this)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_publisher_wait_for_acknowledgments (
    gapi_publisher _this,
    const gapi_duration_t *max_wait)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_domainParticipant
gapi_publisher_get_participant (
    gapi_publisher _this)
{
    _Publisher publisher;
    _DomainParticipant participant = NULL;
    
    publisher  = gapi_publisherClaim(_this, NULL);

    if ( publisher != NULL ) {
        participant = _DomainEntityParticipant(_DomainEntity(publisher));
    }

    _EntityRelease(publisher);
    
    return (gapi_domainParticipant)_EntityHandle(participant);
}

gapi_returnCode_t
gapi_publisher_set_default_datawriter_qos (
    gapi_publisher _this,
    const gapi_dataWriterQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Publisher publisher;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_DEFAULT_DATAWRITER_QOS);

    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher ) {
        if ( qos ) {
            result = gapi_dataWriterQosIsConsistent(qos, &context);
            if ( result == GAPI_RETCODE_OK ) {
                gapi_dataWriterQosCopy (qos, &publisher->_defDataWriterQos);
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(publisher);

    return result;
}

gapi_returnCode_t

gapi_publisher_get_default_datawriter_qos (
    gapi_publisher _this,
    gapi_dataWriterQos *qos)
{
    _Publisher publisher;
    gapi_returnCode_t result;    
        
    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher && qos ) {
        gapi_dataWriterQosCopy (&publisher->_defDataWriterQos, qos);
    }

    _EntityRelease(publisher);
    return result;
}

gapi_returnCode_t
gapi_publisher_copy_from_topic_qos (
    gapi_publisher _this,
    gapi_dataWriterQos *a_datawriter_qos,
    const gapi_topicQos *a_topic_qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Publisher publisher;

    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher != NULL ) {
        if ( (a_topic_qos      != GAPI_TOPIC_QOS_DEFAULT)      &&
             (a_datawriter_qos != GAPI_DATAWRITER_QOS_DEFAULT) ) {
            gapi_mergeTopicQosWithDataWriterQos(a_topic_qos, a_datawriter_qos);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }
    _EntityRelease(publisher);

    return result;
}

gapi_boolean
_PublisherSetListenerInterestOnChildren (
    _Publisher publisher,
    _ListenerInterestInfo info)
{
    gapi_setIter iterSet;
    _Entity      entity;
    gapi_boolean result = TRUE;

    assert(publisher);

    iterSet = gapi_setFirst(publisher->dataWriterSet);
    while (result && gapi_setIterObject (iterSet)) {
        entity = _Entity(gapi_setIterObject(iterSet));
        result = _EntitySetListenerInterest(entity, info);
        gapi_setIterNext (iterSet);
    }
    gapi_setIterFree (iterSet);
    
    return result;
}

gapi_boolean
_PublisherContainsEntity (
    _Publisher _this,
    gapi_instanceHandle_t handle)
{
    gapi_boolean result = FALSE;
    gapi_setIter iterSet;

    assert(_this);

    _EntityClaim(_this);

    iterSet = gapi_setFirst(_this->dataWriterSet);
    while ( !result && gapi_setIterObject(iterSet) ) {
        _DataWriter writer = _DataWriter(gapi_setIterObject(iterSet));
        result = _EntityHandleEqual(_Entity(writer), handle);
        gapi_setIterNext(iterSet);                
    }
    gapi_setIterFree (iterSet);
    
    _EntityRelease(_this);

    return result;
}


