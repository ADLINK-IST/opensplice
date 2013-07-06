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
#include "gapi_publisher.h"
#include "gapi_domainParticipant.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_dataWriter.h"
#include "gapi_topicDescription.h"
#include "gapi_topic.h"
#include "gapi_entity.h"
#include "gapi_kernel.h"
#include "gapi_structured.h"
#include "gapi_typeSupport.h"
#include "gapi_qos.h"
#include "gapi_objManag.h"
#include "gapi_error.h"

#include "os_heap.h"
#include "u_user.h"

#define U_PUBLISHER_SET(p,e) \
        _EntitySetUserEntity(_Entity(p), u_entity(e))


C_STRUCT(_Publisher) {
    C_EXTENDS(_Entity);
    gapi_dataWriterQos             _defDataWriterQos;
    struct gapi_publisherListener  _Listener;
};

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
    v_publisherQos publisherQos;
    _Publisher newPublisher;

    assert(uParticipant);
    assert(qos);
    assert(participant);

    newPublisher = _PublisherAlloc();

    if ( newPublisher ) {
        _EntityInit (_Entity(newPublisher),
                           _Entity(participant));
        gapi_dataWriterQosCopy (&gapi_dataWriterQosDefault,
                                &newPublisher->_defDataWriterQos);
        if ( a_listener ) {
            newPublisher->_Listener = *a_listener;
        }
    }

    if ( newPublisher ) {
        publisherQos = u_publisherQosNew(NULL);
        if ( publisherQos ) {
            if ( !copyPublisherQosIn(qos, publisherQos)) {
                _EntityDispose(_Entity(newPublisher));
                newPublisher = NULL;
            }
        } else {
            _EntityDispose(_Entity(newPublisher));
            newPublisher = NULL;
        }
    }

    if ( newPublisher  ) {
        u_publisher uPublisher;

        uPublisher = u_publisherNew (uParticipant,
                                     "publisher",
                                     publisherQos,
                                     FALSE);
        u_publisherQosFree(publisherQos);
        if ( uPublisher ) {
            U_PUBLISHER_SET(newPublisher, uPublisher);
        } else {
            _EntityDispose(_Entity(newPublisher));
            newPublisher = NULL;
        }
    }

    if ( newPublisher ) {
        _Status status;

        status = _StatusNew(_Entity(newPublisher),
                            STATUS_KIND_PUBLISHER,
                            (struct gapi_listener *)a_listener, mask);
        if (status) {
            _EntityStatus(newPublisher) = status;
            if ( qos->partition.name._length == 0 ) {
                /*
                 * behaviour of the kernel in case of an empty sequence
                 * is that it is related to none of the partitions,
                 * while DCPS expects it to be conected to all partitions.
                 * Therefore this has to be done seperately.
                 */
                u_publisherPublish (U_PUBLISHER_GET(newPublisher), "");
            }
        } else {
            u_publisherFree(U_PUBLISHER_GET(newPublisher));
            _EntityDispose(_Entity(newPublisher));
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
    _Status status;
    u_publisher p;

    assert(_this);

    status = _EntityStatus(_this);
    _StatusSetListener(status, NULL, 0);

    _EntityClaim(status);
    _StatusDeinit(status);

    gapi_dataWriterQos_free(&_this->_defDataWriterQos);

    p = U_PUBLISHER_GET(_this);
    _EntityDispose (_Entity(_this));
    if (u_publisherFree(p) != U_RESULT_OK) {
        result = GAPI_RETCODE_ERROR;
    }

    return result;
}

c_long
_PublisherWriterCount (
    _Publisher _this)
{
    return u_publisherWriterCount(U_PUBLISHER_GET(_this));
}

u_publisher
_PublisherUpublisher (
    _Publisher _this)
{
    assert(_this);
    return U_PUBLISHER_GET(_this);
}

u_result
_PublisherGetQos (
    _Publisher _this,
    gapi_publisherQos *qos)
{
    v_publisherQos publisherQos;
    u_publisher uPublisher;
    u_result result;

    assert(_this);

    uPublisher = U_PUBLISHER_GET(_this);

    result = u_entityQoS(u_entity(uPublisher), (v_qos*)&publisherQos);
    if ( result == U_RESULT_OK ) {
        copyPublisherQosOut(publisherQos,  qos);
        u_publisherQosFree(publisherQos);
    }

    return result;
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

        rc = gapi_dataWriterQosIsConsistent(writerQos, &context);

        if ( rc == GAPI_RETCODE_OK ) {
            gapi_char *typeName;
            gapi_char *topicName;
            _DomainParticipant participant;
            _TypeSupport typeSupport = NULL;

            /* find topic with the participant for consistency */
            typeName  = _TopicGetTypeName(topic);
            topicName = _TopicGetName(topic);
            participant = _EntityParticipant(_Entity(publisher));

            /* find type support for the data type to find data writer create function */
            typeSupport = _DomainParticipantFindType(participant, typeName);
            if(typeSupport)
            {
                /* if create function is NULL, take default from data writer */
                datawriter = _DataWriterNew(topic,
                                            typeSupport,
                                            writerQos,
                                            a_listener,
                                            mask,
                                            publisher);
                if ( datawriter ) {
                    _ENTITY_REGISTER_OBJECT(_Entity(publisher),
                                            (_Object)datawriter);
                }
            }else{
                OS_REPORT_1(OS_WARNING,
                            "gapi_publisher_create_datawriter", 0,
                            "TypeSupport %s not found !",
                            typeName);
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
    _DataWriter datawriter;
    c_bool contains;

    publisher = gapi_publisherClaim(_this, &result);

    if ( publisher ) {
        datawriter = gapi_dataWriterClaimNB(a_datawriter, NULL);
        if ( datawriter ) {
            contains = u_publisherContainsWriter(U_PUBLISHER_GET(publisher),
                                                 U_WRITER_GET(datawriter));
            if (contains) {
                result = _DataWriterFree(datawriter);
                if ( result != GAPI_RETCODE_OK ) {
                    _EntityRelease(datawriter);
                }
            } else {
                _EntityRelease(datawriter);
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
        _EntityRelease(publisher);
    }
    return result;
}

gapi_dataWriter
gapi_publisher_lookup_datawriter (
    gapi_publisher _this,
    const gapi_char *topic_name)
{
    _Publisher publisher;
    u_writer found;
    gapi_dataWriter handle = NULL;
    c_iter iter;

    if (topic_name) {
        publisher  = gapi_publisherClaim(_this, NULL);
        if (publisher) {
            iter = u_publisherLookupWriters(U_PUBLISHER_GET(publisher), topic_name);
            if (iter) {
                found = c_iterTakeFirst(iter);
                if (found) {
                    handle = u_entityGetUserData(u_entity(found));
                }
                c_iterFree(iter);
            }
            _EntityRelease(publisher);
        }
    }

    return handle;
}

gapi_returnCode_t
gapi_publisher_delete_contained_entities (
    gapi_publisher _this)
{
    _Publisher publisher;
    _DataWriter dataWriter;
    gapi_dataWriter handle;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    c_iter writers;
    u_writer w;
    void *userData;

    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher != NULL ) {
        /* delete all datawriters in the datawriterSet */
        writers = u_publisherLookupWriters(U_PUBLISHER_GET(publisher),NULL);

        w = c_iterTakeFirst(writers);
        while (w) {
            handle = u_entityGetUserData(u_entity(w));
            dataWriter = gapi_dataWriterClaimNB(handle,&result);
            if (dataWriter) {
                userData = _ObjectGetUserData(_Object(dataWriter));
                _DataWriterFree(dataWriter);
            }
            w = c_iterTakeFirst(writers);
        }
        c_iterFree(writers);
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

    if ((result == GAPI_RETCODE_OK ) && (_EntityEnabled(publisher))) {
        existing_qos = gapi_publisherQos__alloc();
        uResult = _PublisherGetQos(publisher, existing_qos);
        result = kernelResultToApiResult(uResult);
        if(result == GAPI_RETCODE_OK)
        {
            result = gapi_publisherQosCheckMutability(
                     qos,
                     existing_qos,
                     &context);
        }
        gapi_free(existing_qos);
    }

    if ( result == GAPI_RETCODE_OK ) {
        publisherQos = u_publisherQosNew(NULL);
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
    u_result uResult;

    publisher = gapi_publisherClaim(_this, &result);
    if ( publisher && qos ) {
        uResult = _PublisherGetQos(publisher, qos);
        result = kernelResultToApiResult(uResult);
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
        _Status status;

        if ( a_listener ) {
            publisher->_Listener = *a_listener;
        } else {
            memset(&publisher->_Listener, 0, sizeof(publisher->_Listener));
        }

        status = _EntityStatus(publisher);
        if ( _StatusSetListener(status,
                                (struct gapi_listener *)a_listener,
                                mask) )
        {
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
        if ( _EntityEnabled(publisher)) {
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
        if ( _EntityEnabled(publisher)) {
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
    _Publisher publisher;
    gapi_returnCode_t result = GAPI_RETCODE_BAD_PARAMETER;
    u_result uResult;

    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher != NULL ) {
        if ( _EntityEnabled(publisher)) {
            uResult = u_publisherCoherentBegin(U_PUBLISHER_GET(publisher));
            result = kernelResultToApiResult(uResult);
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }
    _EntityRelease(publisher);

    return result;
}

gapi_returnCode_t
gapi_publisher_end_coherent_changes (
    gapi_publisher _this)
{
    _Publisher publisher;
    gapi_returnCode_t result = GAPI_RETCODE_BAD_PARAMETER;
    u_result uResult;

    publisher  = gapi_publisherClaim(_this, &result);

    if ( publisher != NULL ) {
        if ( _EntityEnabled(publisher)) {
            uResult = u_publisherCoherentEnd(U_PUBLISHER_GET(publisher));
            result = kernelResultToApiResult(uResult);
        } else {
            result = GAPI_RETCODE_NOT_ENABLED;
        }
    }
    _EntityRelease(publisher);

    return result;
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
        participant = _EntityParticipant(_Entity(publisher));
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

    publisher = gapi_publisherClaim(_this, &result);
    if (result == GAPI_RETCODE_OK) {
        if (qos == GAPI_DATAWRITER_QOS_DEFAULT) {
            qos = &gapi_dataWriterQosDefault;
        }
        result = gapi_dataWriterQosIsConsistent(qos, &context);
        if (result == GAPI_RETCODE_OK) {
            gapi_dataWriterQosCopy (qos, &publisher->_defDataWriterQos);
        }
        _EntityRelease(publisher);
    }

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
    if (result == GAPI_RETCODE_OK) {
        if (qos) {
            gapi_dataWriterQosCopy (&publisher->_defDataWriterQos, qos);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
        _EntityRelease(publisher);
    }

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

struct check_handle_arg {
    gapi_instanceHandle_t handle;
    gapi_boolean result;
};

static c_bool
check_handle(
    u_writer writer,
    struct check_handle_arg *arg)
{
    gapi_object handle;
    _Entity e;

    assert(writer);
    assert(arg);

    if (!arg->result) {
        handle = u_entityGetUserData(u_entity(writer));
        e = _Entity(gapi_objectPeekUnchecked(handle));
        if (e) {
            arg->result = _EntityHandleEqual(e,arg->handle);
        }
    }
    return !arg->result;
}

gapi_boolean
_PublisherContainsEntity (
    _Publisher _this,
    gapi_instanceHandle_t handle)
{
    struct check_handle_arg arg;

    assert(_this);

    _EntityClaim(_this);

    arg.handle = handle;
    arg.result = FALSE;

    u_publisherWalkWriters(U_PUBLISHER_GET(_this),
                           (u_writerAction)check_handle,
                           (c_voidp)&arg);

    _EntityRelease(_this);

    return arg.result;
}

static gapi_boolean
gapi_requestedIncompatibleQosStatus_free (
    void *object)
{
    gapi_requestedIncompatibleQosStatus *o;

    o = (gapi_requestedIncompatibleQosStatus *) object;

    gapi_free(o->policies._buffer);
    return TRUE;
}


gapi_requestedIncompatibleQosStatus *
gapi_requestedIncompatibleQosStatus_alloc (
    void)
{
    gapi_requestedIncompatibleQosStatus *newStatus;

    newStatus = (gapi_requestedIncompatibleQosStatus *)
                    gapi__malloc(gapi_requestedIncompatibleQosStatus_free, 0, sizeof(gapi_requestedIncompatibleQosStatus));

    if ( newStatus ) {
        newStatus->policies._buffer  = gapi_qosPolicyCountSeq_allocbuf(MAX_POLICY_COUNT_ID);
        newStatus->policies._length  = 0;
        newStatus->policies._maximum = MAX_POLICY_COUNT_ID;
        newStatus->policies._release = TRUE;

        if ( newStatus->policies._buffer == NULL ) {
            gapi_free(newStatus);
            newStatus = NULL;
        }
    }

    return newStatus;
}

static gapi_boolean
gapi_offeredIncompatibleQosStatus_free (
    void *object)
{
    gapi_offeredIncompatibleQosStatus *o;

    o = (gapi_offeredIncompatibleQosStatus *) object;

    gapi_free(o->policies._buffer);
    return TRUE;
}

gapi_offeredIncompatibleQosStatus *
gapi_offeredIncompatibleQosStatus_alloc (
    void)
{
    gapi_offeredIncompatibleQosStatus *newStatus;

    newStatus = (gapi_offeredIncompatibleQosStatus *)
                    gapi__malloc(gapi_offeredIncompatibleQosStatus_free, 0, sizeof(gapi_offeredIncompatibleQosStatus));

    if ( newStatus ) {
        newStatus->policies._buffer  = gapi_qosPolicyCountSeq_allocbuf(MAX_POLICY_COUNT_ID);
        newStatus->policies._length  = 0;
        newStatus->policies._maximum = MAX_POLICY_COUNT_ID;
        newStatus->policies._release = TRUE;

        if ( newStatus->policies._buffer == NULL ) {
            gapi_free(newStatus);
            newStatus = NULL;
        }
    }

    return newStatus;
}

