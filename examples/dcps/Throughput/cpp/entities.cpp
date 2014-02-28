
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

#include "common/example_error_sacpp.hpp"
#include "ccpp_ThroughputData.h"

namespace examples { namespace dcps { namespace Throughput { namespace sacpp {

static DDS::GuardCondition_var terminated = new DDS::GuardCondition();

/**
 * This class serves as a container holding initialised entities used for publishing
 */
class PubEntities
{
public:
    /**
     * This constructor initialises the entities used for publishing
     */
    PubEntities(const char *partitionName)
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
        }
        catch(...)
        {
            DDS::String_var exception = DDS::string_dup(
                "Failed to create DomainParticipant : Is ospl running? start it with the \"ospl start\" command");
            throw exception;
        }

        /** The sample type is created and registered */
        typeSupport = new ThroughputModule::SampleTypeSupport();
        status = typeSupport.in()->register_type(participant.in(), typeSupport.in()->get_type_name());
        CHECK_STATUS_MACRO(status);

        /** A DDS::Topic is created for our sample type on the domain participant. */
        topic = participant.in()->create_topic(
            "Throughput", typeSupport.in()->get_type_name(), TOPIC_QOS_DEFAULT, 0, DDS::STATUS_MASK_NONE);
        CHECK_HANDLE_MACRO(topic.in());

        /** A DDS::Publisher is created on the domain participant. */
        DDS::PublisherQos pubQos;
        status = participant->get_default_publisher_qos(pubQos);
        CHECK_STATUS_MACRO(status);
        pubQos.partition.name.length(1);
        pubQos.partition.name[0] = DDS::string_dup(partitionName);
        publisher = participant->create_publisher(pubQos, 0, DDS::STATUS_MASK_NONE);
        CHECK_HANDLE_MACRO(publisher.in());

        /** A DDS::DataWriter is created on the Publisher & Topic with a modififed Qos. */
        DDS::DataWriterQos dwQos;
        status = publisher->get_default_datawriter_qos(dwQos);
        CHECK_STATUS_MACRO(status);
        dwQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
        dwQos.reliability.max_blocking_time.sec = 10;
        dwQos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
        dwQos.resource_limits.max_samples = 100;
        DDS::DataWriter_var tmpWriter = publisher->create_datawriter(topic.in(), dwQos, 0, DDS::STATUS_MASK_NONE);
        writer = ThroughputModule::SampleDataWriter::_narrow(tmpWriter.in());
        CHECK_HANDLE_MACRO(writer.in());
    }

    /**
     * This destructor cleans up after the application has finished running
     */
    ~PubEntities()
    {
        DDS::ReturnCode_t status;
        status = participant->delete_contained_entities();
        CHECK_STATUS_MACRO(status);
        status = domainParticipantFactory->delete_participant(participant);
        CHECK_STATUS_MACRO(status);
    }

public:
    /** The DomainParticipantFactory used by the publisher */
    DDS::DomainParticipantFactory_var domainParticipantFactory;
    /** The DomainParticipant used by the publisher */
    DDS::DomainParticipant_var participant;
    /** The TypeSupport for the sample */
    ThroughputModule::SampleTypeSupport_var typeSupport;
    /** The Topic used by the publisher */
    DDS::Topic_var topic;
    /** The Publisher used by the publisher */
    DDS::Publisher_var publisher;
    /** The DataWriter used by the publisher */
    ThroughputModule::SampleDataWriter_var writer;
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
    SubEntities(const char *partitionName)
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
        }
        catch(...)
        {
            DDS::String_var exception = DDS::string_dup(
                "Failed to create DomainParticipant : Is ospl running? start it with the \"ospl start\" command");
            throw exception;
        }

        /** The sample type is created and registered */
        typeSupport = new ThroughputModule::SampleTypeSupport();
        status = typeSupport.in()->register_type(participant.in(), typeSupport.in()->get_type_name());
        CHECK_STATUS_MACRO(status);

        /** A DDS::Topic is created for our sample type on the domain participant. */
        topic = participant.in()->create_topic(
            "Throughput", typeSupport.in()->get_type_name(), TOPIC_QOS_DEFAULT, 0, DDS::STATUS_MASK_NONE);
        CHECK_HANDLE_MACRO(topic.in());

        /** A DDS::Subscriber is created on the domain participant. */
        DDS::SubscriberQos subQos;
        status = participant->get_default_subscriber_qos(subQos);
        CHECK_STATUS_MACRO(status);
        subQos.partition.name.length(1);
        subQos.partition.name[0] = DDS::string_dup(partitionName);
        subscriber = participant->create_subscriber(subQos, 0, DDS::STATUS_MASK_NONE);
        CHECK_HANDLE_MACRO(subscriber.in());

        /** A DDS::DataReader is created on the Subscriber & Topic with a modififed Qos. */
        DDS::DataReaderQos drQos;
        status = subscriber->get_default_datareader_qos(drQos);
        CHECK_STATUS_MACRO(status);
        drQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
        drQos.reliability.max_blocking_time.sec = 10;
        drQos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
        drQos.resource_limits.max_samples = 400;
        DDS::DataReader_var tmpReader = subscriber->create_datareader(topic.in(), drQos, 0, DDS::STATUS_MASK_NONE);
        reader = ThroughputModule::SampleDataReader::_narrow(tmpReader.in());
        CHECK_HANDLE_MACRO(reader.in());

        /** A DDS::StatusCondition is created which is triggered when data is available to read */
        dataAvailable = reader->get_statuscondition();
        CHECK_HANDLE_MACRO(dataAvailable.in());
        status = dataAvailable->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS);
        CHECK_STATUS_MACRO(status);

        /** A DDS::WaitSet is created and the data available status condition is attached */
        waitSet = new DDS::WaitSet();
        status = waitSet->attach_condition(dataAvailable.in());
        CHECK_STATUS_MACRO(status);

        status = waitSet->attach_condition(terminated.in());
        CHECK_STATUS_MACRO(status);
    }

    /**
     * This destructor cleans up after the application has finished running
     */
    ~SubEntities()
    {
        DDS::ReturnCode_t status;
        status = waitSet->detach_condition(dataAvailable.in());
        CHECK_STATUS_MACRO(status);
        status = waitSet->detach_condition(terminated.in());
        CHECK_STATUS_MACRO(status);
        status = participant->delete_contained_entities();
        CHECK_STATUS_MACRO(status);
        status = domainParticipantFactory->delete_participant(participant);
        CHECK_STATUS_MACRO(status);
    }

public:
    /** The DomainParticipantFactory used by the subscriber */
    DDS::DomainParticipantFactory_var domainParticipantFactory;
    /** The DomainParticipant used by the subscriber */
    DDS::DomainParticipant_var participant;
    /** The TypeSupport for the sample */
    ThroughputModule::SampleTypeSupport_var typeSupport;
    /** The Topic used by the subscriber */
    DDS::Topic_var topic;
    DDS::Subscriber_var subscriber;
    /** The DataReader used by the subscriber */
    ThroughputModule::SampleDataReader_var reader;
    /** The WaitSet used by the subscriber */
    DDS::WaitSet_var waitSet;
    /** The StatusCondition used by the subscriber,
     * triggered when data is available to read */
    DDS::StatusCondition_var dataAvailable;
};

}
}
}
}
