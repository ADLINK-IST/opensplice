/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_objManag.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_publisher.h"
#include "sac_dataWriter.h"
#include "sac_topicDescription.h"
#include "sac_domainParticipant.h"
#include "u_observable.h"
#include "u_participant.h"
#include "v_status.h"
#include "sac_report.h"

#define DDS_PublisherClaim(_this, publisher) \
        DDS_Object_claim(DDS_Object(_this), DDS_PUBLISHER, (_Object *)publisher)

#define DDS_PublisherClaimRead(_this, publisher) \
        DDS_Object_claim(DDS_Object(_this), DDS_PUBLISHER, (_Object *)publisher)

#define DDS_PublisherRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_PublisherCheck(_this, publisher) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_PUBLISHER, (_Object *)publisher)

#define _Publisher_get_user_entity(_this) \
        u_publisher(_Entity_get_user_entity(_Entity(_Publisher(_this))))

#define PUBLISHER_STATUS_MASK (DDS_LIVELINESS_LOST_STATUS            |\
                               DDS_OFFERED_DEADLINE_MISSED_STATUS    |\
                               DDS_OFFERED_INCOMPATIBLE_QOS_STATUS   |\
                               DDS_PUBLICATION_MATCHED_STATUS)

static DDS_ReturnCode_t
_Publisher_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Publisher pub;

    if (_this != NULL) {
        pub = _Publisher(_this);
        if (c_iterLength(pub->writerList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Publisher has %d DataWriters",
                        c_iterLength(pub->writerList));
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Publisher = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        DDS_Entity_set_listener_interest(DDS_Entity(pub), 0);
        DDS_Entity_disable_callbacks(DDS_Entity(pub));
        DDS_free(pub->defaultDataWriterQos);
        c_iterFree(pub->writerList);
        result = _Entity_deinit(_this);
    }
    return result;
}

DDS_Publisher
DDS_PublisherNew (
    DDS_DomainParticipant participant,
    const DDS_char *name,
    const DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DomainParticipant _participant = NULL;
    _Publisher _this = NULL;
    u_participant uParticipant;
    u_publisher uPublisher;
    u_publisherQos pQos;
    cmn_listenerDispatcher dispatcher;

    assert (qos != NULL && qos != DDS_PUBLISHER_QOS_DEFAULT);

    pQos = DDS_PublisherQos_copyIn(qos);
    if (pQos == NULL) {
        result = DDS_RETCODE_OUT_OF_RESOURCES;
    }
    if (result == DDS_RETCODE_OK) {
        uParticipant = u_participant(_Entity_get_user_entity(DDS_Entity(participant)));
        _participant = _DomainParticipant(participant);
        if (uParticipant != NULL && _participant != NULL) {
            uPublisher = u_publisherNew(uParticipant, name, pQos, FALSE);
            if (uPublisher != NULL) {
                result = DDS_Object_new(DDS_PUBLISHER,
                                        _Publisher_deinit,
                                        (_Object *)&_this);
                if (result == DDS_RETCODE_OK) {
                    result = DDS_Entity_init(_this, u_entity(uPublisher));
                    DDS_Object_set_domain_id(_Object(_this), DDS_Object_get_domain_id(participant));
                }
            } else {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
            }
            if (result == DDS_RETCODE_OK) {
                dispatcher = DDS_Entity_get_listenerDispatcher(_participant);
                result = DDS_Entity_set_listenerDispatcher(_this, dispatcher);
            }
            if (result == DDS_RETCODE_OK) {
                _this->defaultDataWriterQos = DDS_DataWriterQos__alloc();
                if (_this->defaultDataWriterQos != NULL) {
                    result = DDS_DataWriterQos_init(
                        _this->defaultDataWriterQos, DDS_DATAWRITER_QOS_DEFAULT);
                } else {
                    result = DDS_RETCODE_OUT_OF_RESOURCES;
                }
                _this->participant = participant;
                _this->writerList = NULL;
                _this->factoryAutoEnable = pQos->entityFactory.v.autoenable_created_entities;
            }
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Operation failed because of invalid domainParticipant (0x%x)", participant);
        }
        u_publisherQosFree(pQos);
    }
    return (DDS_Publisher)_this;
}

