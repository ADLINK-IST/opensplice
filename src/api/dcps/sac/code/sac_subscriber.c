/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "sac_subscriber.h"
#include "sac_contentFilteredTopic.h"
#include "sac_dataReader.h"
#include "sac_domainParticipant.h"
#include "u_observable.h"
#include "u_subscriber.h"
#include "v_status.h"
#include "sac_report.h"

#define DDS_SubscriberClaim(_this, subscriber) \
        DDS_Object_claim(DDS_Object(_this), DDS_SUBSCRIBER, (_Object *)subscriber)

#define DDS_SubscriberClaimRead(_this, subscriber) \
        DDS_Object_claim(DDS_Object(_this), DDS_SUBSCRIBER, (_Object *)subscriber)

#define DDS_SubscriberRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_SubscriberCheck(_this, subscriber) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_SUBSCRIBER, (_Object *)subscriber)

#define _Subscriber_get_user_entity(_this) \
        u_subscriber(_Entity_get_user_entity(_Entity(_Subscriber(_this))))

#define SUBSCRIBER_STATUS_MASK (DDS_DATA_ON_READERS_STATUS            |\
                                DDS_SAMPLE_REJECTED_STATUS            |\
                                DDS_LIVELINESS_CHANGED_STATUS         |\
                                DDS_REQUESTED_DEADLINE_MISSED_STATUS  |\
                                DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS |\
                                DDS_DATA_AVAILABLE_STATUS             |\
                                DDS_SAMPLE_LOST_STATUS                |\
                                DDS_SUBSCRIPTION_MATCHED_STATUS)

static DDS_ReturnCode_t
_Subscriber_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Subscriber sub;

    sub = _Subscriber(_this);
    if (sub != NULL) {
        if (c_iterLength(sub->readerList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Subscribers has %d DataReaders",
                        c_iterLength(sub->readerList));
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Subscriber = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        DDS_Entity_set_listener_interest(DDS_Entity(sub), 0);
        DDS_Entity_disable_callbacks(DDS_Entity(sub));
        DDS_free(sub->defaultDataReaderQos);
        c_iterFree(sub->readerList);
        result = _Entity_deinit(_this);
    }
    return result;
}

