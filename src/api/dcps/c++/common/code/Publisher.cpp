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
#include "Publisher.h"
#include "QosUtils.h"
#include "TypeSupportMetaHolder.h"
#include "StatusUtils.h"
#include "MiscUtils.h"
#include "ReportUtils.h"
#include "Constants.h"
#include "u_user.h"
#include "u_observable.h"


using namespace DDS::OpenSplice::Utils;

DDS::OpenSplice::Publisher::Publisher() :
    DDS::OpenSplice::Entity(DDS::OpenSplice::PUBLISHER),
    participant(NULL),
    defaultWriterQos(DATAWRITER_QOS_DEFAULT),
    writers(new ObjSet(TRUE)),
    factoryAutoEnable(FALSE)
{
    /* do nothing */
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::init(
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *name,
    const DDS::PublisherQos &qos)
{
    return nlReq_init(participant, name, qos);
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::nlReq_init(
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *name,
    const DDS::PublisherQos &qos)
{
    DDS::ReturnCode_t result;
    u_participant uParticipant;
    u_publisher uPublisher;
    u_publisherQos uPublisherQos;

    assert (name);

    /* participant should have verified qos consistency */
    uPublisherQos = u_publisherQosNew (NULL);
    if (uPublisherQos) {
        result = DDS::OpenSplice::Utils::copyQosIn (qos, uPublisherQos);
    } else {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
        CPP_REPORT(result, "Could not copy PublisherQos.");
    }

    if (result == DDS::RETCODE_OK) {
        result = writers->init();
    }

    if (result == DDS::RETCODE_OK) {
        uParticipant = u_participant (participant->rlReq_get_user_entity ());
        uPublisher = u_publisherNew (uParticipant, name, uPublisherQos, FALSE);
        if (uPublisher) {
            result = DDS::OpenSplice::Entity::nlReq_init (u_entity (uPublisher));
            if (result == DDS::RETCODE_OK) {
                this->factoryAutoEnable
                    = qos.entity_factory.autoenable_created_entities;
                (void) DDS::DomainParticipant::_duplicate(participant);
                this->participant = participant;
                setDomainId(participant->getDomainId());
            }
        } else {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create Publisher.");
        }
    }

    if (uPublisherQos) {
        u_publisherQosFree (uPublisherQos);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::wlReq_deinit()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (writers->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "Publisher still contains '%d' DataWriter entities.",
            writers->getNrElements());
    } else {
        this->disable_callbacks();

        /* Here writers list is always empty, list will be destroyed by destructor. */

        if (this->participant) {
            DDS::release(this->participant);
            this->participant = NULL;
        }

        /* This will also free the user layer object. */
        result = DDS::OpenSplice::Entity::wlReq_deinit();
    }

    return result;
}

DDS::OpenSplice::Publisher::~Publisher()
{
    // Delegate to parent.
    delete writers;
}

DDS::Boolean
DDS::OpenSplice::Publisher::wlReq_insertWriter(
        DDS::OpenSplice::DataWriter *writer)
{
    return writers->insertElement(writer);
}

DDS::Boolean
DDS::OpenSplice::Publisher::wlReq_removeWriter(
        DDS::OpenSplice::DataWriter *writer)
{
    return writers->removeElement(writer);
}

DDS::Boolean
DDS::OpenSplice::Publisher::rlReq_writerCheckHandle (
    DDS::Object_ptr object,
    DDS::InstanceHandle_t *argHandle)
{
    DDS::OpenSplice::DataWriter* writer;
    DDS::Boolean result = TRUE;

    assert(object);
    assert(argHandle);

    writer = dynamic_cast<DDS::OpenSplice::DataWriter *>(object);
    if (writer) {
        if (writer->get_instance_handle () == *argHandle) {
            result = FALSE;
        }
    }

    return result;
}

DDS::Boolean
DDS::OpenSplice::Publisher::contains_entity (
    DDS::InstanceHandle_t a_handle) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Boolean contained = FALSE;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        contained = (writers->walk((DDS::OpenSplice::ObjSet::ObjSetActionFunc) rlReq_writerCheckHandle, &a_handle) == FALSE);
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return contained;
}

char *
DDS::OpenSplice::Publisher::create_datawriter_name (
    DDS::OpenSplice::Topic *a_topic)
{
    char *name;
    char *topic_name;
    unsigned int length;

    assert (a_topic != NULL);

    length = strlen("writer <>") + 1;
    topic_name = a_topic->get_name ();
    if (topic_name != NULL) {
        length += strlen(topic_name);
        name = DDS::string_alloc(length);
        if (name != NULL) {
            snprintf (name, length, "writer <%s>", topic_name);
        }
        DDS::string_free (topic_name);
    } else {
        name = DDS::string_alloc(length);
        if (name != NULL) {
            snprintf (name, length, "writer");
        }
    }

    if (name == NULL) {
        CPP_REPORT(DDS::RETCODE_OUT_OF_RESOURCES, "Could not allocate memory.");
    }

    return name;
}

DDS::DataWriter_ptr
DDS::OpenSplice::Publisher::create_datawriter (
    DDS::Topic_ptr a_topic,
    const DDS::DataWriterQos &qos,
    DDS::DataWriterListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::DataWriter *writer = NULL;
    DDS::DataWriterQos mergedQos;
    const DDS::DataWriterQos *writerQosPtr;
    DDS::OpenSplice::TypeSupportMetaHolder *tsMetaHolder = NULL;
    DDS::OpenSplice::Topic *topic;
    char *name = NULL;

    CPP_REPORT_STACK();

    topic = dynamic_cast<DDS::OpenSplice::Topic *>(a_topic);
    if (!topic) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_topic is invalid, not of type '%s'.",
            "DDS::OpenSplice::Topic");
    }

    if (result == DDS::RETCODE_OK) {
        name = this->create_datawriter_name (topic);
        if (!name) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
        }
    }
    if (result == DDS::RETCODE_OK) {
        tsMetaHolder = topic->get_typesupport_meta_holder();
        if (tsMetaHolder) {
            writer = tsMetaHolder->create_datawriter();
            if (!writer) {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = DDS::RETCODE_BAD_PARAMETER;
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
        if (result == DDS::RETCODE_OK) {
            if (&qos == &DATAWRITER_QOS_DEFAULT) {
                writerQosPtr = &this->defaultWriterQos;
            } else if (&qos == &DATAWRITER_QOS_USE_TOPIC_QOS) {
                DDS::TopicQos topicQos;

                writerQosPtr = &mergedQos;
                mergedQos = this->defaultWriterQos;
                result = topic->get_qos (topicQos);
                if (result == DDS::RETCODE_OK) {
                    result = this->copy_from_topic_qos(mergedQos, topicQos);
                    if (result == DDS::RETCODE_OK) {
                        result = DDS::OpenSplice::Utils::qosIsConsistent(mergedQos);
                    }
                }

            } else {
                writerQosPtr = &qos;
                result = DDS::OpenSplice::Utils::qosIsConsistent (qos);
            }
            if (result == DDS::RETCODE_OK) {
                result = writer->init (
                        this, this->participant, *writerQosPtr, topic, name,
                        tsMetaHolder->get_copy_in(), tsMetaHolder->get_copy_out());
                if (result == DDS::RETCODE_OK) {
                    /* Duplicate and store the created DataWriter object. */
                    if (!this->wlReq_insertWriter (writer)) {
                        result = DDS::RETCODE_OUT_OF_RESOURCES;
                    } else {
                        writer->wlReq_set_listenerDispatcher(this->rlReq_get_listenerDispatcher());
                        result = writer->set_listener(a_listener, mask);
                        if (result == DDS::RETCODE_OK && this->factoryAutoEnable) {
                            result = writer->enable ();
                        }
                        /* Remove DataWriter from list if an error occurred */
                        if (result != DDS::RETCODE_OK) {
                            (void)writer->set_listener(NULL, 0);
                            writer->wlReq_set_listenerDispatcher(NULL);
                            (void)this->wlReq_removeWriter (writer);
                        }
                    }
                    if (result != DDS::RETCODE_OK) {
                        /* After a failure: deinit() it and destroy writer. */
                        (void)writer->deinit ();
                    }
                }
            }
            this->unlock ();
        }
    }

    if (name) {
        DDS::string_free (name);
    }
    if (tsMetaHolder) {
        DDS::release(tsMetaHolder);
    }

    if (result != DDS::RETCODE_OK && writer) {
        DDS::release (writer);
        writer = NULL;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return writer;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::delete_datawriter (
    DDS::DataWriter_ptr a_datawriter
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::DataWriter *writer;

    CPP_REPORT_STACK();

    if (a_datawriter == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_datawriter '<NULL>' is invalid.");
    } else {
        writer = dynamic_cast<DDS::OpenSplice::DataWriter *>(a_datawriter);
        if (writer == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_datawriter is invalid, not of type '%s'.",
                "DDS::OpenSplice::DataWriter");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        if (wlReq_removeWriter(writer) == FALSE) {
            if (writer->get_kind() == DDS::OpenSplice::DATAWRITER) {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
            } else {
                result = DDS::RETCODE_BAD_PARAMETER;
            }
            CPP_REPORT(result, "DataWriter not created by Publisher.");
        }

        if (result == DDS::RETCODE_OK) {
            /* Destroy the Writer. */
            (void)writer->set_listener(NULL, 0);
            result = writer->deinit ();
            if (result != DDS::RETCODE_OK) {
                if (result == DDS::RETCODE_PRECONDITION_NOT_MET) {
                    /* Re-store when the deinit didn't work. */
                    DDS::Boolean reinserted = wlReq_insertWriter(writer);
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
DDS::OpenSplice::Publisher::delete_contained_entities ()
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        result = wlReq_deleteEntityList<DDS::OpenSplice::DataWriter *>(writers);
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::Boolean
DDS::OpenSplice::Publisher::rlReq_lookupByTopic (
    DDS::Object_ptr element,
    lookupByTopicArg *arg)
{
    DDS::OpenSplice::DataWriter *writer;
    DDS::OpenSplice::Topic *topic;
    const char *topicName;
    DDS::Boolean result = TRUE;

    assert (element);
    assert (arg);

    writer = dynamic_cast<DDS::OpenSplice::DataWriter *>(element);
    topic = writer->topic;
    topicName = topic->topic_name.in();
    if (strcmp(topicName, arg->topicName) == 0) {
        arg->writer = writer;
        result = FALSE;
    }

    return result;
}

DDS::DataWriter_ptr
DDS::OpenSplice::Publisher::lookup_datawriter (
    const char *topic_name
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    lookupByTopicArg arg;

    CPP_REPORT_STACK();

    arg.writer = NULL;
    arg.topicName = topic_name;

    if (topic_name != NULL) {
        result = this->read_lock ();
        if (result == DDS::RETCODE_OK) {
            (void)writers->walk((DDS::OpenSplice::ObjSet::ObjSetActionFunc) rlReq_lookupByTopic, &arg);
            this->unlock ();
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return DDS::DataWriter::_duplicate(arg.writer);
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::set_qos (
    const DDS::PublisherQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::PublisherQos publisherQos;
    DDS::PublisherQos *publisherQos_ptr;
    u_publisher uPublisher;
    u_publisherQos uPublisherQos;
    u_result uResult;

    CPP_REPORT_STACK();

    if (&qos == &PUBLISHER_QOS_DEFAULT) {
        publisherQos_ptr = NULL;
        result = DDS::RETCODE_OK;
    } else {
        publisherQos_ptr = const_cast<DDS::PublisherQos *>(&qos);
        result = DDS::OpenSplice::Utils::qosIsConsistent (qos);
    }

    if (result == DDS::RETCODE_OK) {
        uPublisherQos = u_publisherQosNew (NULL);
        if (uPublisherQos == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not copy PublisherQos.");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
        if (result == DDS::RETCODE_OK) {
            if (publisherQos_ptr == NULL) {
                result = participant->get_default_publisher_qos (publisherQos);
                publisherQos_ptr = &publisherQos;
            }

            if (result == DDS::RETCODE_OK) {
                result = DDS::OpenSplice::Utils::copyQosIn (
                    *publisherQos_ptr, uPublisherQos);
                if (result == DDS::RETCODE_OK) {
                    uPublisher = u_publisher (rlReq_get_user_entity ());
                    uResult = u_publisherSetQos (uPublisher, uPublisherQos);
                    result = uResultToReturnCode (uResult);
                    if (result == DDS::RETCODE_OK) {
                        factoryAutoEnable =
                            publisherQos_ptr->entity_factory.autoenable_created_entities;
                    } else {
                        CPP_REPORT(result, "Could not apply PublisherQos.");
                    }
                }
            }
            this->unlock ();
        }

        u_publisherQosFree (uPublisherQos);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::get_qos (
    DDS::PublisherQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_publisher uPublisher;
    u_publisherQos uPublisherQos;
    u_result uResult;

    CPP_REPORT_STACK();

    if (&qos == &PUBLISHER_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'PUBLISHER_QOS_DEFAULT' is read-only.");
    } else {
        result = this->check ();
    }

    if (result == DDS::RETCODE_OK) {
        uPublisher = u_publisher (rlReq_get_user_entity ());
        uResult = u_publisherGetQos (uPublisher, &uPublisherQos);
        result = uResultToReturnCode (uResult);
        if (result == DDS::RETCODE_OK) {
            result = DDS::OpenSplice::Utils::copyQosOut (uPublisherQos, qos);
            u_publisherQosFree(uPublisherQos);
        } else {
            CPP_REPORT(result, "Could not copy PublisherQos.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::set_listener (
    DDS::PublisherListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = DDS::OpenSplice::Entity::nlReq_set_listener(a_listener, mask);
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::PublisherListener_ptr
DDS::OpenSplice::Publisher::get_listener (
) THROW_ORB_EXCEPTIONS
{
    DDS::PublisherListener_ptr listener;

    CPP_REPORT_STACK();
    listener = dynamic_cast<DDS::PublisherListener_ptr>(
        DDS::OpenSplice::Entity::nlReq_get_listener());
    CPP_REPORT_FLUSH(this, listener == NULL);

    return listener;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::suspend_publications (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_publisher uPublisher;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uPublisher = u_publisher (rlReq_get_user_entity ());
        uResult = u_publisherSuspend (uPublisher);
        result = uResultToReturnCode (uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::resume_publications (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_publisher uPublisher;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uPublisher = u_publisher (rlReq_get_user_entity ());
        uResult = u_publisherResume (uPublisher);
        result = uResultToReturnCode (uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::begin_coherent_changes (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_publisher uPublisher;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uPublisher = u_publisher (rlReq_get_user_entity ());
        uResult = u_publisherCoherentBegin (uPublisher);
        result = uResultToReturnCode (uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::end_coherent_changes (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_publisher uPublisher;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uPublisher = u_publisher (rlReq_get_user_entity ());
        uResult = u_publisherCoherentEnd (uPublisher);
        result = uResultToReturnCode (uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::wait_for_acknowledgments (
    const Duration_t &max_wait
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(max_wait);
    return DDS::RETCODE_UNSUPPORTED;
}

DDS::DomainParticipant_ptr
DDS::OpenSplice::Publisher::get_participant (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::DomainParticipant_ptr participantPtr = NULL;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        participantPtr = DDS::DomainParticipant::_duplicate(participant);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return participantPtr;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::set_default_datawriter_qos (
    const DDS::DataWriterQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    CPP_REPORT_STACK();

    if (&qos == &DATAWRITER_QOS_USE_TOPIC_QOS) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is invalid in this context.");
    } else if (&qos != &DATAWRITER_QOS_DEFAULT) {
        result = DDS::OpenSplice::Utils::qosIsConsistent (qos);
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        this->defaultWriterQos = qos;
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::get_default_datawriter_qos (
    DDS::DataWriterQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    if (&qos == &DATAWRITER_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAWRITER_QOS_DEFAULT' is read-only.");
    } else if (&qos == &DATAWRITER_QOS_USE_TOPIC_QOS) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is read-only.");
    } else {
        result = this->read_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        qos = this->defaultWriterQos;
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Publisher::copy_from_topic_qos (
    DDS::DataWriterQos &a_datawriter_qos,
    const TopicQos &a_topic_qos
) THROW_ORB_EXCEPTIONS
{
    DDS::TopicQos topicQos;
    DDS::TopicQos *topicQos_ptr = NULL;
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::DomainParticipant_ptr participant;

    CPP_REPORT_STACK();

    if (&a_datawriter_qos == &DATAWRITER_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_datawriter_qos 'DATAWRITER_QOS_DEFAULT' is read-only");
    } else if (&a_datawriter_qos == &DATAWRITER_QOS_USE_TOPIC_QOS) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_datawriter_qos 'DATAWRITER_QOS_USE_TOPIC_QOS' is read-only");
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
        a_datawriter_qos.durability = topicQos_ptr->durability;
        a_datawriter_qos.deadline = topicQos_ptr->deadline;
        a_datawriter_qos.latency_budget = topicQos_ptr->latency_budget;
        a_datawriter_qos.liveliness = topicQos_ptr->liveliness;
        a_datawriter_qos.reliability = topicQos_ptr->reliability;
        a_datawriter_qos.destination_order = topicQos_ptr->destination_order;
        a_datawriter_qos.history = topicQos_ptr->history;
        a_datawriter_qos.resource_limits = topicQos_ptr->resource_limits;
        a_datawriter_qos.transport_priority = topicQos_ptr->transport_priority;
        a_datawriter_qos.lifespan = topicQos_ptr->lifespan;
        a_datawriter_qos.ownership = topicQos_ptr->ownership;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

void
DDS::OpenSplice::Publisher::nlReq_notify_listener(
    DDS::OpenSplice::Entity *sourceEntity,
    DDS::ULong               triggerMask,
    void                    *eventData)
{
    DDS::PublisherListener_ptr listener;

    /* Using _narrow to cast Listener, this increases the refcount to ensure
     * the ListenerObject is not deleted while notifying. */
    listener = DDS::PublisherListener::_narrow(this->listener);
    if (listener) {

        if (triggerMask & V_EVENT_LIVELINESS_LOST) {
            DDS::LivelinessLostStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->livelinessLost,
                                                  status);
            listener->on_liveliness_lost(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                         status);
        }

        if (triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
            DDS::OfferedDeadlineMissedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->deadlineMissed,
                                                  status);
            listener->on_offered_deadline_missed(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                                 status);
        }

        if (triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            DDS::OfferedIncompatibleQosStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->incompatibleQos,
                                                  status);
            listener->on_offered_incompatible_qos(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                                  status);
        }

        if (triggerMask & V_EVENT_PUBLICATION_MATCHED) {
            DDS::PublicationMatchedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->publicationMatch,
                                                  status);
            listener->on_publication_matched(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                             status);
        }
        DDS::release(listener);
    }
}
