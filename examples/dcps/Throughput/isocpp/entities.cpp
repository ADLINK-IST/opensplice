
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

#include "ThroughputData_DCPS.hpp"

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
        dds::topic::Topic<ThroughputModule::Sample> topic = dds::topic::Topic<ThroughputModule::Sample>(participant, "Throughput");

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
        writer = dds::pub::DataWriter<ThroughputModule::Sample>(publisher, topic, dwqos);
    }

public:
    /** The DataWriter used by the publisher. */
    dds::pub::DataWriter<ThroughputModule::Sample> writer;
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
        dds::topic::Topic<ThroughputModule::Sample> topic = dds::topic::Topic<ThroughputModule::Sample>(participant, "Throughput");

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
        reader = dds::sub::DataReader<ThroughputModule::Sample>(subscriber, topic, drqos);

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
    /** The DataReader used by the subscriber. */
    dds::sub::DataReader<ThroughputModule::Sample> reader;
    /** The WaitSet used by the subscriber. */
    dds::core::cond::WaitSet waitSet;
};

}
}
}
}