DDS_Subscriber
DDS_SubscriberNew (
    DDS_DomainParticipant participant,
    const DDS_char *name,
    const DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DomainParticipant _participant = NULL;
    _Subscriber _this = NULL;
    u_participant uParticipant;
    u_subscriber uSubscriber;
    u_subscriberQos sQos;
    cmn_listenerDispatcher dispatcher;

    assert (qos != NULL && qos != DDS_SUBSCRIBER_QOS_DEFAULT);

    sQos = DDS_SubscriberQos_copyIn(qos);
    if (sQos == NULL) {
        result = DDS_RETCODE_ERROR;
    }
    if (result == DDS_RETCODE_OK) {
        uParticipant = u_participant(_Entity_get_user_entity(participant));
        _participant = _DomainParticipant(participant);
        if (uParticipant != NULL && _participant != NULL) {
            uSubscriber = u_subscriberNew(uParticipant, name, sQos, FALSE);
            if (uSubscriber != NULL) {
                result = DDS_Object_new(DDS_SUBSCRIBER, _Subscriber_deinit, (_Object *)&_this);
                if (result == DDS_RETCODE_OK) {
                    result = DDS_Entity_init(_this, u_entity(uSubscriber));
                    DDS_Object_set_domain_id(_Object(_this), DDS_Object_get_domain_id(participant));
                }
                if (result == DDS_RETCODE_OK) {
                    dispatcher = DDS_Entity_get_listenerDispatcher(_participant);
                    result = DDS_Entity_set_listenerDispatcher(_this, dispatcher);
                }
                if (result == DDS_RETCODE_OK) {
                    _this->defaultDataReaderQos = DDS_DataReaderQos__alloc();
                    if (_this->defaultDataReaderQos != NULL) {
                        result = DDS_DataReaderQos_init(
                            _this->defaultDataReaderQos, DDS_DATAREADER_QOS_DEFAULT);
                    } else {
                        result = DDS_RETCODE_OUT_OF_RESOURCES;
                    }
                    _this->readerList = NULL;
                    _this->participant = participant;
                    _this->factoryAutoEnable = sQos->entityFactory.v.autoenable_created_entities;
                }
            } else {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
        }
        u_subscriberQosFree(sQos);
    }
    return (DDS_Subscriber)_this;
}

static DDS_ReturnCode_t
_Subscriber_delete_contained_entities (
    _Subscriber _this)
{
    DDS_DataReader reader;
    DDS_ReturnCode_t result, endResult = DDS_RETCODE_OK;
    c_ulong i, nrReaders;

    /* Make sure we attempt to delete each entity only once:
     * entities that are not ready to be deleted should be
     * inserted back into the list, but should not be encountered
     * again during this iteration. So iterate by the number
     * of elements instead of by taking until the list is empty.
     */
    nrReaders = c_iterLength(_this->readerList);
    for (i = 0; i < nrReaders; i++) {
        reader = DDS_DataReader(c_iterTakeFirst(_this->readerList));
        result = DDS_DataReaderFree(reader);
        if (result != DDS_RETCODE_OK) {
            c_iterInsert(_this->readerList, reader);
            endResult = result;
        }
    }
    return endResult;
}

DDS_ReturnCode_t
DDS_SubscriberFree (
    DDS_Subscriber _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Subscriber sub;

    if (_this) {
        result = DDS_SubscriberClaim(_this, &sub);
        if (result == DDS_RETCODE_OK) {
            result = _Subscriber_delete_contained_entities(sub);
            DDS_SubscriberRelease(_this);
        }
        if (result == DDS_RETCODE_OK) {
            result = DDS__free(_this);
        }
    }
    return result;
}

/*     DataReader
 *     create_datareader(
 *         in TopicDescription a_topic,
 *         in DataReaderQos qos,
 *         in DataReaderListener a_listener);
 */
DDS_DataReader
DDS_Subscriber_create_datareader (
    DDS_Subscriber _this,
    const DDS_TopicDescription a_topic,
    const DDS_DataReaderQos *qos,
    const struct DDS_DataReaderListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DataReader reader = NULL;
    DDS_char *topicName;
    DDS_char *name;
    _Subscriber sub;
    size_t length;
    DDS_boolean factoryAutoEnable = FALSE;
    DDS_DataReaderQos readerQos;

    SAC_REPORT_STACK();

    memset(&readerQos, 0, sizeof(DDS_DataReaderQos));
    (void)DDS_DataReaderQos_init(&readerQos, DDS_DATAREADER_QOS_DEFAULT);

    if (a_topic == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Topic = NULL");
    } else {
        result = DDS_DataReaderQos_is_consistent(qos);
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_SubscriberClaim(_this, &sub);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_DATAREADER_QOS_DEFAULT) {
            qos = sub->defaultDataReaderQos;
        } else if (qos == DDS_DATAREADER_QOS_USE_TOPIC_QOS) {
            result = DDS_DataReaderQos_init(
                &readerQos, sub->defaultDataReaderQos);
            if (result == DDS_RETCODE_OK) {
                result = DDS_Subscriber_copy_from_topicdescription(
                    _this, &readerQos, a_topic);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_DataReaderQos_is_consistent(&readerQos);
            }
            qos = &readerQos;
        }

        if (result == DDS_RETCODE_OK) {
            length = strlen("reader <>") + 1;
            topicName = DDS_Topic_get_name(a_topic);
            if (topicName) {
                length += strlen(topicName);
                name = os_malloc(length);
                snprintf (name, length, "reader <%s>", topicName);
                DDS_free(topicName);
            } else {
                name = os_malloc (length);
                os_strncpy (name, "reader", length - 1);
            }

            reader = DDS_DataReaderNew (_this, name, qos, DDS_TopicDescription(a_topic));
            os_free(name);
            if ( reader ) {
                sub->readerList = c_iterInsert (sub->readerList, reader);
            } else {
                result = DDS_RETCODE_ERROR;
            }
            factoryAutoEnable = sub->factoryAutoEnable;
        }
        DDS_SubscriberRelease(_this);
    }
    if (result == DDS_RETCODE_OK) {
        cmn_listenerDispatcher listenerDispatcher;

        listenerDispatcher = DDS_Entity_get_listenerDispatcher(_this);
        result = DDS_Entity_set_listenerDispatcher(reader, listenerDispatcher);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_DataReader_set_listener(reader, a_listener, mask);
        if (factoryAutoEnable) {
            result = DDS_Entity_enable(reader);
            if (result != DDS_RETCODE_OK) {
                DDS_ReturnCode_t destroyed;
                destroyed = DDS_Subscriber_delete_datareader (_this, reader);
                assert (destroyed == DDS_RETCODE_OK);
                OS_UNUSED_ARG(destroyed);
                reader = NULL;
            }
        }
    }

    (void)DDS_DataReaderQos_deinit(&readerQos);

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return reader;
}

