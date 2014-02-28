
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

#include "implementation.hpp"
#include "common/example_utilities.h"

#include <iostream>
#include <string>

#include "LifecycleData_DCPS.hpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace Lifecycle {

/**
 * @addtogroup examplesdcpsLifecycleisocpp The ISO C++ DCPS API Lifecycle example
 *
 * This is example demonstrates the changes in Lifecycle states when a DataWriter
 * is created, sends samples or is deleted.
 * @ingroup examplesdcpsisocpp
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp  {

void usagePub()
{
    std::cerr << "*** ERROR" << std::endl;
    std::cerr << "*** usage : publisher <autodispose_flag> <writer_action>" << std::endl;
    std::cerr << "***         . autodispose_flag = false | true" << std::endl;
    std::cerr << "***         . writer_action = dispose | unregister | stoppub" << std::endl;
}

void writeSample(std::string writer_action, dds::pub::DataWriter<LifecycleData::Msg> dw)
{
    /** A sample is created and then written. */
    LifecycleData::Msg msgInstance;
    if(writer_action == "dispose")
    {
        msgInstance.userID() = 1;
        msgInstance.message() = "Lifecycle_1";
        msgInstance.writerStates() = "SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED";
    }
    else if(writer_action == "unregister")
    {
        msgInstance.userID() = 2;
        msgInstance.message() = "Lifecycle_2";
        msgInstance.writerStates() = "SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED";
    }
    else if(writer_action == "stoppub")
    {
        msgInstance.userID() = 3;
        msgInstance.message() = "Lifecycle_3";
        msgInstance.writerStates() = "SAMPLE_SENT -> DATAWRITER_DELETED";
    }

    std::cout << "=== [Publisher] written a message containing :" << std::endl;
    std::cout << "    userID        : " << msgInstance.userID() << std::endl;
    std::cout << "    Message       : \"" << msgInstance.message() << "\"" << std::endl;
    std::cout << "    writerStates  : \"" << msgInstance.writerStates() << "\"" << std::endl;
    dw << msgInstance;
    exampleSleepMilliseconds(500);
    std::cout << "=== [Publisher]  : SAMPLE_SENT" << std::endl;

    /** The sample instance is then either disposed of or unregistered depending on user preference */
    if(writer_action == "dispose")
    {
        dw.dispose_instance(dw.lookup_instance(msgInstance));
    }
    else if(writer_action == "unregister")
    {
        dw.unregister_instance(dw.lookup_instance(msgInstance));
    }
}

/**
 * This function performs the publisher role in this example.
 * @return 0 if a sample is successfully written, 1 otherwise.
 */
int publisher(int argc, char *argv[])
{
    int result = 0;

    if(argc < 3 ||
        ((strcmp(argv[1], "false") != 0) &&
        (strcmp(argv[1], "true") != 0) &&
        (strcmp(argv[2], "dispose") != 0) &&
        (strcmp(argv[2], "unregister") != 0) &&
        (strcmp(argv[2], "stoppub") != 0)))
    {
        usagePub();
        return -1;
    }
    bool autodispose_unregistered_instances = (strcmp(argv[1], "true") == 0);
    std::string writer_action = argv[2];

    try
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());

        /** The Durability::Transient policy is specified as a dds::topic::qos::TopicQos
         * so that even if the subscriber does not join until after the sample is written
         * then the DDS will still retain the sample for it. The Reliability::Reliable
         * policy is also specified to guarantee delivery. */
        dds::topic::qos::TopicQos topicQos
             = dp.default_topic_qos()
                << dds::core::policy::Durability::Transient()
                << dds::core::policy::Reliability::Reliable();

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<LifecycleData::Msg> topic(dp, "LifecycleData_Msg", topicQos);

        /** A dds::pub::Publisher is created on the domain participant. */
        std::string name = "Lifecycle example";
        dds::pub::qos::PublisherQos pubQos
            = dp.default_publisher_qos()
                << dds::core::policy::Partition(name);
        dds::pub::Publisher pub(dp, pubQos);

        /** The dds::pub::qos::DataWriterQos is derived from the topic qos and autodispose is
         * set to either manual or automatic based on the users preference. This is so the publisher
         * can optionally be run (and exit) before the subscriber. It prevents or allows the middleware
         * default 'clean up' of the topic instance after the writer deletion, this deletion
         * implicitly performs DataWriter::unregister_instance */
        dds::pub::qos::DataWriterQos dwqos = topic.qos();
        dwqos << dds::core::policy::WriterDataLifecycle(autodispose_unregistered_instances);

        /** A dds::pub::DataWriter to write messages and another to indicate when to stop the example are
         *  created on the Publisher & Topic with the modififed Qos. */
        dds::pub::DataWriter<LifecycleData::Msg> dw(pub, topic, dwqos);
        dds::pub::DataWriter<LifecycleData::Msg> dwStopper(pub, topic, dwqos);
        exampleSleepMilliseconds(500);

        writeSample(writer_action, dw);

        std::cout << "=== [Publisher] waiting 500ms to let the subscriber handle the previous write state ..." << std::endl;
        exampleSleepMilliseconds(500);

        /** The dds::pub::DataWriter is reassigned to a null writer to force deletion */
        dw = dds::pub::DataWriter<LifecycleData::Msg>(dds::core::null);
        exampleSleepMilliseconds(500);

        /** A message is sent to stop the subscriber */
        LifecycleData::Msg msgInstance(4, "Lifecycle_4", "STOPPING_SUBSCRIBER");
        std::cout << "=== [Publisher]   :" << std::endl;
        std::cout << "    userID        : " << msgInstance.userID() << std::endl;
        std::cout << "    Message       : \"" << msgInstance.message() << "\"" << std::endl;
        std::cout << "    writerStates  : \"" << msgInstance.writerStates() << "\"" << std::endl;
        dwStopper << msgInstance;

        /* A sleep ensures time is allowed for the sample to be written to the network.
           If the example is running in *Single Process Mode* exiting immediately might
           otherwise shutdown the domain services before this could occur */
        exampleSleepMilliseconds(1000);
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = 1;
    }
    return result;
}

