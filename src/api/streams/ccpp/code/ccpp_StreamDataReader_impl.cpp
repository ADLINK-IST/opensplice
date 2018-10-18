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

#include "ccpp_StreamDataReader_impl.h"

static DDS::Streams::LocalFactoryMutex classLock;

DDS::Streams::StreamDataReaderQos DDS::Streams::DefaultStreamDataReaderQos = {
    DDS::octSeq()
};

DDS::DomainParticipant_var DDS::Streams::StreamDataReader_impl::participant(NULL);
os_uint32 DDS::Streams::StreamDataReader_impl::nrParticipantUsers(0);

DDS::Streams::StreamDataReader_impl::StreamDataReader_impl(
    DDS::Subscriber_ptr subscriber_in,
    DDS::DomainId_t domainId,
    DDS::Streams::StreamDataReaderQos &qos_in,
    DDS::TypeSupport_ptr typeSupport,
    const char *streamName)
{
    DDS::DomainParticipant_var myParticipant;
    DDS::Subscriber_var mySubscriber;
    DDS::String_var typeName;
    DDS::ReturnCode_t result;

    assert(typeSupport);
    assert(streamName);
    assert(domainId);

    if (!subscriber_in) {
        os_mutexLock(&classLock.lfMutex);
        if (this->nrParticipantUsers == 0) {
            myParticipant = DDS::DomainParticipantFactory::get_instance()->create_participant(domainId, PARTICIPANT_QOS_DEFAULT, NULL, 0);
            if (myParticipant) {
                participant = myParticipant;
                nrParticipantUsers = 1;
            } else {
                OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataReader_impl", 0, "Failed to create internal participant");
                throw StreamsException("Failed to create internal participant. Is the domain running?", DDS::RETCODE_PRECONDITION_NOT_MET);
            }
        } else {
            myParticipant = participant;
            nrParticipantUsers++;
        }
        mySubscriber = myParticipant->create_subscriber(DDS::Streams::default_subscriber_qos, NULL, 0);
        if (mySubscriber) {
            subscriber = mySubscriber;
        } else {
            OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataReader_impl", 0, "Failed to create internal subscriber");
            throw StreamsException("Failed to create internal publisher", DDS::RETCODE_PRECONDITION_NOT_MET);
        }
        os_mutexUnlock(&classLock.lfMutex);
    } else {
        subscriber = NULL;
        myParticipant = subscriber_in->get_participant();
    }

    typeName = typeSupport->get_type_name();
    result = typeSupport->register_type(myParticipant, typeName);
    /* TypeSupport is released here because caller is the parent constructor which cannot release it in the member init list */
    release(typeSupport);
    if (result != DDS::RETCODE_OK) {
        OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataReader_impl", 0, "Failed to register streams topic for type %s", typeName.in());
        throw StreamsException("Failed to register streams type", result);
    }

    topic = myParticipant->create_topic(streamName, typeName, DDS::Streams::default_topic_qos, NULL, 0);
    if (!topic.in()) {
        OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataReader_impl", 0, "Failed to create topic %s", streamName);
        throw StreamsException("Failed to create streams topic", result);
    }
    qos = qos_in;
}

DDS::Streams::StreamDataReader_impl::~StreamDataReader_impl() {
    DDS::DomainParticipant_var tmp;

    os_mutexLock(&classLock.lfMutex);

    if (topic.in()) {
        tmp = topic->get_participant();
        /* participant may have been deleted */
        if (tmp != NULL) {
            tmp->delete_topic(topic);
        }
    }

    if (subscriber.in()) {
        DDS::ReturnCode_t result;
        participant->delete_subscriber(subscriber);
        if (--nrParticipantUsers == 0) {
            (void)DDS::DomainParticipantFactory::get_instance()->delete_participant(participant);
        }
    }

    os_mutexUnlock(&classLock.lfMutex);
}

::DDS::ReturnCode_t
DDS::Streams::StreamDataReader_impl::get_qos(
    ::DDS::Streams::StreamDataReaderQos & qos_out) THROW_ORB_EXCEPTIONS {
    qos_out = this->qos;
    return DDS::RETCODE_OK;
}

::DDS::ReturnCode_t
DDS::Streams::StreamDataReader_impl::get_default_qos(
    DDS::Streams::StreamDataReaderQos & qos) THROW_ORB_EXCEPTIONS
{
    qos = DefaultStreamDataReaderQos;
    return DDS::RETCODE_OK;
}
