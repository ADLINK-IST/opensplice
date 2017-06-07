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
#include "u_user.h"
#include "Constants.h"
#include "QosUtils.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "Subscriber.h"
#include "TypeSupportMetaHolder.h"
#include "u_observable.h"

using namespace DDS::OpenSplice::Utils;

DDS::OpenSplice::Subscriber::Subscriber() :
    DDS::OpenSplice::Entity(DDS::OpenSplice::SUBSCRIBER),
    participant(NULL),
    defaultReaderQos(DATAREADER_QOS_DEFAULT),
    readers(new ObjSet(TRUE)),
    factoryAutoEnable(FALSE)
{
    /* do nothing */
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::init (
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *name,
    const DDS::SubscriberQos &qos)
{
    return this->nlReq_init(participant, name, qos);
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::nlReq_init (
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *name,
    const DDS::SubscriberQos &qos)
{
    DDS::ReturnCode_t result;
    u_participant uParticipant;
    u_subscriber uSubscriber;
    u_subscriberQos uSubscriberQos;

    assert (name != NULL);

    /* participant should have verified qos consistency */
    uSubscriberQos = u_subscriberQosNew (NULL);
    if (uSubscriberQos) {
        result = DDS::OpenSplice::Utils::copyQosIn (qos, uSubscriberQos);
    } else {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
        CPP_REPORT(result, "Could not copy SubscriberQos.");
    }

    if (result == DDS::RETCODE_OK) {
        result = readers->init();
    }

    if (result == DDS::RETCODE_OK) {
        uParticipant = u_participant (participant->rlReq_get_user_entity ());
        uSubscriber = u_subscriberNew (uParticipant, name, uSubscriberQos, FALSE);
        if (uSubscriber) {
            result = DDS::OpenSplice::Entity::nlReq_init (u_entity (uSubscriber));
            if (result == DDS::RETCODE_OK) {
                this->factoryAutoEnable
                    = qos.entity_factory.autoenable_created_entities;
                (void) DDS::DomainParticipant::_duplicate(participant);
                this->participant = participant;
                setDomainId(participant->getDomainId());
            }
        } else {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create Subscriber.");
        }
    }

    if (uSubscriberQos) {
        u_subscriberQosFree (uSubscriberQos);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::wlReq_deinit()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (readers->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "Subscriber still contains '%d' DataReader entities.",
            readers->getNrElements());
    } else {
        this->disable_callbacks();

        /* Here readers list is always empty, list will be destroyed by destructor. */

        if (this->participant) {
            DDS::release(this->participant);
            this->participant = NULL;
        }

        /* This will also free the user layer object. */
        result = DDS::OpenSplice::Entity::wlReq_deinit();
    }

    return result;
}

DDS::OpenSplice::Subscriber::~Subscriber()
{
    delete readers;
}

DDS::Boolean
DDS::OpenSplice::Subscriber::wlReq_insertReader(
        DDS::OpenSplice::DataReader *reader)
{
    return readers->insertElement(reader);
}

DDS::Boolean
DDS::OpenSplice::Subscriber::wlReq_removeReader(
        DDS::OpenSplice::DataReader *reader)
{
    return readers->removeElement(reader);
}

char *
DDS::OpenSplice::Subscriber::create_datareader_name (
    DDS::OpenSplice::TopicDescription *a_topic)
{
    char *name;
    char *topic_name;
    unsigned int length;

    assert (a_topic != NULL);

    length = strlen("reader <>") + 1;
    topic_name = a_topic->get_name ();
    if (topic_name != NULL) {
        length += strlen(topic_name);
        name = DDS::string_alloc(length);
        if (name != NULL) {
            snprintf (name, length, "reader <%s>", topic_name);
        }
        DDS::string_free (topic_name);
    } else {
        name = DDS::string_alloc(length);
        if (name != NULL) {
            snprintf (name, length, "reader");
        }
    }

    if (name == NULL) {
        CPP_REPORT(DDS::RETCODE_OUT_OF_RESOURCES, "Could not allocate memory.");
    }

    return name;
}

DDS::DataReader_ptr
DDS::OpenSplice::Subscriber::create_datareader (
    DDS::TopicDescription_ptr a_topic,
    const DDS::DataReaderQos &qos,
    DDS::DataReaderListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::DataReader *reader = NULL;
    DDS::OpenSplice::TopicDescription *topic;
    DDS::OpenSplice::TypeSupportMetaHolder *tsMetaHolder = NULL;
    DDS::DataReaderQos mergedQos;
    const DDS::DataReaderQos *readerQosPtr;
    char *name = NULL;

    CPP_REPORT_STACK();

    if (a_topic == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_topic '<NULL>' is invalid.");
    } else {
        topic = dynamic_cast<DDS::OpenSplice::TopicDescription *>(a_topic);
        if (!topic) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_topic is invalid, not of type '%s'.",
                "DDS::OpenSplice::TopicDescription");
        }
    }

    if (result == DDS::RETCODE_OK) {
        tsMetaHolder = topic->get_typesupport_meta_holder();
        if (!tsMetaHolder) {
            result = DDS::RETCODE_BAD_PARAMETER;
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
        if (result == DDS::RETCODE_OK) {
            if (&qos == &DATAREADER_QOS_DEFAULT) {
                readerQosPtr = &this->defaultReaderQos;
            } else if (&qos == &DATAREADER_QOS_USE_TOPIC_QOS){
                readerQosPtr = &mergedQos;
                mergedQos = this->defaultReaderQos;
                result = this->copy_from_topicdescription(mergedQos, topic);
                if (result == DDS::RETCODE_OK) {
                    result = DDS::OpenSplice::Utils::qosIsConsistent(mergedQos);
                }
            } else {
                readerQosPtr = &qos;
                result = DDS::OpenSplice::Utils::qosIsConsistent (qos);
            }
            if (result == DDS::RETCODE_OK) {
                name = this->create_datareader_name (topic);
                if (!name) {
                    result = DDS::RETCODE_OUT_OF_RESOURCES;
                }
            }
            if (result == DDS::RETCODE_OK) {
                reader = tsMetaHolder->create_datareader();
                if (reader) {
                    result = reader->init (this, *readerQosPtr, topic, name, tsMetaHolder->get_copy_in(), tsMetaHolder->get_copy_out());
                    if (result == DDS::RETCODE_OK) {
                        /* Duplicate and store the created DataWriter object. */
                        if (!this->wlReq_insertReader (reader)) {
                            result = DDS::RETCODE_OUT_OF_RESOURCES;
                        } else {
                            reader->wlReq_set_listenerDispatcher(this->rlReq_get_listenerDispatcher());
                            result = reader->set_listener(a_listener, mask);
                            if (result == DDS::RETCODE_OK && this->factoryAutoEnable) {
                                result = reader->enable();
                            }
                            /* Remove DataReader from list if an error occurred */
                            if (result != DDS::RETCODE_OK) {
                                (void)reader->set_listener(NULL, 0);
                                reader->wlReq_set_listenerDispatcher(NULL);
                                (void)this->wlReq_removeReader (reader);
                            }
                        }
                        /* Destroy DataReader if an error occurred */
                        if (result != DDS::RETCODE_OK) {
                            (void) reader->deinit ();
                        }
                    }
                    if (result != DDS::RETCODE_OK) {
                        DDS::release(reader);
                        reader = NULL;
                    }
                } else {
                    result = DDS::RETCODE_OUT_OF_RESOURCES;
                }
            }
            this->unlock ();
        }
    }

    if (name) {
        DDS::string_free (name);
    }
    if (tsMetaHolder) {
        DDS::release (tsMetaHolder);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return reader;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::delete_datareader (
    DDS::DataReader_ptr a_datareader
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::DataReader *reader;

    CPP_REPORT_STACK();

    if (a_datareader == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_datareader '<NULL>' is invalid.");
    } else {
        reader = dynamic_cast<DDS::OpenSplice::DataReader *>(a_datareader);
        if (reader == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_datareader is invalid, not of type '%s'.",
                "DDS::OpenSplice::DataReader");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        if (wlReq_removeReader(reader) == FALSE) {
            if (reader->get_kind() == DDS::OpenSplice::DATAREADER) {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
            } else {
                result = DDS::RETCODE_BAD_PARAMETER;
            }
            CPP_REPORT(result, "DataReader not created by Subscriber.");
        }

        if (result == DDS::RETCODE_OK) {
            /* Destroy the Reader. */
            (void)reader->set_listener(NULL, 0);
            result = reader->deinit ();
            if (result != DDS::RETCODE_OK) {
                if (result == DDS::RETCODE_PRECONDITION_NOT_MET) {
                    /* Re-store when the deinit didn't work. */
                    DDS::Boolean reinserted = wlReq_insertReader(reader);
                    assert(reinserted);
                    OS_UNUSED_ARG(reinserted);
                }
            }
        }

        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        wlReq_deleteFactoryList<DDS::OpenSplice::DataReader *>(readers);
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::Boolean
DDS::OpenSplice::Subscriber::rlReq_lookupByTopic (
    DDS::Object_ptr element,
    lookupByTopicArg *arg)
{
    DDS::OpenSplice::DataReader *reader;
    DDS::OpenSplice::TopicDescription *topic;
    const char *topicName;
    DDS::Boolean result = TRUE;

    assert (element);
    assert (arg);

    reader = dynamic_cast<DDS::OpenSplice::DataReader *>(element);
    topic = reader->get_topic();
    topicName = topic->topic_name.in();
    if (strcmp(topicName, arg->topicName) == 0) {
        arg->reader = reader;
        result = FALSE;
    }

    return result;
}

DDS::DataReader_ptr
DDS::OpenSplice::Subscriber::lookup_datareader (
    const char *topic_name
) THROW_ORB_EXCEPTIONS
{
    DDS::DataReader_ptr reader = NULL;
    DDS::ReturnCode_t result;
    lookupByTopicArg arg;
    u_subscriber uSubscriber;
    os_char *name = NULL;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        arg.topicName = topic_name;
        arg.reader = NULL;
        (void)readers->walk((DDS::OpenSplice::ObjSet::ObjSetActionFunc) rlReq_lookupByTopic, &arg);
        if (arg.reader == NULL) {
            uSubscriber = u_subscriber (rlReq_get_user_entity ());
            name = u_entityName(u_entity(uSubscriber));
        } else {
            reader = DDS::DataReader::_duplicate(arg.reader);
        }
        this->unlock ();
    }
    if (name) {
        if (strcmp(name, "BuiltinSubscriber") == 0) {
            DDS::OpenSplice::DomainParticipant *participant;
            DDS::Topic_var topic;
            DDS::DataReaderQos rQos = DATAREADER_QOS_DEFAULT;

            rQos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
            rQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;

            participant = dynamic_cast<DDS::OpenSplice::DomainParticipant *>(this->get_participant());
            if (participant) {
                topic = participant->find_builtin_topic(topic_name);
                DDS::release(participant);
                reader = this->create_datareader(topic, rQos, NULL, 0);
            } else {
                result = DDS::RETCODE_ERROR;
            }
        }
        os_free(name);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return reader;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::get_datareaders (
    DDS::DataReaderSeq& readers,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_sampleMask mask;
    u_subscriber uSubscriber;
    u_dataReader uReader;
    u_result uResult;
    c_iter list;
    c_long length;
    DDS::ULong i;
    DDS::OpenSplice::Entity *entity;
    DDS::DataReader_ptr reader;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->read_lock ();
        if (result == DDS::RETCODE_OK) {
            uSubscriber = u_subscriber (rlReq_get_user_entity ());
            uResult = u_subscriberGetDataReaders(uSubscriber, mask, &list);
            if (uResult == U_RESULT_OK) {
                length = c_iterLength(list);
                assert(length >= 0);
                readers.length(length);
                i = 0;
                while ((uReader = u_dataReader(c_iterTakeFirst(list))) != NULL) {
                    entity = reinterpret_cast<DDS::OpenSplice::Entity *>(u_observableGetUserData(u_observable(uReader)));
                    reader = dynamic_cast<DDS::DataReader_ptr>(entity);
                    readers[i] = DDS::DataReader::_duplicate(reader);
                    i++;
                }
                c_iterFree(list);
                result = DDS::RETCODE_OK;
            } else {
                result = uResultToReturnCode (uResult);
            }
            this->unlock();
        }
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

static DDS::Boolean
notifyReader(DDS::Object_ptr reader, void *arg)
{
    DDS::Boolean result = FALSE;
    OS_UNUSED_ARG(arg);
    DDS::DataReader_ptr dataReader = dynamic_cast<DDS::DataReader_ptr>(reader);
    if (dataReader) {
        if (dataReader->get_status_changes() & DDS::DATA_AVAILABLE_STATUS) {
            DDS::DataReaderListener_var drListener;

            drListener = dataReader->get_listener();
            if (drListener.in() != NULL) {
                drListener->on_data_available(dataReader);
            }
        }
        result = TRUE;
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::notify_datareaders (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        (void) readers->walk(notifyReader, NULL);
        this->unlock ();
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::set_qos (
    const DDS::SubscriberQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::SubscriberQos subscriberQos;
    DDS::SubscriberQos *subscriberQos_ptr;
    u_subscriber uSubscriber;
    u_subscriberQos uSubscriberQos;
    u_result uResult;

    CPP_REPORT_STACK();

    if (&qos == &SUBSCRIBER_QOS_DEFAULT) {
        subscriberQos_ptr = NULL;
        result = DDS::RETCODE_OK;
    } else {
        subscriberQos_ptr = const_cast<DDS::SubscriberQos *>(&qos);
        result = DDS::OpenSplice::Utils::qosIsConsistent (qos);
    }

    if (result == DDS::RETCODE_OK) {
        uSubscriberQos = u_subscriberQosNew (NULL);
        if (uSubscriberQos == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not copy SubscriberQos.");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
        if (result == DDS::RETCODE_OK) {
            if (subscriberQos_ptr == NULL) {
                result = participant->get_default_subscriber_qos (subscriberQos);
                subscriberQos_ptr = &subscriberQos;
            }

            if (result == DDS::RETCODE_OK) {
                result = DDS::OpenSplice::Utils::copyQosIn (
                    *subscriberQos_ptr, uSubscriberQos);
                if (result == DDS::RETCODE_OK) {
                    uSubscriber = u_subscriber (rlReq_get_user_entity ());
                    uResult = u_subscriberSetQos (uSubscriber, uSubscriberQos);
                    result = uResultToReturnCode (uResult);
                    if (result == DDS::RETCODE_OK) {
                        factoryAutoEnable =
                            subscriberQos_ptr->entity_factory.autoenable_created_entities;
                    } else {
                        CPP_REPORT(result, "Could not apply SubscriberQos.");
                    }
                }
            }
            unlock ();
        }

        u_subscriberQosFree (uSubscriberQos);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::get_qos (
    DDS::SubscriberQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_subscriber uSubscriber;
    u_subscriberQos uSubscriberQos;
    u_result uResult;

    CPP_REPORT_STACK();

    if (&qos == &SUBSCRIBER_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'SUBSCRIBER_QOS_DEFAULT' is read-only.");
    } else {
        result = this->check ();
    }

    if (result == DDS::RETCODE_OK) {
        uSubscriber = u_subscriber (rlReq_get_user_entity ());
        uResult = u_subscriberGetQos (uSubscriber, &uSubscriberQos);
        result = uResultToReturnCode (uResult);
        if (result == DDS::RETCODE_OK) {
            result = DDS::OpenSplice::Utils::copyQosOut (uSubscriberQos, qos);
            u_subscriberQosFree (uSubscriberQos);
        } else {
            CPP_REPORT(result, "Could not copy SubscriberQos.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::set_listener (
    DDS::SubscriberListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = DDS::OpenSplice::Entity::nlReq_set_listener(a_listener, mask);

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::SubscriberListener_ptr
DDS::OpenSplice::Subscriber::get_listener (
) THROW_ORB_EXCEPTIONS
{
    DDS::SubscriberListener_ptr listener;

    CPP_REPORT_STACK();
    listener = dynamic_cast<DDS::SubscriberListener_ptr>(
        DDS::OpenSplice::Entity::nlReq_get_listener());
    CPP_REPORT_FLUSH(this, listener == NULL);

    return listener;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::begin_access (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_subscriber uSubscriber;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uSubscriber = u_subscriber (rlReq_get_user_entity ());
        uResult = u_subscriberBeginAccess(uSubscriber);
        result = uResultToReturnCode (uResult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not Begin coherent access.");
        }
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::end_access (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_subscriber uSubscriber;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uSubscriber = u_subscriber (rlReq_get_user_entity ());
        uResult = u_subscriberEndAccess(uSubscriber);
        result = uResultToReturnCode (uResult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not Begin coherent access.");
        }
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::DomainParticipant_ptr
DDS::OpenSplice::Subscriber::get_participant (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::DomainParticipant_ptr participantPtr = NULL;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        participantPtr = DDS::DomainParticipant::_duplicate (this->participant);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return participantPtr;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::set_default_datareader_qos (
    const DDS::DataReaderQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    CPP_REPORT_STACK();

    if (&qos == &DATAREADER_QOS_USE_TOPIC_QOS) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS' is invalid in this context.");
    } else if (&qos != &DATAREADER_QOS_DEFAULT) {
        result = DDS::OpenSplice::Utils::qosIsConsistent (qos);
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        this->defaultReaderQos = qos;
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::get_default_datareader_qos (
    DDS::DataReaderQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    CPP_REPORT_STACK();

    if (&qos == &DATAREADER_QOS_DEFAULT) {
        CPP_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT' is read-only.");
    } else if (&qos == &DATAREADER_QOS_USE_TOPIC_QOS) {
        CPP_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS is read-only.");
    } else {
        result = this->read_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        qos = this->defaultReaderQos;
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::copy_from_topic_qos (
    DDS::DataReaderQos &a_datareader_qos,
    const DDS::TopicQos &a_topic_qos
) THROW_ORB_EXCEPTIONS
{
    DDS::TopicQos topicQos;
    DDS::TopicQos *topicQos_ptr = NULL;
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::DomainParticipant_ptr participant;

    CPP_REPORT_STACK();

    if (&a_datareader_qos == &DATAREADER_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT' is read-only.");
    } else if (&a_datareader_qos == &DATAREADER_QOS_USE_TOPIC_QOS) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT_USE_TOPIC_QOS' is read-only.");
    } else if (&a_topic_qos == &TOPIC_QOS_DEFAULT) {
        participant = this->get_participant ();
        if (participant != NULL) {
            result = participant->get_default_topic_qos (topicQos);
            if (result == DDS::RETCODE_OK) {
                topicQos_ptr = &topicQos;
            }
            DDS::release (participant);
        } else {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
        }
    } else {
        topicQos_ptr = const_cast<DDS::TopicQos *>(&a_topic_qos);
    }

    if (result == DDS::RETCODE_OK) {
        a_datareader_qos.durability = topicQos_ptr->durability;
        a_datareader_qos.deadline = topicQos_ptr->deadline;
        a_datareader_qos.latency_budget = topicQos_ptr->latency_budget;
        a_datareader_qos.liveliness = topicQos_ptr->liveliness;
        a_datareader_qos.reliability = topicQos_ptr->reliability;
        a_datareader_qos.destination_order = topicQos_ptr->destination_order;
        a_datareader_qos.history = topicQos_ptr->history;
        a_datareader_qos.resource_limits = topicQos_ptr->resource_limits;
        a_datareader_qos.ownership = topicQos_ptr->ownership;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Subscriber::copy_from_topicdescription (
    DDS::DataReaderQos &a_datareader_qos,
    const DDS::OpenSplice::TopicDescription *a_topic)
{
    DDS::ContentFilteredTopic_ptr contentFilteredTopic;
    DDS::Topic_ptr topic = NULL;
    DDS::TopicQos topicQos;
    DDS::OpenSplice::TopicDescription *topicDescription;
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    assert (a_topic != NULL);

    topicDescription = const_cast<DDS::OpenSplice::TopicDescription *>(a_topic);

    switch (topicDescription->get_kind ()) {
    case DDS::OpenSplice::CONTENTFILTEREDTOPIC:
        contentFilteredTopic = dynamic_cast<DDS::ContentFilteredTopic_ptr>(topicDescription);
        if (contentFilteredTopic != NULL) {
            topic = contentFilteredTopic->get_related_topic ();
            /* Duplicated topic is returned. Can be released right away
             * as topic exists as long as contentFilteredTopic exists.
             */
            DDS::release(topic);
        }
    break;
    case DDS::OpenSplice::TOPIC:
        topic = dynamic_cast<DDS::Topic_ptr>(topicDescription);
    break;
    default:
    break;
    }

    if (topic != NULL) {
        result = topic->get_qos (topicQos);
        if (result == DDS::RETCODE_OK) {
            result = this->copy_from_topic_qos (a_datareader_qos, topicQos);
        }
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
    }

    return result;
}

DDS::Boolean
DDS::OpenSplice::Subscriber::rlReq_readerCheckHandle (
    DDS::Object_ptr object,
    DDS::InstanceHandle_t *argHandle)
{
    DDS::OpenSplice::DataReader* reader;
    DDS::Boolean result = TRUE;

    assert(object);
    assert(argHandle);

    reader = dynamic_cast<DDS::OpenSplice::DataReader *>(object);
    if (reader) {
        if (reader->get_instance_handle() == *argHandle) {
            result = FALSE;
        }
    }

    return result;
}

DDS::Boolean
DDS::OpenSplice::Subscriber::contains_entity (
    DDS::InstanceHandle_t a_handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Boolean contained = FALSE;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        contained = (readers->walk((DDS::OpenSplice::ObjSet::ObjSetActionFunc) rlReq_readerCheckHandle, &a_handle) == FALSE);
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return contained;
}

void
DDS::OpenSplice::Subscriber::nlReq_notify_listener(
    DDS::OpenSplice::Entity *sourceEntity,
    DDS::ULong               triggerMask,
    void                    *eventData)
{
    DDS::SubscriberListener_ptr listener;
    DDS::ReturnCode_t result;

    /* Using _narrow to cast Listener, this increases the refcount to ensure
     * the ListenerObject is not deleted while notifying. */
    listener = DDS::SubscriberListener::_narrow(this->listener);
    if (listener) {

        if (triggerMask & V_EVENT_ON_DATA_ON_READERS) {
            result = sourceEntity->reset_on_data_on_readers_status();
            if (result == DDS::RETCODE_OK) {
                listener->on_data_on_readers(dynamic_cast<DDS::Subscriber_ptr>(sourceEntity));
            }
        } else {
            if (triggerMask & V_EVENT_DATA_AVAILABLE) {
                result = sourceEntity->reset_dataAvailable_status();
                if (result == DDS::RETCODE_OK) {
                    listener->on_data_available(dynamic_cast<DDS::DataReader_ptr>(sourceEntity));
                }
            }
        }

        if (triggerMask & V_EVENT_SAMPLE_REJECTED) {
            DDS::SampleRejectedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->sampleRejected,
                                                  status);
            listener->on_sample_rejected(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                         status);
        }

        if (triggerMask & V_EVENT_LIVELINESS_CHANGED) {
            DDS::LivelinessChangedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->livelinessChanged,
                                                  status);
            listener->on_liveliness_changed(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                            status);
        }

        if (triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            DDS::RequestedDeadlineMissedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->deadlineMissed,
                                                  status);
            listener->on_requested_deadline_missed(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                                   status);
        }

        if (triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            DDS::RequestedIncompatibleQosStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->incompatibleQos,
                                                  status);
            listener->on_requested_incompatible_qos(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                                    status);
        }

        if (triggerMask & V_EVENT_SAMPLE_LOST) {
            DDS::SampleLostStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->sampleLost,
                                                  status);
            listener->on_sample_lost(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                     status);
        }

        if (triggerMask & V_EVENT_SUBSCRIPTION_MATCHED) {
            DDS::SubscriptionMatchedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->subscriptionMatch,
                                                  status);
            listener->on_subscription_matched(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                              status);
        }
        DDS::release(listener);
    }
}