std::string sampleStateToString(const dds::sub::status::SampleState& state)
{
    std::string stateString = "INVALID_STATE";
    if(state == dds::sub::status::SampleState::read())
    {
        stateString = "READ_SAMPLE_STATE";
    }
    else if(state == dds::sub::status::SampleState::not_read())
    {
        stateString = "NOT_READ_SAMPLE_STATE";
    }
    return stateString;
}

std::string viewStateToString(const dds::sub::status::ViewState& state)
{
    std::string stateString = "INVALID_STATE";
    if(state == dds::sub::status::ViewState::new_view())
    {
        stateString = "NEW_VIEW_STATE";
    }
    else if(state == dds::sub::status::ViewState::not_new_view())
    {
        stateString = "NOT_NEW_VIEW_STATE";
    }
    else if(state == dds::sub::status::ViewState::any())
    {
        stateString = "ANY_VIEW_STATE";
    }
    return stateString;
}

std::string instanceStateToString(const dds::sub::status::InstanceState& state)
{
    std::string stateString = "INVALID_STATE";
    if(state == dds::sub::status::InstanceState::alive())
    {
        stateString = "ALIVE_INSTANCE_STATE";
    }
    else if(state == dds::sub::status::InstanceState::not_alive_disposed())
    {
        stateString = "NOT_ALIVE_DISPOSED_INSTANCE_STATE";
    }
    else if(state == dds::sub::status::InstanceState::not_alive_no_writers())
    {
        stateString = "NOT_ALIVE_NO_WRITERS_INSTANCE_STATE";
    }
    else if(state == dds::sub::status::InstanceState::any())
    {
        stateString = "ANY_INSTANCE_STATE";
    }
    return stateString;
}

/**
 * Runs the subscriber role of this example.
 * @return 0 if a sample is successfully read, 1 otherwise.
 */
int subscriber(int argc, char *argv[])
{
    int result = 0;
    try
    {
        /** A domain participant and topic are created identically as in
         the ::publisher */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
        dds::topic::qos::TopicQos topicQos = dp.default_topic_qos()
                                                    << dds::core::policy::Durability::Transient()
                                                    << dds::core::policy::Reliability::Reliable();
        dds::topic::Topic<LifecycleData::Msg> topic(dp, "LifecycleData_Msg", topicQos);

        /** A dds::sub::Subscriber is created on the domain participant. */
        std::string name = "Lifecycle example";
        dds::sub::qos::SubscriberQos subQos
            = dp.default_subscriber_qos()
                << dds::core::policy::Partition(name);
        dds::sub::Subscriber sub(dp, subQos);

        /** The dds::sub::qos::DataReaderQos are derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos = topic.qos();

        /** A dds::sub::DataReader is created on the Subscriber & Topic with the DataReaderQos. */
        dds::sub::DataReader<LifecycleData::Msg> dr(sub, topic, drqos);

        std::cout << "=== [Subscriber] Ready ..." << std::endl;

        /** An attempt to take samples is made repeatedly until it succeeds,
         * or one hundred iterations have occurred. */
        bool closed = false;
        int count = 0;
        int maxCount = 100;
        do
        {
            dds::sub::LoanedSamples<LifecycleData::Msg> samples = dr.read();
            for (dds::sub::LoanedSamples<LifecycleData::Msg>::const_iterator sample = samples.begin();
                 sample < samples.end();
                 ++sample)
            {
                std::cout << "=== [Subscriber] message received :" << std::endl;
                std::cout << "    userID        : " << sample->data().userID() << std::endl;
                std::cout << "    Message       : \"" << sample->data().message() << "\"" << std::endl;
                std::cout << "    writerStates  : \"" << sample->data().writerStates() << "\"" << std::endl;
                std::cout << "    valid_data    : " << sample->info().valid() << std::endl;
                std::cout << "    sample_state:" << sampleStateToString(sample->info().state().sample_state());
                std::cout << "-view_state:" << viewStateToString(sample->info().state().view_state());
                std::cout << "-instance_state:" << instanceStateToString(sample->info().state().instance_state()) << std::endl;
                exampleSleepMilliseconds(200);
                closed = (sample->data().writerStates() == "STOPPING_SUBSCRIBER");
            }
            exampleSleepMilliseconds(20);
            ++count;
        }
        while (!closed && count < maxCount);

        std::cout << "=== [Subscriber] stopping after " << count << " iterations - closed = " << closed << std::endl;
        if (count == maxCount)
        {
            std::cout << "*** Error : max " << maxCount << " iterations reached" << std::endl;
            result = 1;
        }
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = 1;
    }
    return result;
}

}
}
}
}

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Lifecycle_publisher, examples::dcps::Lifecycle::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Lifecycle_subscriber, examples::dcps::Lifecycle::isocpp::subscriber)