static DDS_ReturnCode_t
_Publisher_delete_contained_entities (
    _Publisher _this)
{
    DDS_ReturnCode_t result, endResult = DDS_RETCODE_OK;
    DDS_DataWriter writer;
    c_ulong i, nrWriters;

    /* Make sure we attempt to delete each entity only once:
     * entities that are not ready to be deleted should be
     * inserted back into the list, but should not be encountered
     * again during this iteration. So iterate by the number
     * of elements instead of by taking until the list is empty.
     */
    nrWriters = c_iterLength(_this->writerList);
    for (i = 0; i < nrWriters; i++) {
        writer = DDS_DataWriter(c_iterTakeFirst(_this->writerList));
        result = DDS__free(writer);
        if (result != DDS_RETCODE_OK) {
            c_iterInsert(_this->writerList, writer);
            endResult = result;
        }
    }
    return endResult;
}

DDS_ReturnCode_t
DDS_PublisherFree (
DDS_Publisher _this)
{
    DDS_ReturnCode_t result;
    _Publisher pub;

    result = DDS_PublisherClaim(_this, &pub);
    if (result == DDS_RETCODE_OK) {
        result = _Publisher_delete_contained_entities(pub);
        DDS_PublisherRelease(_this);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS__free(_this);
    }
    return result;
}

/*     DataWriter
 *     create_datawriter(
 *         in Topic a_topic,
 *         in DataWriterQos qos,
 *         in DataWriterListener a_listener);
 */
DDS_DataWriter
DDS_Publisher_create_datawriter (
    DDS_Publisher _this,
    const DDS_Topic topic,
    const DDS_DataWriterQos *qos,
    const struct DDS_DataWriterListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DataWriter writer = NULL;
    DDS_char *topicName;
    DDS_char *name;
    size_t length;
    _Publisher publisher;
    u_entity uEntity;
    u_writer uWriter = NULL;
    u_writerQos wQos = NULL;
    DDS_boolean factoryAutoEnable = FALSE;
    DDS_DataWriterQos writerQos;
    DDS_TopicQos topicQos;

    SAC_REPORT_STACK();

    memset(&writerQos, 0, sizeof(DDS_DataWriterQos));
    (void)DDS_DataWriterQos_init(&writerQos, DDS_DATAWRITER_QOS_DEFAULT);

    if (topic == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Topic = NULL");
    } else if (qos != DDS_DATAWRITER_QOS_DEFAULT &&
               qos != DDS_DATAWRITER_QOS_USE_TOPIC_QOS)
    {
        result = DDS_DataWriterQos_is_consistent(qos);
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_PublisherClaim(_this, &publisher);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_DATAWRITER_QOS_DEFAULT) {
            qos = publisher->defaultDataWriterQos;
        } else if (qos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS) {
            memset(&topicQos, 0, sizeof(DDS_TopicQos));
            (void)DDS_TopicQos_init(&topicQos, DDS_TOPIC_QOS_DEFAULT);
            result = DDS_TopicDescription_get_qos(topic, &topicQos);
            if (result == DDS_RETCODE_OK) {
                result = DDS_DataWriterQos_init(
                    &writerQos, publisher->defaultDataWriterQos);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_Publisher_copy_from_topic_qos(
                    publisher, &writerQos, &topicQos);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_DataWriterQos_is_consistent(&writerQos);
            }
            (void)DDS_TopicQos_deinit(&topicQos);
            qos = &writerQos;
        }
        if (result == DDS_RETCODE_OK) {
            wQos = DDS_DataWriterQos_copyIn(qos);
            if (wQos == NULL) {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Failed to copy in qos values");
            }
        }
        if (result == DDS_RETCODE_OK) {
            length = strlen("writer <>") + 1;
            topicName = DDS_Topic_get_name(topic);
            if (topicName) {
                length += strlen(topicName);
                name = os_malloc(length);
                snprintf (name, length, "writer <%s>", topicName);
                DDS_free (topicName);
            } else {
                name = os_malloc(length);
                snprintf (name, length, "writer");
            }
            /* Try to get the user layer entity of this topic.
             * If it fails then it is already deleted.
             * Don't try to create a u_writer with a u_topic NULL.
             */
            result = DDS_Entity_get_user_entity(topic, DDS_TOPIC, &uEntity);
            if (result == DDS_RETCODE_OK) {
                uWriter = u_writerNew(_Publisher_get_user_entity(publisher),
                                      name,
                                      u_topic(uEntity),
                                      wQos);
            }
            os_free(name);
            u_writerQosFree(wQos);
        }
        if (result == DDS_RETCODE_OK) {
            result = DDS_DataWriterNew(uWriter, publisher, topic, &writer);
            if (result == DDS_RETCODE_OK) {
                publisher->writerList = c_iterInsert(publisher->writerList, writer);
            }
        }
        factoryAutoEnable = publisher->factoryAutoEnable;
        DDS_PublisherRelease(_this);
    }
    if (result == DDS_RETCODE_OK) {
        cmn_listenerDispatcher listenerDispatcher;

        listenerDispatcher = DDS_Entity_get_listenerDispatcher(_this);
        result = DDS_Entity_set_listenerDispatcher(writer, listenerDispatcher);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_DataWriter_set_listener(writer, a_listener, mask);
        if (factoryAutoEnable) {
            result = DDS_Entity_enable(writer);
            if (result != DDS_RETCODE_OK) {
                DDS_ReturnCode_t destroyed;
                destroyed = DDS_Publisher_delete_datawriter (_this, writer);
                assert (destroyed == DDS_RETCODE_OK);
                OS_UNUSED_ARG(destroyed);
                writer = NULL;
            }
        }
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    (void)DDS_DataWriterQos_deinit(&writerQos);

    return writer;
}

