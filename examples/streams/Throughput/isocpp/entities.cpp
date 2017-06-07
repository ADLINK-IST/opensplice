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

#include "ThroughputStreams_DCPS.hpp"

namespace examples { namespace streams { namespace Throughput { namespace isocpp {

static dds::core::cond::GuardCondition terminated;

/**
 * This class serves as a container holding initialised entities for publishing.
 */
class PubEntities
{
public:
    /**
     * This constructor initialises the entities used for publishing.
     */
    PubEntities(std::string partitionName, dds::core::Duration timeOut, uint32_t maxSamples) : writer(dds::core::null)
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant participant =
            dds::domain::DomainParticipant(org::opensplice::domain::default_id(),
                                           dds::domain::DomainParticipant::default_participant_qos(),
                                           0,
                                           dds::core::status::StatusMask::none());

        /** A dds::pub::Publisher is created on the domain participant. */
        dds::pub::qos::PublisherQos pubQos
            = participant.default_publisher_qos() << dds::core::policy::Partition(partitionName);
        dds::pub::Publisher publisher(participant, pubQos, 0, dds::core::status::StatusMask::none());

        /** A dds::streams::pub::StreamDataWriter is created on the Publisher with a modififed Qos. */
        dds::streams::pub::qos::StreamDataWriterQos sdwqos;
        sdwqos << dds::streams::core::policy::StreamFlush(timeOut, maxSamples);
        writer = dds::streams::pub::StreamDataWriter<ThroughputModule::DataType>(publisher, "StreamsThroughput", sdwqos);
    }

public:
    /** The DataWriter used by the publisher. */
    dds::streams::pub::StreamDataWriter<ThroughputModule::DataType> writer;
};

/**
 * This class serves as a container holding initialised entities used by the subscriber.
 */
class SubEntities
{
public:
    /**
     * This constructor initialises the entities used by the subscriber.
     */
    SubEntities(std::string partitionName) : reader(dds::core::null)
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant participant =
            dds::domain::DomainParticipant(org::opensplice::domain::default_id(),
                                           dds::domain::DomainParticipant::default_participant_qos(),
                                           0,
                                           dds::core::status::StatusMask::none());

        /** A dds::sub::Subscriber is created on the domain participant. */
        dds::sub::qos::SubscriberQos subQos
            = participant.default_subscriber_qos() << dds::core::policy::Partition(partitionName);
        dds::sub::Subscriber subscriber(participant, subQos, 0, dds::core::status::StatusMask::none());

        /** A dds::streams::sub::StreamDataReader is created. */
        reader = dds::streams::sub::StreamDataReader<ThroughputModule::DataType>(subscriber, "StreamsThroughput");
    }

public:
    /** The DataReader used by the subscriber. */
    dds::streams::sub::StreamDataReader<ThroughputModule::DataType> reader;
};

}
}
}
}