/*     ReturnCode_t
 *     delete_datareader(
 *         in DataReader a_datareader);
 */
DDS_ReturnCode_t
DDS_Subscriber_delete_datareader (
    DDS_Subscriber _this,
    const DDS_DataReader reader)
{
    DDS_ReturnCode_t result;
    DDS_DataReader found;
    _Subscriber sub;

    SAC_REPORT_STACK();

    if (reader != NULL) {
        result = DDS_SubscriberClaim(_this, &sub);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReader = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(sub->readerList, reader);
        if (found != reader) {
            /* The following call is expensive so only use it in case of exceptions. */
            if (DDS_Object_get_kind(DDS_Object(reader)) == DDS_DATAREADER) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "DataReader is not from Subscriber");
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "DataReader parameter 'reader' is of type %s",
                            DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(reader))));
            }
        } else {
            result = DDS__free(reader);
        }
        if (result != DDS_RETCODE_OK) {
           (void)c_iterInsert(sub->readerList, found);
        }
        DDS_SubscriberRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

struct check_handle_arg {
    DDS_InstanceHandle_t handle;
    DDS_boolean result;
};

static c_bool
reader_check_handle(
    void *object,
    struct check_handle_arg *arg)
{
    assert(object);
    assert(arg);

    if (!arg->result) {
        arg->result = DDS_Entity_check_handle(DDS_Entity(object), arg->handle);
        if ( !arg->result ) {
            arg->result = DDS_DataReader_contains_entity(DDS_DataReader(object), arg->handle);
        }
    }
    return !arg->result;
}

