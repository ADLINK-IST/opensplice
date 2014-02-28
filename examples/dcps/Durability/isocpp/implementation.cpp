
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
#include <sstream>

#include "DurabilityData_DCPS.hpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace Durability {

/**
 * @addtogroup examplesdcpsDurabilityisocpp The ISO C++ DCPS API Durability example
 *
 * This example demonstrates the two kinds of durability, transient and persistent
 * and allows the user to specify which durability kind is used.
 * @ingroup examplesdcpsisocpp
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp  {

void usagePub()
{
    std::cerr << "*** ERROR" << std::endl;
    std::cerr << "*** usage : publisher <durability_kind> <autodispose_flag>" << std::endl;
    std::cerr << "***         . durability_kind = transient | persistent" << std::endl;
    std::cerr << "***         . autodispose_flag = false | true" << std::endl;
    std::cerr << "***         . automatic_flag = false | true" << std::endl;
}

/**
 * This function performs the publisher role in this example.
 * @return 0 if a sample is successfully written, 1 otherwise.
 */
int publisher(int argc, char *argv[])
{
    int result = 0;

    if(argc < 4 || (strcmp(argv[1], "transient") && strcmp(argv[1], "persistent")) ||
        (strcmp(argv[2], "false") && strcmp(argv[2], "true")) ||
        (strcmp(argv[3], "false") && strcmp(argv[3], "true")))
    {
        usagePub();
        return -1;
    }
    std::string durability_kind(argv[1]);
    bool autodispose_unregistered_instances = (strcmp(argv[2], "true") == 0);
    bool automatic = (strcmp(argv[3], "true") == 0);

    // Wait for the Subscriber
    exampleSleepMilliseconds(2000);

    try
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());

        /** The Durability is specified based on the users preference. The Reliability::Reliable
          * policy is also specified to guarantee delivery. */
        dds::topic::qos::TopicQos topicQos = dp.default_topic_qos() << dds::core::policy::Reliability::Reliable();
        if(durability_kind == "transient")
        {
            topicQos << dds::core::policy::Durability::Transient();
        }
        else if(durability_kind == "persistent")
        {
            topicQos << dds::core::policy::Durability::Persistent();
        }

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<DurabilityData::Msg> topic(dp, "DurabilityData_Msg", topicQos);

        /** A dds::pub::Publisher is created on the domain participant. */
        std::string name = "Durability example";
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

        /** A dds::pub::DataWriter is created on the Publisher & Topic with the modififed Qos. */
        dds::pub::DataWriter<DurabilityData::Msg> dw(pub, topic, dwqos);

        /** Ten samples are created and registered as an instance before being written. */
        std::vector<DurabilityData::Msg> msgInstances;
        std::vector<dds::core::InstanceHandle> userHandle;

        for(int i = 0; i < 10; i++)
        {
            std::stringstream ss;
            ss << i;
            DurabilityData::Msg msg(i, ss.str());
            userHandle.push_back(dw.register_instance(msg));
            msgInstances.push_back(msg);
            std::cout << msg.content() << std::endl;
        }
        dw.write(msgInstances.begin(), msgInstances.end(), userHandle.begin(), userHandle.end());

        if(!automatic)
        {
            char c = 0;
            std::cout << "Enter E to exit" << std::endl;
            while(c != 'E')
            {
                std::cin >> c;
            }
        }
        else
        {
            /* A sleep ensures time is allowed for the sample to be written to the network.
               If the example is running in *Single Process Mode* exiting immediately might
               otherwise shutdown the domain services before this could occur */
            std::cout << "=== sleeping 20s..." << std::endl;
            exampleSleepMilliseconds(20000);
        }

        if(autodispose_unregistered_instances)
        {
            while(!userHandle.empty())
            {
                dw.unregister_instance(userHandle.back());
                userHandle.pop_back();
            }
        }
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = 1;
    }
    return result;
}

void usageSub()
{
    std::cerr << "*** ERROR" << std::endl;
    std::cerr << "*** usage : subscriber <durability_kind>" << std::endl;
    std::cerr << "***         . durability_kind = transient | persistent" << std::endl;
}

/**
 * Runs the subscriber role of this example.
 * @return 0 if a sample is successfully read, 1 otherwise.
 */
int subscriber(int argc, char *argv[])
{
    int result = 0;

    if(argc < 2 || (strcmp(argv[1], "transient") && strcmp(argv[1], "persistent")))
    {
        usageSub();
        return -1;
    }
    std::string durability_kind(argv[1]);

    try
    {
        /** A domain participant and topic are created identically as in
         the ::publisher */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
        dds::topic::qos::TopicQos topicQos = dp.default_topic_qos() << dds::core::policy::Reliability::Reliable();
        if(durability_kind == "transient")
        {
            topicQos << dds::core::policy::Durability::Transient();
        }
        else if(durability_kind == "persistent")
        {
            topicQos << dds::core::policy::Durability::Persistent();
        }

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<DurabilityData::Msg> topic(dp, "DurabilityData_Msg", topicQos);

        /** A dds::sub::Subscriber is created on the domain participant. */
        std::string name = "Durability example";
        dds::sub::qos::SubscriberQos subQos
            = dp.default_subscriber_qos()
                << dds::core::policy::Partition(name);
        dds::sub::Subscriber sub(dp, subQos);

        /** The dds::sub::qos::DataReaderQos are derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos = topic.qos();

        /** A dds::sub::DataReader is created on the Subscriber & Topic with the DataReaderQos. */
        dds::sub::DataReader<DurabilityData::Msg> dr(sub, topic, drqos);
        dds::core::Duration timeout(4, 10000000);
        dr.wait_for_historical_data(timeout);

        std::cout << "=== [Subscriber] Ready ..." << std::endl;

        /** An attempt to take samples is made repeatedly until it succeeds,
         * or twenty seconds have elapsed. */
        bool closed = false;
        do
        {
            dds::sub::LoanedSamples<DurabilityData::Msg> samples = dr.take();
            for (dds::sub::LoanedSamples<DurabilityData::Msg>::const_iterator sample = samples.begin();
                 sample < samples.end();
                 ++sample)
            {
                if (sample->info().valid())
                {
                    std::cout << sample->data().content() << std::endl;

                    if (sample->data().content() == "9")
                    {
                        closed = true;
                    }
                }
            }
        }
        while (!closed);
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

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Durability_publisher, examples::dcps::Durability::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Durability_subscriber, examples::dcps::Durability::isocpp::subscriber)
