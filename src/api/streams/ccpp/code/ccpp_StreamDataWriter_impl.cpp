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
#include "ccpp_StreamDataWriter_impl.h"

static DDS::Streams::LocalFactoryMutex classLock;

DDS::Streams::StreamDataWriterQos DDS::Streams::DefaultStreamDataWriterQos = {
    { DDS::DURATION_INFINITE, 64UL }
};

DDS::DomainParticipant_var DDS::Streams::StreamDataWriter_impl::participant(NULL);
os_uint32 DDS::Streams::StreamDataWriter_impl::nrParticipantUsers(0);

DDS::Streams::StreamDataWriter_impl::StreamDataWriter_impl(
    DDS::Publisher_ptr publisher_in,
    DDS::DomainId_t domainId,
    DDS::Streams::StreamDataWriterQos &qos_in,
    DDS::TypeSupport_ptr typeSupport,
    const char *streamName)
{
    DDS::DomainParticipant_var myParticipant;
    DDS::Publisher_var myPublisher;
    DDS::String_var typeName;
    DDS::ReturnCode_t result;

    assert(typeSupport);
    assert(streamName);
    assert(domainId);

    if(!publisher_in) {
        os_mutexLock(&classLock.lfMutex);
        if (this->nrParticipantUsers == 0) {
            myParticipant = DDS::DomainParticipantFactory::get_instance()->create_participant(domainId, PARTICIPANT_QOS_DEFAULT, NULL, 0);
            if (myParticipant) {
                this->participant = myParticipant;
                nrParticipantUsers = 1;
            } else {
                OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter_impl", 0, "Failed to create internal participant");
                os_mutexUnlock(&classLock.lfMutex);
                throw StreamsException("Failed to create internal participant. Is the domain running?", DDS::RETCODE_PRECONDITION_NOT_MET);
            }
        } else {
            myParticipant = this->participant;
            nrParticipantUsers++;
        }
        myPublisher = myParticipant->create_publisher(DDS::Streams::default_publisher_qos, NULL, 0);
        if (myPublisher) {
            publisher = myPublisher;
        } else {
            OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter_impl", 0, "Failed to create internal publisher");
            os_mutexUnlock(&classLock.lfMutex);
            throw StreamsException("Failed to create internal publisher", DDS::RETCODE_PRECONDITION_NOT_MET);
        }
        os_mutexUnlock(&classLock.lfMutex);
    } else {
        publisher = NULL;
        myParticipant = publisher_in->get_participant();
        if (myParticipant == NULL) {
            OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter_impl", 0, "Failed to get participant");
            throw StreamsException("Failed to get participant. Is the domain running?", DDS::RETCODE_PRECONDITION_NOT_MET);
        }
    }

    typeName = typeSupport->get_type_name();
    result = typeSupport->register_type(myParticipant, typeName);
    /* TypeSupport is released here because caller is the parent constructor which cannot release it in the member init list */
    release(typeSupport);
    if (result != DDS::RETCODE_OK) {
        OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter_impl", 0, "Failed to register streams topic for type %s", typeName.in());
        throw StreamsException("Failed to register streams topic", result);
    }

    topic = myParticipant->create_topic(streamName, typeName, DDS::Streams::default_topic_qos, NULL, 0);
    if (!topic.in()) {
        OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter_impl", 0, "Failed to create topic %s", streamName);
        throw StreamsException("Failed to create streams topic", result);
    }

    /* Check if QoS is valid */
    if ((qos_in.flush.max_delay.sec < 0) ||
        ((qos_in.flush.max_delay.sec == 0) &&
        (qos_in.flush.max_delay.nanosec == 0))) {
        throw StreamsException("Invalid QoS: auto-flush delay is not a valid duration", DDS::RETCODE_BAD_PARAMETER);
    } else if ((qos_in.flush.max_samples <= 0) ||
            (qos_in.flush.max_samples == DDS::LENGTH_UNLIMITED)) {
        throw StreamsException("Invalid QoS: auto-flush sample limit must be a positive number and cannot be DDS::LENGTH_UNLIMITED", DDS::RETCODE_BAD_PARAMETER);
    } else {
        qos = qos_in;
    }
}

DDS::Streams::StreamDataWriter_impl::~StreamDataWriter_impl() {
    DDS::DomainParticipant_var tmp;

    os_mutexLock(&classLock.lfMutex);

    if (topic.in()) {
        tmp = topic->get_participant();
        /* participant may have been deleted */
        if (tmp != NULL) {
            tmp->delete_topic(topic);
        }
    }

    if (publisher.in()) {
        DDS::ReturnCode_t result;
        participant->delete_publisher(publisher);
        if (--nrParticipantUsers == 0) {
            (void)DDS::DomainParticipantFactory::get_instance()->delete_participant(participant.in());
        }
    }

    os_mutexUnlock(&classLock.lfMutex);
}

::DDS::ReturnCode_t
DDS::Streams::StreamDataWriter_impl::get_qos(
    ::DDS::Streams::StreamDataWriterQos &qos_out) THROW_ORB_EXCEPTIONS {
    qos_out = qos;
    return DDS::RETCODE_OK;
}

::DDS::ReturnCode_t
DDS::Streams::StreamDataWriter_impl::get_default_qos(
    DDS::Streams::StreamDataWriterQos & qos) THROW_ORB_EXCEPTIONS
{
    qos = DefaultStreamDataWriterQos;
    return DDS::RETCODE_OK;
}
