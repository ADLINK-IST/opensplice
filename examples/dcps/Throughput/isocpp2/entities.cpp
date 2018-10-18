
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

#include "Throughput_DCPS.hpp"

namespace examples { namespace dcps { namespace Throughput { namespace isocpp {

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
    PubEntities(std::string partitionName) : writer(dds::core::null)
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant participant = dds::domain::DomainParticipant(org::opensplice::domain::default_id());

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<ThroughputModule::DataType> topic = dds::topic::Topic<ThroughputModule::DataType>(participant, "Throughput");

        /** A dds::pub::Publisher is created on the domain participant. */
        dds::pub::qos::PublisherQos pubQos
            = participant.default_publisher_qos()
                << dds::core::policy::Partition(partitionName);
        dds::pub::Publisher publisher(participant, pubQos);

        /** A dds::pub::DataWriter is created on the Publisher & Topic with a modififed Qos. */
        dds::pub::qos::DataWriterQos dwqos = topic.qos();
        dwqos << dds::core::policy::Reliability::Reliable(dds::core::Duration(10, 0))
              << dds::core::policy::History::KeepAll()
              << dds::core::policy::ResourceLimits(100);
        writer = dds::pub::DataWriter<ThroughputModule::DataType>(publisher, topic, dwqos);
    }

public:
    /** @cond don't let this entity clutter the API documentation. */
    dds::pub::DataWriter<ThroughputModule::DataType> writer;
    /** @endcond */
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
    SubEntities(std::string partitionName) : reader(dds::core::null), waitSet(dds::core::null)
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant participant = dds::domain::DomainParticipant(org::opensplice::domain::default_id());

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<ThroughputModule::DataType> topic = dds::topic::Topic<ThroughputModule::DataType>(participant, "Throughput");

        /** A dds::sub::Subscriber is created on the domain participant. */
        dds::sub::qos::SubscriberQos subQos
            = participant.default_subscriber_qos()
                << dds::core::policy::Partition(partitionName);
        dds::sub::Subscriber subscriber(participant, subQos);

        /** The dds::sub::qos::DataReaderQos is derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos = topic.qos();
        drqos << dds::core::policy::Reliability::Reliable(dds::core::Duration(10, 0))
              << dds::core::policy::History::KeepAll()
              << dds::core::policy::ResourceLimits(400);

        /** A dds::sub::DataReader is created on the Subscriber & Topic with the DataReaderQos. */
        reader = dds::sub::DataReader<ThroughputModule::DataType>(subscriber, topic, drqos);

        /** A StatusCondition is created which is triggered when data is available to read */
        dds::core::cond::StatusCondition dataAvailable(reader);
        dds::core::status::StatusMask statusMask;
        statusMask << dds::core::status::StatusMask::data_available();
        dataAvailable.enabled_statuses(statusMask);

        /** A WaitSet is created and the data available status condition is attached */
        waitSet = dds::core::cond::WaitSet();
        waitSet += dataAvailable;

        waitSet += terminated;
    }

public:
    /** @cond don't let these entities clutter the API documentation. */
    dds::sub::DataReader<ThroughputModule::DataType> reader;
    dds::core::cond::WaitSet waitSet;
    /** @endcond */
};

}
}
}
}
