
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

#include "ExampleDataReaderListener.hpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace Listener {

/**
 * @addtogroup examplesdcpsListenerisocpp The ISO C++ DCPS API Listener example
 *
 * In this example a listener is registered on the datareader of the subscriber
 * which listens for the receipt of a message from the publisher and handles it.
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
         * policy is also specified to guarantee delivery and a policy::Deadline() of 1 second is
         * used to trigger the ExampleDataReaderListener::on_request_deadline_missed method
         * after no message  has been received for that duration. */
        dds::topic::qos::TopicQos topicQos
             = dp.default_topic_qos()
                << dds::core::policy::Durability::Transient()
                << dds::core::policy::Reliability::Reliable()
                << dds::core::policy::Deadline(dds::core::Duration(1, 0));

        /** These tailored QoS are made the new participant default */
        dp.default_topic_qos(topicQos);

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<ListenerData::Msg> topic(dp, "ListenerData_Msg", topicQos);

        /** A dds::pub::Publisher is created on the domain participant. */
        std::string name = "Listener example";
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
        dds::pub::DataWriter<ListenerData::Msg> dw(pub, topic, dwqos);

        /** A sample is created on the stack and then written. */
        ListenerData::Msg msgInstance(1, "Hello World");
        dw << msgInstance;

        std::cout << "=== [Publisher] written a message containing :" << std::endl;
        std::cout << "    userID  : " << msgInstance.userID() << std::endl;
        std::cout << "    Message : \"" << msgInstance.message() << "\"" << std::endl;

        /* A short sleep ensures time is allowed for the sample to be written to the network.
        If the example is running in *Single Process Mode* exiting immediately might
        otherwise shutdown the domain services before this could occur */
        exampleSleepMilliseconds(1500);
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
 * @return 0 if a sample was successfully read by the listener and the listener
 * received a notification of deadline expiration successfully, 1 otherwise.
 */
int subscriber(int argc, char *argv[])
{
    int result = 0;
    try
    {
        /** A domain participant, topic, and subscriber are created with matching QoS to
         the ::publisher */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
        dds::topic::qos::TopicQos topicQos = dp.default_topic_qos()
                                                    << dds::core::policy::Durability::Transient()
                                                    << dds::core::policy::Reliability::Reliable()
                                                    << dds::core::policy::Deadline(dds::core::Duration(1, 0));
        dds::topic::Topic<ListenerData::Msg> topic(dp, "ListenerData_Msg", topicQos);
        dp.default_topic_qos(topicQos);
        std::string name = "Listener example";
        dds::sub::qos::SubscriberQos subQos
            = dp.default_subscriber_qos()
                << dds::core::policy::Partition(name);
        dds::sub::Subscriber sub(dp, subQos);
        dds::sub::qos::DataReaderQos drqos = topic.qos();

        /** An ExampleDataReaderListener and dds::core::status::StatusMask for the events
         * StatusMask::requested_deadline_missed() and StatusMask::data_available() are
         * both created and then specified when creating the dds::sub::DataReader */
        ExampleDataReaderListener listener;
        dds::core::status::StatusMask mask;
        mask << dds::core::status::StatusMask::data_available()
             << dds::core::status::StatusMask::requested_deadline_missed();
        std::cout << "=== [ListenerDataSubscriber] Set listener" << std::endl;
        dds::sub::DataReader<ListenerData::Msg> dr(sub, topic, drqos, &listener, mask);
        std::cout << "=== [ListenerDataSubscriber] Ready ..." << std::endl;

        /** The main thread then pauses until the listener thread has received a notification
         * that the deadline has expired (i.e. no more messages have been received for at least
         * 1 second) */
        int count = 0;
        while(! listener.deadline_expired_ && count < 20)
        {
            exampleSleepMilliseconds(1000);
            count++;
        }

        result = ! (listener.deadline_expired_ && listener.data_received_);

        /** Remove the ExampleDataReaderListener from the DataReader */
        dr.listener(0, dds::core::status::StatusMask::none());

        std::cout << "=== [ListenerDataSubscriber] Closed" << std::endl;
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

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Listener_publisher, examples::dcps::Listener::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Listener_subscriber, examples::dcps::Listener::isocpp::subscriber)
