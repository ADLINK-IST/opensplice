
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

#include "common/example_error_sacpp.hpp"
#include "ThroughputStreamsApi.h"

#include <iostream>

namespace examples { namespace streams { namespace Throughput { namespace sacpp {

static DDS::GuardCondition_var terminated = new DDS::GuardCondition();
static ThroughputModule::DataTypeStreamDataReader_ptr reader = NULL;

/**
 * This class serves as a container holding initialised entities used for publishing
 */
class PubEntities
{
public:
    /**
     * This constructor initialises the entities used for publishing
     */
    PubEntities(const DDS::String_var& partitionName, DDS::Duration_t& timeOut, unsigned long maxSamples)
    {
        DDS::ReturnCode_t status;
        try
        {
            /** A DDS::DomainParticipant is created for the default domain. */
            domainParticipantFactory = DDS::DomainParticipantFactory::get_instance();
            CHECK_HANDLE_MACRO(domainParticipantFactory);
            participant = domainParticipantFactory->create_participant(
                DDS::DOMAIN_ID_DEFAULT, PARTICIPANT_QOS_DEFAULT, 0, DDS::STATUS_MASK_NONE);
            CHECK_HANDLE_MACRO(participant);

            /** A DDS::Publisher is created on the domain participant. */
            DDS::PublisherQos pubQos;
            status = participant->get_default_publisher_qos(pubQos);
            CHECK_STATUS_MACRO(status);
            pubQos.partition.name.length(1);
            pubQos.partition.name[0] = DDS::string_dup(partitionName);
            publisher = participant->create_publisher(pubQos, 0, DDS::STATUS_MASK_NONE);
            CHECK_HANDLE_MACRO(publisher.in());

            /** A DDS::DataWriter is created on the Publisher with a modififed Qos. */
            DDS::Streams::StreamDataWriterQos sdwQos;
            status = ThroughputModule::DataTypeStreamDataWriter::get_default_qos(sdwQos);
            CHECK_STATUS_MACRO(status);
            sdwQos.flush.max_delay = timeOut;
            sdwQos.flush.max_samples = maxSamples;
            writer = new ThroughputModule::DataTypeStreamDataWriter(publisher, sdwQos, "StreamsThroughput");
            CHECK_HANDLE_MACRO(writer.in());
        }
        catch(DDS::Streams::StreamsException e)
        {
            std::cout << e.message << std::endl;
            throw e;
        }
        catch(...)
        {
            DDS::String_var exception = DDS::string_dup(
                "Failed to create DomainParticipant : Is ospl running? start it with the \"ospl start\" command");
            throw exception;
        }
    }

public:
    /** The DomainParticipantFactory used by the publisher */
    DDS::DomainParticipantFactory_var domainParticipantFactory;
    /** The DomainParticipant used by the publisher */
    DDS::DomainParticipant_var participant;
    /** The Publisher used by the publisher */
    DDS::Publisher_var publisher;
    /** The DataWriter used by the publisher */
    ThroughputModule::DataTypeStreamDataWriter_var writer;
};

/**
 * This class serves as a container holding initialised entities used for subscribing
 */
class SubEntities
{
public:
    /**
     * This constructor initialises the entities used for subscribing
     */
    SubEntities(const DDS::String_var& partitionName)
    {
        DDS::ReturnCode_t status;
        try
        {
            /** A DDS::DomainParticipant is created for the default domain. */
            domainParticipantFactory = DDS::DomainParticipantFactory::get_instance();
            CHECK_HANDLE_MACRO(domainParticipantFactory);
            participant = domainParticipantFactory->create_participant(
                DDS::DOMAIN_ID_DEFAULT, PARTICIPANT_QOS_DEFAULT, 0, DDS::STATUS_MASK_NONE);
            CHECK_HANDLE_MACRO(participant);

            /** A DDS::Subscriber is created on the domain participant. */
            DDS::SubscriberQos subQos;
            status = participant->get_default_subscriber_qos(subQos);
            CHECK_STATUS_MACRO(status);
            subQos.partition.name.length(1);
            subQos.partition.name[0] = DDS::string_dup(partitionName);
            subscriber = participant->create_subscriber(subQos, 0, DDS::STATUS_MASK_NONE);
            CHECK_HANDLE_MACRO(subscriber.in());

            /** A DDS::DataReader is created on the Subscriber */
            reader = new ThroughputModule::DataTypeStreamDataReader(subscriber, "StreamsThroughput");
            CHECK_HANDLE_MACRO(reader.in());
        }
        catch(DDS::Streams::StreamsException e)
        {
            std::cout << e.message << std::endl;
            throw e;
        }
        catch(...)
        {
            DDS::String_var exception = DDS::string_dup(
                "Failed to create DomainParticipant : Is ospl running? start it with the \"ospl start\" command");
            throw exception;
        }
    }

public:
    /** The DomainParticipantFactory used by the subscriber */
    DDS::DomainParticipantFactory_var domainParticipantFactory;
    /** The DomainParticipant used by the subscriber */
    DDS::DomainParticipant_var participant;
    /* The Subscriber used by the subscriber */
    DDS::Subscriber_var subscriber;
    /** The DataReader used by the subscriber */
    ThroughputModule::DataTypeStreamDataReader_var reader;
};

}
}
}
}