/*     ReturnCode_t
 *     delete_datawriter(
 *         in DataWriter a_datawriter);
 */
DDS_ReturnCode_t
DDS_Publisher_delete_datawriter (
    DDS_Publisher _this,
    const DDS_DataWriter writer)
{
    DDS_ReturnCode_t result;
    DDS_DataWriter found;
    _Publisher publisher;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaim(_this, &publisher);
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(publisher->writerList, writer);
        if (found != writer) {
            /* The following call is expensive so only use it in case of exceptions. */
            if (DDS_Object_get_kind(DDS_Object(writer)) == DDS_DATAWRITER) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Writer does not belong to this Publisher");
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "DataWriter parameter 'writer' is of type %s",
                            DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(writer))));
            }
        } else {
            result = DDS__free(writer);
        }
        if (result != DDS_RETCODE_OK) {
            (void)c_iterInsert(publisher->writerList, found);
        }
        DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

struct lookupByTopicArg {
    const DDS_char *topicName;
    DDS_DataWriter writer;
};

static c_equality
lookupByTopic (
    c_voidp o,
    c_voidp arg)
{
    DDS_DataWriter writer = (DDS_DataWriter)o;
    struct lookupByTopicArg *a = (struct lookupByTopicArg *)arg;
    DDS_Topic topic;
    DDS_char *topicName;
    c_equality equality = C_NE;

    topic = DDS_DataWriter_get_topic(writer);
    topicName = DDS_TopicDescription_get_name(topic);
    if (strcmp(topicName, a->topicName) == 0) {
        a->writer = writer;
        equality = C_EQ;
    }
    DDS_free(topicName);

    return equality;
}

/*     DataWriter
 *     lookup_datawriter(
 *         in string topic_name);
 */
