
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

#include "OwnershipData_DCPS.hpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace Ownership {

/**
 * @addtogroup examplesdcpsOwnershipisocpp The ISO C++ DCPS API Ownership example
 *
 * This example demonstrates the Ownership QoS. The subscriber will read messages sent
 * by the publishers and display those with the highest ownership strength.
 * @ingroup examplesdcpsisocpp
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp  {

void usagePub()
{
    std::cerr << "*** ERROR" << std::endl;
    std::cerr << "*** usage : publisher <publisher_name> <ownership_strength> <max_iterations> <stop_subscriber_flag>" << std::endl;
    std::cerr << "***         . publisher_name" << std::endl;
    std::cerr << "***         . ownership_strength" << std::endl;
    std::cerr << "***         . max_iterations" << std::endl;
    std::cerr << "***         . stop_subscriber_flag = false | true" << std::endl;
}

/**
 * This function performs the publisher role in this example.
 * @return 0 if a sample is successfully written, 1 otherwise.
 */
int publisher(int argc, char *argv[])
{
    int result = 0;

    if(argc < 5 || (strcmp(argv[4], "false") && strcmp(argv[4], "true")))
    {
        usagePub();
        return -1;
    }
    std::string publisher_name = argv[1];
    int ownership_strength = atoi(argv[2]);
    int max_iterations = atoi(argv[3]);
    bool stop_subscriber_flag = (strcmp(argv[4], "true") == 0);

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
                << dds::core::policy::Reliability::Reliable()
                << dds::core::policy::Ownership::Exclusive();

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<OwnershipData::Stock> topic(dp, "OwnershipStockTracker", topicQos);

        /** A dds::pub::Publisher is created on the domain participant. */
        std::string name = "Ownership example";
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
        dwqos << dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances()
            << dds::core::policy::OwnershipStrength(ownership_strength);

        /** A dds::pub::DataWriter is created on the Publisher & Topic with the modififed Qos. */
        dds::pub::DataWriter<OwnershipData::Stock> dw(pub, topic, dwqos);

        std::cout << "=== [Publisher] Publisher " << publisher_name << " with strength : " << ownership_strength;
        std::cout << " / sending " << max_iterations << " prices ..." << " stop_subscriber_flag=" << argv[4] << std::endl;

        /** Samples are created and then written. */
        OwnershipData::Stock msgInstance("MSFT", 10.0f, publisher_name.c_str(), ownership_strength);
        dds::core::InstanceHandle ih = dw.register_instance(msgInstance);

        for (int i = 0; i < max_iterations; i++)
        {
            dw.write(msgInstance, ih);
            exampleSleepMilliseconds(200);
            msgInstance.price() += 0.5f;
        }
        exampleSleepMilliseconds(2000);

        /** The subscriber is stopped if specified by the user */
        if (stop_subscriber_flag)
        {
            msgInstance.price() = -1.0f;
            std::cout << "=== Stopping the subscriber" << std::endl;
            dw.write(msgInstance, ih);
        }

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
                                                    << dds::core::policy::Reliability::Reliable()
                                                    << dds::core::policy::Ownership::Exclusive();
        dds::topic::Topic<OwnershipData::Stock> topic(dp, "OwnershipStockTracker", topicQos);

        /** A dds::sub::Subscriber is created on the domain participant. */
        std::string name = "Ownership example";
        dds::sub::qos::SubscriberQos subQos
            = dp.default_subscriber_qos()
                << dds::core::policy::Partition(name);
        dds::sub::Subscriber sub(dp, subQos);

        /** The dds::sub::qos::DataReaderQos are derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos = topic.qos();

        /** A dds::sub::DataReader is created on the Subscriber & Topic with the DataReaderQos. */
        dds::sub::DataReader<OwnershipData::Stock> dr(sub, topic, drqos);

        std::cout << "===[Subscriber] Ready ..." << std::endl;
        std::cout << "   Ticker   Price   Publisher   ownership strength" << std::endl;

        /** The subscriber will read messages sent by the publishers and display those with the
         * highest ownership strength. */
        bool closed = false;
        int count = 0;
        do
        {
            dds::sub::LoanedSamples<OwnershipData::Stock> samples = dr.take();
            for (dds::sub::LoanedSamples<OwnershipData::Stock>::const_iterator sample = samples.begin();
                 sample < samples.end();
                 ++sample)
            {
                if (sample->info().valid())
                {
                    if(sample->data().price() < -0.0f)
                    {
                        closed = true;
                        break;
                    }
                    std::cout << "   " << sample->data().ticker() << "     " << sample->data().price() << "   " << sample->data().publisher() << "   " << sample->data().strength() << std::endl;
                }
            }
            exampleSleepMilliseconds(200);
            ++count;
        }
        while (!closed && count < 1500);

        std::cout << "===[Subscriber] Market Closed" << std::endl;
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = -1;
    }
    return result;
}

}
}
}
}

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Ownership_publisher, examples::dcps::Ownership::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Ownership_subscriber, examples::dcps::Ownership::isocpp::subscriber)
