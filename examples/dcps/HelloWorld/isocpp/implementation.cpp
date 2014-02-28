
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

#include "HelloWorldData_DCPS.hpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace HelloWorld {

/**
 * @addtogroup examplesdcpsHelloWorldisocpp The ISO C++ DCPS API HelloWorld example
 *
 * This is a simple publisher and subscriber example. One sample is
 * published, the sample is then read.
 * Some non-default QoS are used to guarantee delivery and to allow
 * the publisher to be optionally run before the subscriber.
 * @ingroup examplesdcpsisocpp
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp  {

/**
 * This function performs the publisher role in this example.
 * @return 0 if a sample is successfully written, 1 otherwise.
 */
int publisher(int argc, char *argv[])
{
    int result = 0;
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
        dds::topic::Topic<HelloWorldData::Msg> topic(dp, "HelloWorldData_Msg", topicQos);

        /** A dds::pub::Publisher is created on the domain participant. */
        std::string name = "HelloWorld example";
        dds::pub::qos::PublisherQos pubQos
            = dp.default_publisher_qos()
                << dds::core::policy::Partition(name);
        dds::pub::Publisher pub(dp, pubQos);

        /** The dds::pub::qos::DataWriterQos is derived from the topic qos and the
         * WriterDataLifecycle::ManuallyDisposeUnregisteredInstances policy is
         * specified as an addition. This is so the publisher can optionally be run (and
         * exit) before the subscriber. It prevents the middleware default 'clean up' of
         * the topic instance after the writer deletion, this deletion implicitly performs
         * DataWriter::unregister_instance */
        dds::pub::qos::DataWriterQos dwqos = topic.qos();
        dwqos << dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances();

        /** A dds::pub::DataWriter is created on the Publisher & Topic with the modififed Qos. */
        dds::pub::DataWriter<HelloWorldData::Msg> dw(pub, topic, dwqos);

        /** A sample is created and then written. */
        HelloWorldData::Msg msgInstance(1, "Hello World");
        dw << msgInstance;

        std::cout << "=== [Publisher] written a message containing :" << std::endl;
        std::cout << "    userID  : " << msgInstance.userID() << std::endl;
        std::cout << "    Message : \"" << msgInstance.message() << "\"" << std::endl;

        /* A short sleep ensures time is allowed for the sample to be written to the network.
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
        dds::topic::Topic<HelloWorldData::Msg> topic(dp, "HelloWorldData_Msg", topicQos);

        /** A dds::sub::Subscriber is created on the domain participant. */
        std::string name = "HelloWorld example";
        dds::sub::qos::SubscriberQos subQos
            = dp.default_subscriber_qos()
                << dds::core::policy::Partition(name);
        dds::sub::Subscriber sub(dp, subQos);

        /** The dds::sub::qos::DataReaderQos are derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos = topic.qos();

        /** A dds::sub::DataReader is created on the Subscriber & Topic with the DataReaderQos. */
        dds::sub::DataReader<HelloWorldData::Msg> dr(sub, topic, drqos);

        /** An attempt to take samples is made repeatedly until it succeeds,
         * or sixty seconds have elapsed. */
        bool sampleReceived = false;
        int count = 0;
        do
        {
            dds::sub::LoanedSamples<HelloWorldData::Msg> samples = dr.take();
            for (dds::sub::LoanedSamples<HelloWorldData::Msg>::const_iterator sample = samples.begin();
                 sample < samples.end();
                 ++sample)
            {
                if (sample->info().valid())
                {
                    std::cout << "=== [Subscriber] message received :" << std::endl;
                    std::cout << "    userID  : " << sample->data().userID() << std::endl;
                    std::cout << "    Message : \"" << sample->data().message() << "\"" << std::endl;
                    sampleReceived = true;
                }
            }
            exampleSleepMilliseconds(100);
            ++count;
        }
        while (!sampleReceived && count < 600);

        if (!sampleReceived)
        {
            std::cerr << "ERROR: Waited for 60 seconds but no sample received" << std::endl;
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

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_HelloWorld_publisher, examples::dcps::HelloWorld::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_HelloWorld_subscriber, examples::dcps::HelloWorld::isocpp::subscriber)