DDS_DataWriter
DDS_Publisher_lookup_datawriter (
    DDS_Publisher _this,
    const DDS_char *topic_name)
{
    DDS_ReturnCode_t result;
    DDS_DataWriter found = NULL;
    _Publisher publisher;
    struct lookupByTopicArg arg;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaimRead(_this, &publisher);
    if (result == DDS_RETCODE_OK) {
        arg.topicName = topic_name;
        arg.writer = NULL;
        c_iterResolve(publisher->writerList, lookupByTopic, &arg);
        found = arg.writer;
        DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return found;
}

struct check_handle_arg {
    DDS_InstanceHandle_t handle;
    DDS_boolean result;
};

static c_bool
writer_check_handle(
    void *object,
    struct check_handle_arg *arg)
{
    assert(object);
    assert(arg);

    if (!arg->result) {
        arg->result = DDS_Entity_check_handle(DDS_Entity(object), arg->handle);
    }
    return !arg->result;
}

DDS_boolean
DDS_Publisher_contains_entity (
    DDS_Publisher _this,
    DDS_InstanceHandle_t  a_handle)
{
    DDS_ReturnCode_t result;
    _Publisher pub;
    struct check_handle_arg arg;

    SAC_REPORT_STACK();

    arg.handle = a_handle;
    arg.result = FALSE;
    result = DDS_PublisherClaimRead(_this, &pub);
    if (result == DDS_RETCODE_OK) {
        c_iterWalkUntil(pub->writerList, (c_iterAction)writer_check_handle, &arg);
        result = DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return arg.result;
}

/*     ReturnCode_t
 *     delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_Publisher_delete_contained_entities (
    DDS_Publisher _this)
{
    DDS_ReturnCode_t result;
    _Publisher publisher;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaim(_this, &publisher);
    if (result == DDS_RETCODE_OK) {
        result = _Publisher_delete_contained_entities(publisher);
        DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_qos(
 *         inout PublisherQos qos);
 */
DDS_ReturnCode_t
DDS_Publisher_get_qos (
    DDS_Publisher _this,
    DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t result;
    _Publisher pub;
    u_publisherQos uQos;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_PublisherCheck(_this, &pub);
    if (result == DDS_RETCODE_OK) {
        if (qos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "PublisherQos = NULL");
        } else if (qos == DDS_PUBLISHER_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'PUBLISHER_QOS_DEFAULT' is read-only.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_publisherGetQos(_Publisher_get_user_entity(pub), &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS_PublisherQos_copyOut(uQos, qos);
            u_publisherQosFree(uQos);
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_qos(
 *         in PublisherQos qos);
 */
DDS_ReturnCode_t
DDS_Publisher_set_qos (
    DDS_Publisher _this,
    const DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_PublisherQos publisherQos;
    _Publisher p;
    u_publisherQos pQos = NULL;
    u_result uResult;
    u_publisher uPublisher;

    SAC_REPORT_STACK();

    memset(&publisherQos, 0, sizeof(DDS_PublisherQos));
    (void)DDS_PublisherQos_init(&publisherQos, DDS_PUBLISHER_QOS_DEFAULT);

    result = DDS_PublisherQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_PublisherClaim(_this, &p);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_PUBLISHER_QOS_DEFAULT) {
            result = DDS_DomainParticipant_get_default_publisher_qos(
                p->participant, &publisherQos);
            qos = &publisherQos;
        }
        if (result == DDS_RETCODE_OK) {
            pQos = DDS_PublisherQos_copyIn(qos);
            if (pQos == NULL) {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Failed to copy DDS_PublisherQos");
            }
        }
        if (result == DDS_RETCODE_OK) {
            uPublisher = _Publisher_get_user_entity(p);
            uResult = u_publisherSetQos(uPublisher, pQos);
            result = DDS_ReturnCode_get(uResult);
            if (result == DDS_RETCODE_OK) {
                p->factoryAutoEnable = pQos->entityFactory.v.autoenable_created_entities;
            }
            u_publisherQosFree(pQos);
        }
        DDS_PublisherRelease(_this);
    }

    (void)DDS_PublisherQos_deinit(&publisherQos);

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_listener(
 *         in PublisherListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_Publisher_set_listener (
    DDS_Publisher _this,
    const struct DDS_PublisherListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;
    _Publisher p;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        if (a_listener != NULL) {
            p->listener = *a_listener;
            result = DDS_Entity_set_listener_interest(DDS_Entity(p), mask);
        } else {
            memset(&p->listener, 0, sizeof(struct DDS_PublisherListener));
            result = DDS_Entity_set_listener_interest(DDS_Entity(p), mask);
        }
        DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     PublisherListener
 *     get_listener();
 */
struct DDS_PublisherListener
DDS_Publisher_get_listener (
    DDS_Publisher _this)
{
    DDS_ReturnCode_t result;
    _Publisher p;
    struct DDS_PublisherListener listener;

    SAC_REPORT_STACK();

    result = DDS_PublisherCheck(_this, &p);
    if (result == DDS_RETCODE_OK) {
        listener = p->listener;
    } else {
        memset(&listener, 0, sizeof(struct DDS_PublisherListener));
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return listener;
}

/*     ReturnCode_t
 *     set_listener_mask(
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_Publisher_set_listener_mask (
    _Publisher _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;

    result = DDS_Entity_set_listener_interest(DDS_Entity(_this), mask);

    return result;
}



/*     ReturnCode_t
 *     suspend_publications();
 */
DDS_ReturnCode_t
DDS_Publisher_suspend_publications (
    DDS_Publisher _this)
{
    DDS_ReturnCode_t result;
    _Publisher p;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        uResult = u_publisherSuspend(_Publisher_get_user_entity(p));
        result = DDS_ReturnCode_get(uResult);
        DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     resume_publications();
 */
DDS_ReturnCode_t
DDS_Publisher_resume_publications (
    DDS_Publisher _this)
{
    DDS_ReturnCode_t result;
    _Publisher p;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        uResult = u_publisherResume(_Publisher_get_user_entity(p));
        result = DDS_ReturnCode_get(uResult);
        DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     begin_coherent_changes();
 */
DDS_ReturnCode_t
DDS_Publisher_begin_coherent_changes (
    DDS_Publisher _this)
{
    DDS_ReturnCode_t result;
    _Publisher p;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        uResult = u_publisherCoherentBegin(_Publisher_get_user_entity(p));
        result = DDS_ReturnCode_get(uResult);
        DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     end_coherent_changes();
 */
DDS_ReturnCode_t
DDS_Publisher_end_coherent_changes (
    DDS_Publisher _this)
{
    DDS_ReturnCode_t result;
    _Publisher p;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        uResult = u_publisherCoherentEnd(_Publisher_get_user_entity(p));
        result = DDS_ReturnCode_get(uResult);
        DDS_PublisherRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/* ReturnCode_t
 *   wait_for_acknowledgments(
 *      in Duration_t max_wait);
 */
DDS_ReturnCode_t
DDS_Publisher_wait_for_acknowledgments (
    DDS_Publisher _this,
    const DDS_Duration_t *max_wait)
{
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(max_wait);

    return DDS_RETCODE_UNSUPPORTED;
}


/*     DomainParticipant
 *     get_participant();
 */
DDS_DomainParticipant
DDS_Publisher_get_participant (
    DDS_Publisher _this)
{
    DDS_ReturnCode_t result;
    _Publisher p;
    DDS_DomainParticipant participant = NULL;

    SAC_REPORT_STACK();

    result = DDS_PublisherCheck(_this, &p);
    if (result == DDS_RETCODE_OK) {
        participant = p->participant;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return participant;
}

/*     ReturnCode_t
 *     set_default_datawriter_qos(
 *         in DataWriterQos qos);
 */
DDS_ReturnCode_t
DDS_Publisher_set_default_datawriter_qos (
    DDS_Publisher _this,
    const DDS_DataWriterQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Publisher p;
    DDS_DataWriterQos *writerQos = NULL;

    SAC_REPORT_STACK();

    if (qos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataWriterQos = DDS_DATAWRITER_QOS_USE_TOPIC_QOS");
    } else {
        result = DDS_DataWriterQos_is_consistent(qos);
    }
    if (result == DDS_RETCODE_OK) {
        writerQos = DDS_DataWriterQos__alloc();
        if (writerQos != NULL) {
            result = DDS_DataWriterQos_init(writerQos, qos);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to copy DDS_DataWriterQos");
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_PublisherClaim(_this, &p);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_free(p->defaultDataWriterQos);
        p->defaultDataWriterQos = writerQos;
        DDS_PublisherRelease(_this);
    } else {
        DDS_free(writerQos);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_default_datawriter_qos(
 *         inout DataWriterQos qos);
 */
DDS_ReturnCode_t
DDS_Publisher_get_default_datawriter_qos (
    DDS_Publisher _this,
    DDS_DataWriterQos *qos)
{
    DDS_ReturnCode_t result;
    _Publisher p;

    SAC_REPORT_STACK();

    result = DDS_PublisherClaimRead(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DataWriterQos_init(qos, p->defaultDataWriterQos);
        DDS_PublisherRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     copy_from_topic_qos(
 *         inout DataWriterQos a_datawriter_qos,
 *         in TopicQos a_topic_qos);
 */
DDS_ReturnCode_t
DDS_Publisher_copy_from_topic_qos (
    DDS_Publisher _this,
    DDS_DataWriterQos *wQos,
    const DDS_TopicQos *tQos)
{
    DDS_ReturnCode_t result;
    DDS_TopicQos *topicQos = NULL;
    _Publisher p;

    SAC_REPORT_STACK();

    result = DDS_PublisherCheck(_this, &p);
    if (result == DDS_RETCODE_OK) {
        if (wQos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "a_datawriter_qos 'null' is invalid.");
        } else if (wQos == DDS_DATAWRITER_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAWRITER_QOS_DEFAULT' is read-only.");
        } else if (wQos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is read-only.");
        } else if (tQos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "a_topic_qos 'null' is invalid.");
        } else if (tQos == DDS_TOPIC_QOS_DEFAULT) {
            topicQos = DDS_TopicQos__alloc();
            if (topicQos == NULL) {
                result = DDS_RETCODE_ERROR;
                SAC_REPORT(result, "Could not copy DataWriterQos.");
            } else {
                result = DDS_DomainParticipant_get_default_topic_qos(
                    p->participant, topicQos);
                if (result == DDS_RETCODE_OK) {
                    tQos = topicQos;
                }
            }
        }

        if (result == DDS_RETCODE_OK) {
            wQos->durability         = tQos->durability;
            wQos->deadline           = tQos->deadline;
            wQos->latency_budget     = tQos->latency_budget;
            wQos->liveliness         = tQos->liveliness;
            wQos->reliability        = tQos->reliability;
            wQos->destination_order  = tQos->destination_order;
            wQos->history            = tQos->history;
            wQos->resource_limits    = tQos->resource_limits;
            wQos->ownership          = tQos->ownership;
            wQos->transport_priority = tQos->transport_priority;
            wQos->lifespan           = tQos->lifespan;
        }
    }

    if (topicQos != NULL) {
        (void)DDS__free(topicQos);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_Publisher_notify_listener(
    DDS_Publisher _this,
    v_listenerEvent event)
{
    DDS_ReturnCode_t result;
    u_eventMask triggerMask;
    DDS_Entity entity;
    struct DDS_PublisherListener cb;

    cb = _Publisher(_this)->listener;
    triggerMask = event->kind;
    result = DDS_RETCODE_OK;

    entity = u_observableGetUserData(u_observable(event->source));

    if ((triggerMask & V_EVENT_LIVELINESS_LOST) &&
            (cb.on_liveliness_lost != NULL))
    {
        DDS_LivelinessLostStatus status;
        DDS_LivelinessLostStatus_init(&status, &((v_writerStatus)event->eventData)->livelinessLost);
        cb.on_liveliness_lost(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED) &&
            (cb.on_offered_deadline_missed != NULL))
    {
        DDS_OfferedDeadlineMissedStatus status;
        DDS_OfferedDeadlineMissedStatus_init(&status, &((v_writerStatus)event->eventData)->deadlineMissed);
        cb.on_offered_deadline_missed(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) &&
            (cb.on_offered_incompatible_qos != NULL))
    {
        DDS_OfferedIncompatibleQosStatus status;
        DDS_OfferedIncompatibleQosStatus_init(&status, &((v_writerStatus)event->eventData)->incompatibleQos);
        cb.on_offered_incompatible_qos(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_PUBLICATION_MATCHED) &&
            (cb.on_publication_matched != NULL))
    {
        DDS_PublicationMatchedStatus status;
        DDS_PublicationMatchedStatus_init(&status, &((v_writerStatus)event->eventData)->publicationMatch);
        cb.on_publication_matched(cb.listener_data, entity, &status);
    }

    return result;
}