DDS_boolean
DDS_Subscriber_contains_entity (
    DDS_Subscriber _this,
    DDS_InstanceHandle_t  a_handle)
{
    DDS_ReturnCode_t result;
    _Subscriber sub;
    struct check_handle_arg arg;

    SAC_REPORT_STACK();

    arg.handle = a_handle;
    arg.result = FALSE;
    result = DDS_SubscriberClaimRead(_this, &sub);
    if (result == DDS_RETCODE_OK) {
        if (!arg.result) {
            c_iterWalkUntil(sub->readerList, (c_iterAction)reader_check_handle, &arg);
        }
        result = DDS_SubscriberRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return arg.result;
}

/*     ReturnCode_t
 *     delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_Subscriber_delete_contained_entities (
    DDS_Subscriber _this)
{
    DDS_ReturnCode_t result;
    _Subscriber sub;

    SAC_REPORT_STACK();

    result = DDS_SubscriberClaim(_this, &sub);
    if (result == DDS_RETCODE_OK) {
        result = _Subscriber_delete_contained_entities(sub);
        DDS_SubscriberRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

struct lookupByTopicArg {
    const DDS_char *topicName;
    DDS_DataReader reader;
};

static c_equality
lookupByTopic (
    c_voidp o,
    c_voidp arg)
{
    DDS_DataReader reader = (DDS_DataReader)o;
    struct lookupByTopicArg *a = (struct lookupByTopicArg *)arg;
    DDS_TopicDescription topic;
    DDS_char *topicName;
    c_equality equality = C_NE;

    topic = DDS_DataReader_get_topicdescription(reader);
    topicName = DDS_TopicDescription_get_name(topic);
    if (strcmp(topicName, a->topicName) == 0) {
        a->reader = reader;
        equality = C_EQ;
    }
    DDS_free(topicName);

    return equality;
}

/*     DataReader
 *     lookup_datareader(
 *         in string topic_name);
 */
DDS_DataReader
DDS_Subscriber_lookup_datareader (
    DDS_Subscriber _this,
    const DDS_char *topic_name)
{
    DDS_ReturnCode_t result;
    DDS_DataReader found = NULL;
    _Subscriber sub;
    struct lookupByTopicArg arg;
    os_char *name = NULL;

    SAC_REPORT_STACK();

    result = DDS_SubscriberClaimRead(_this, &sub);
    if (result == DDS_RETCODE_OK) {
        arg.topicName = topic_name;
        arg.reader = NULL;
        c_iterResolve(sub->readerList, lookupByTopic, &arg);
        found = arg.reader;
        if (found == NULL) {
            name = u_entityName(u_entity(_Subscriber_get_user_entity(sub)));
        }
        DDS_SubscriberRelease(_this);
    }
    if (name) {
        if (strcmp(name, "BuiltinSubscriber") == 0) {
            DDS_DomainParticipant participant;
            DDS_Topic topic;
            DDS_DataReaderQos *rQos = DDS_DataReaderQos__alloc();

            if (rQos) {
                memset(rQos, 0, sizeof(DDS_DataReaderQos));
                result = DDS_Subscriber_get_default_datareader_qos (_this, rQos);
                rQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;
                rQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

                participant = DDS_Subscriber_get_participant(_this);
                topic = DDS_DomainParticipant_lookup_builtin_topic(participant, topic_name);
                found = DDS_Subscriber_create_datareader(_this, topic, rQos, NULL, 0);

                DDS_free(rQos);
            } else {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Failed to allocate DDS_DataReaderQos");
            }
        }
        os_free(name);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return found;
}

/*     ReturnCode_t
 *     get_datareaders(
 *         inout DataReaderSeq readers,
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
DDS_ReturnCode_t
DDS_Subscriber_get_datareaders (
    DDS_Subscriber _this,
    DDS_DataReaderSeq *readers,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _Subscriber sub;
    u_dataReader uReader;
    u_result uResult;
    u_sampleMask mask;
    c_iter list = NULL;
    c_ulong length;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        if (!DDS_sequence_is_valid((_DDS_sequence)readers)) {
            result = DDS_RETCODE_BAD_PARAMETER;
        } else {
            result = DDS_SubscriberClaimRead(_this, &sub);
        }
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }

    if (result == DDS_RETCODE_OK) {
        uResult = u_subscriberGetDataReaders(_Subscriber_get_user_entity(sub), mask, &list);
        if (uResult == U_RESULT_OK) {
            length = c_iterLength(list);
            if (length > readers->_maximum) {
                if (readers->_release == TRUE) {
                    DDS_free(readers->_buffer);
                }
                readers->_buffer = DDS_DataReaderSeq_allocbuf(length);
                readers->_maximum = length;
                readers->_release = TRUE;
            }
            readers->_length = 0;
            while ((uReader = u_dataReader(c_iterTakeFirst(list))) != NULL) {
                readers->_buffer[readers->_length] = u_observableGetUserData(u_observable(uReader));
                readers->_length++;
            }
            c_iterFree(list);
            result = DDS_RETCODE_OK;
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
        DDS_SubscriberRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static void
notifyReader(DDS_Object reader, void *arg)
{
    DDS_DataReader dataReader = (DDS_DataReader) reader;
    OS_UNUSED_ARG(arg);
    if (DDS_DataReader_get_status_changes(reader) & DDS_DATA_AVAILABLE_STATUS) {
        struct DDS_DataReaderListener drListener;

        drListener = DDS_DataReader_get_listener(reader);
        if (drListener.on_data_available != NULL) {
            drListener.on_data_available(drListener.listener_data, dataReader);
        }
    }
}

/*     ReturnCode_t
 *     notify_datareaders();
 */
DDS_ReturnCode_t
DDS_Subscriber_notify_datareaders (
    DDS_Subscriber _this)
{
    DDS_ReturnCode_t result;
    _Subscriber s;

    SAC_REPORT_STACK();

    result = DDS_SubscriberClaim(_this, &s);
    if (result == DDS_RETCODE_OK) {
        c_iterWalk(s->readerList, notifyReader, NULL);
        DDS_SubscriberRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_qos(
 *         in SubscriberQos qos);
 */
DDS_ReturnCode_t
DDS_Subscriber_set_qos (
    DDS_Subscriber _this,
    const DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_SubscriberQos subscriberQos;
    _Subscriber s;
    u_subscriberQos sQos = NULL;
    u_result uResult;
    u_subscriber uSubscriber;

    SAC_REPORT_STACK();

    memset(&subscriberQos, 0, sizeof(DDS_SubscriberQos));
    (void)DDS_SubscriberQos_init(&subscriberQos, DDS_SUBSCRIBER_QOS_DEFAULT);

    result = DDS_SubscriberQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_SubscriberClaim(_this, &s);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_SUBSCRIBER_QOS_DEFAULT) {
            result = DDS_DomainParticipant_get_default_subscriber_qos(
                s->participant, &subscriberQos);
            qos = &subscriberQos;
        }
        if (result == DDS_RETCODE_OK) {
            sQos = DDS_SubscriberQos_copyIn(qos);
            if (sQos == NULL) {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Failed to copy DDS_SubscriberQos");
            }
        }
        if (result == DDS_RETCODE_OK) {
            uSubscriber = _Subscriber_get_user_entity(s);
            uResult = u_subscriberSetQos(uSubscriber, sQos);
            result = DDS_ReturnCode_get(uResult);
            if (result == DDS_RETCODE_OK) {
                s->factoryAutoEnable = sQos->entityFactory.v.autoenable_created_entities;
            }
            u_subscriberQosFree(sQos);
        }
        DDS_SubscriberRelease(_this);
    }

    (void)DDS_SubscriberQos_deinit(&subscriberQos);

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_qos(
 *         inout SubscriberQos qos);
 */
DDS_ReturnCode_t
DDS_Subscriber_get_qos (
    DDS_Subscriber _this,
    DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t result;
    _Subscriber s;
    u_subscriberQos uQos;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_SubscriberCheck(_this, &s);
    if (result == DDS_RETCODE_OK) {
        if (qos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "SubscriberQos = NULL");
        } else if (qos == DDS_SUBSCRIBER_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'SUBSCRIBER_QOS_DEFAULT' is read-only.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_subscriberGetQos(_Subscriber_get_user_entity(s), &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS_SubscriberQos_copyOut(uQos, qos);
            u_subscriberQosFree(uQos);
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_listener(
 *         in SubscriberListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_Subscriber_set_listener (
    DDS_Subscriber _this,
    const struct DDS_SubscriberListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;
    _Subscriber s;

    SAC_REPORT_STACK();

    result = DDS_SubscriberClaim(_this, &s);
    if (result == DDS_RETCODE_OK) {
        if (a_listener != NULL) {
            s->listener = *a_listener;
            result = DDS_Entity_set_listener_interest(DDS_Entity(s), mask);
        } else {
            memset(&s->listener, 0, sizeof(struct DDS_SubscriberListener));
            result = DDS_Entity_set_listener_interest(DDS_Entity(s), mask);
        }
        DDS_SubscriberRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     SubscriberListener
 *     get_listener();
 */
struct DDS_SubscriberListener
DDS_Subscriber_get_listener (
    DDS_Subscriber _this)
{
    DDS_ReturnCode_t result;
    _Subscriber s;
    struct DDS_SubscriberListener listener;

    SAC_REPORT_STACK();

    result = DDS_SubscriberCheck(_this, &s);
    if (result == DDS_RETCODE_OK) {
        listener = s->listener;
    } else {
        memset(&listener, 0, sizeof(struct DDS_SubscriberListener));
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return listener;
}

/*     ReturnCode_t
 *     set_listener_mask(
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_Subscriber_set_listener_mask (
    _Subscriber _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;

    assert(_this);

    result = DDS_Entity_set_listener_interest(DDS_Entity(_this), mask);

    return result;
}

/*     ReturnCode_t
 *     begin_access();
 */
DDS_ReturnCode_t
DDS_Subscriber_begin_access (
    DDS_Subscriber _this)
{
    DDS_ReturnCode_t result;
    _Subscriber s;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_SubscriberCheck(_this, &s);
    if (result == DDS_RETCODE_OK) {
        uResult = u_subscriberBeginAccess(_Subscriber_get_user_entity(s));
        if (uResult != U_RESULT_OK) {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     end_access();
 */
DDS_ReturnCode_t
DDS_Subscriber_end_access (
    DDS_Subscriber _this)
{
    DDS_ReturnCode_t result;
    _Subscriber s;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_SubscriberCheck(_this, &s);
    if (result == DDS_RETCODE_OK) {
        uResult = u_subscriberEndAccess(_Subscriber_get_user_entity(s));
        if (uResult != U_RESULT_OK) {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     DomainParticipant
 *     get_participant();
 */
DDS_DomainParticipant
DDS_Subscriber_get_participant (
    DDS_Subscriber _this)
{
    DDS_ReturnCode_t result;
    _Subscriber s;
    DDS_DomainParticipant participant = NULL;

    SAC_REPORT_STACK();

    result = DDS_SubscriberCheck(_this, &s);
    if (result == DDS_RETCODE_OK) {
        participant = s->participant;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return participant;
}

/*     ReturnCode_t
 *     set_default_datareader_qos(
 *         in DataReaderQos qos);
 */
DDS_ReturnCode_t
DDS_Subscriber_set_default_datareader_qos (
    DDS_Subscriber _this,
    const DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Subscriber s;
    DDS_DataReaderQos *readerQos = NULL;

    SAC_REPORT_STACK();

    if (qos == DDS_DATAREADER_QOS_USE_TOPIC_QOS) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderQos = DDS_DATAREADER_QOS_USE_TOPIC_QOS");
    } else {
        result = DDS_DataReaderQos_is_consistent(qos);
    }
    if (result == DDS_RETCODE_OK) {
        readerQos = DDS_DataReaderQos__alloc();
        if (readerQos != NULL) {
            result = DDS_DataReaderQos_init(readerQos, qos);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to copy DDS_DataReaderQos");
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_SubscriberClaim(_this, &s);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_free(s->defaultDataReaderQos);
        s->defaultDataReaderQos = readerQos;
        DDS_SubscriberRelease(_this);
    } else {
        DDS_free(readerQos);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_default_datareader_qos(
 *         inout DataReaderQos qos);
 */
DDS_ReturnCode_t
DDS_Subscriber_get_default_datareader_qos (
    DDS_Subscriber _this,
    DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t result;
    _Subscriber s;

    SAC_REPORT_STACK();

    result = DDS_SubscriberClaimRead(_this, &s);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DataReaderQos_init(qos, s->defaultDataReaderQos);
        DDS_SubscriberRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     copy_from_topic_qos(
 *         inout DataReaderQos a_datareader_qos,
 *         in TopicQos a_topic_qos);
 */
DDS_ReturnCode_t
DDS_Subscriber_copy_from_topic_qos (
    DDS_Subscriber _this,
    DDS_DataReaderQos *rQos,
    const DDS_TopicQos *tQos)
{
    DDS_ReturnCode_t result;
    DDS_TopicQos *topicQos = NULL;
    _Subscriber s;

    SAC_REPORT_STACK();

    result = DDS_SubscriberCheck(_this, &s);
    if (result == DDS_RETCODE_OK) {
        if (rQos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "a_datareader_qos 'null' is invalid.");
        } else if (rQos == DDS_DATAREADER_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT' is read-only.");
        } else if (rQos == DDS_DATAREADER_QOS_USE_TOPIC_QOS) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS' is read-only.");
        } else if (tQos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "a_topic_qos 'null' is invalid.");
        } else if (tQos == DDS_TOPIC_QOS_DEFAULT) {
            topicQos = DDS_TopicQos__alloc();
            if (topicQos == NULL) {
                result = DDS_RETCODE_ERROR;
                SAC_REPORT(result, "Could not copy DataReaderQos.");
            } else {
                result = DDS_DomainParticipant_get_default_topic_qos(
                    s->participant, topicQos);
                if (result == DDS_RETCODE_OK) {
                    tQos = topicQos;
                }
            }
        }

        if (result == DDS_RETCODE_OK) {
            rQos->durability         = tQos->durability;
            rQos->deadline           = tQos->deadline;
            rQos->latency_budget     = tQos->latency_budget;
            rQos->liveliness         = tQos->liveliness;
            rQos->reliability        = tQos->reliability;
            rQos->destination_order  = tQos->destination_order;
            rQos->history            = tQos->history;
            rQos->ownership          = tQos->ownership;
            rQos->resource_limits    = tQos->resource_limits;
        }
    }

    if (topicQos != NULL) {
        DDS__free(topicQos);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_Subscriber_copy_from_topicdescription (
    DDS_Subscriber _this,
    DDS_DataReaderQos *qos,
    const DDS_TopicDescription description)
{
    DDS_ReturnCode_t result;
    DDS_ObjectKind objectKind;
    DDS_Topic topic;
    DDS_TopicQos topicQos;

    assert (_this != NULL);
    assert (qos != NULL);
    assert (description != NULL);

    /* initialize to safely deinitialize on exit */
    memset(&topicQos, 0, sizeof(DDS_TopicQos));
    (void)DDS_TopicQos_init(&topicQos, DDS_TOPIC_QOS_DEFAULT);

    objectKind = DDS_Object_get_kind(DDS_Object(description));
    switch (objectKind) {
        case DDS_CONTENTFILTEREDTOPIC:
            topic = DDS_ContentFilteredTopic_get_related_topic(description);
            break;
        default:
            assert (objectKind == DDS_TOPIC);
            topic = DDS_Topic(description);
            break;
    }

    assert (topic != NULL);

    result = DDS_Topic_get_qos(topic, &topicQos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_Subscriber_copy_from_topic_qos(_this, qos, &topicQos);
    }

    (void)DDS_TopicQos_deinit(&topicQos);

    return result;
}

DDS_ReturnCode_t
DDS_Subscriber_notify_listener (
    DDS_Subscriber _this,
    v_listenerEvent event)
{
    DDS_ReturnCode_t result;
    u_eventMask triggerMask;
    DDS_Entity entity;
    struct DDS_SubscriberListener cb;

    cb = _Subscriber(_this)->listener;
    triggerMask = event->kind;
    result = DDS_RETCODE_OK;

    entity = u_observableGetUserData(u_observable(event->source));

    if (triggerMask & V_EVENT_ON_DATA_ON_READERS) {
        if (cb.on_data_on_readers != NULL) {
            result = DDS_Entity_reset_on_data_on_readers_status(DDS_Entity(entity));
            if (result == DDS_RETCODE_OK) {
                cb.on_data_on_readers(cb.listener_data, entity);
            }
        }
    } else {
        if (triggerMask & V_EVENT_DATA_AVAILABLE) {
            if (cb.on_data_available != NULL) {
                result = DDS_Entity_reset_dataAvailable_status(DDS_Entity(entity));
                if (result == DDS_RETCODE_OK) {
                    cb.on_data_available(cb.listener_data, entity);
                }
            }
        }
    }
    if ((triggerMask & V_EVENT_SAMPLE_REJECTED) &&
            (cb.on_sample_rejected != NULL))
    {
        DDS_SampleRejectedStatus status;
        DDS_SampleRejectedStatus_init(&status, &((v_readerStatus)event->eventData)->sampleRejected);
        cb.on_sample_rejected(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_LIVELINESS_CHANGED) &&
            (cb.on_liveliness_changed != NULL))
    {
        DDS_LivelinessChangedStatus status;
        DDS_LivelinessChangedStatus_init(&status, &((v_readerStatus)event->eventData)->livelinessChanged);
        cb.on_liveliness_changed(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED) &&
            (cb.on_requested_deadline_missed != NULL))
    {
        DDS_RequestedDeadlineMissedStatus status;
        DDS_RequestedDeadlineMissedStatus_init(&status, &((v_readerStatus)event->eventData)->deadlineMissed);
        cb.on_requested_deadline_missed(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) &&
            (cb.on_requested_incompatible_qos != NULL))
    {
        DDS_RequestedIncompatibleQosStatus status;
        DDS_RequestedIncompatibleQosStatus_init(&status, &((v_readerStatus)event->eventData)->incompatibleQos);
        cb.on_requested_incompatible_qos(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_SAMPLE_LOST) &&
            (cb.on_sample_lost != NULL))
    {
        DDS_SampleLostStatus status;
        DDS_SampleLostStatus_init(&status, &((v_readerStatus)event->eventData)->sampleLost);
        cb.on_sample_lost(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_SUBSCRIPTION_MATCHED) &&
            (cb.on_subscription_matched != NULL))
    {
        DDS_SubscriptionMatchedStatus status;
        DDS_SubscriptionMatchedStatus_init(&status, &((v_readerStatus)event->eventData)->subscriptionMatch);
        cb.on_subscription_matched(cb.listener_data, entity, &status);
    }

    return result;
}
