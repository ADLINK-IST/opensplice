
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
#include <sstream>

#include "ContentFilteredTopicData_DCPS.hpp"

namespace {
    const unsigned int write_loop_count = 20;
}

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace ContentFilteredTopic {

/**
 * @addtogroup examplesdcpsContentFilteredTopicisocpp The ISO C++ DCPS API ContentFilteredTopic example
 *
 * In this example a ContentFilteredTopic is used to filter
 * the messages received by the subscriber to be only those
 * with a ticket equal to the one supplied as a parameter
 * to the program. Some non-default QoS are used to guarantee
 * delivery.
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
        dds::topic::Topic<StockMarket::Stock> topic(dp, "StockTrackerExclusive", topicQos);

        /** A dds::pub::Publisher is created on the domain participant. */
        std::string name = "ContentFilteredTopic example";
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
        dwqos << dds::core::policy::WriterDataLifecycle::AutoDisposeUnregisteredInstances();

        /** A dds::pub::DataWriter is created on the Publisher & Topic with the modififed Qos. */
        dds::pub::DataWriter<StockMarket::Stock> dw(pub, topic, dwqos);

        /** Two samples are created */
        StockMarket::Stock geQuote("GE", 12.00f);

        StockMarket::Stock msftQuote("MSFT", 25.00f);

        /** Update sample data and write data each second for 20 seconds */
        for (unsigned int i = 0; i < write_loop_count; i++)
        {
            geQuote.price() += 0.5f;
            msftQuote.price() += 1.5f;
            std::cout << "=== [ContentFilteredTopicDataPublisher] sends 2 stockQuotes : (GE, "
                      << geQuote.price() << ") (MSFT, " << msftQuote.price() << ")" << std::endl;
            dw << geQuote;
            dw << msftQuote;
            exampleSleepMilliseconds(1000);
        }

        /** A signal to terminate is sent to the subscriber */
        geQuote.price() = -1;
        msftQuote.price() = -1;
        dw << geQuote;
        dw << msftQuote;
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
        /** A check is performed to determine if a subscription string has been supplied to the program as an
         *  argument a string value of GE will subscribe to stocks from GE and a string value of MSFT will subscribe
         *  to stocks from MSFT. Failing to provide a string will cause the program to exit. */
        if(argc < 2)
        {
            std::cerr << "*** [ContentFilteredTopicDataSubscriber] Subscription string not specified" << std::endl;
            std::cerr << "*** usage : ContentFilteredTopicDataSubscriber <subscription_string>" << std::endl;
            return  - 1;
        }

        std::string requested_ticker = argv[1];

        /** A domain participant and topic are created identically as in
         the ::publisher */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
        dds::topic::qos::TopicQos topicQos = dp.default_topic_qos()
                                                    << dds::core::policy::Durability::Transient()
                                                    << dds::core::policy::Reliability::Reliable();
        dds::topic::Topic<StockMarket::Stock> topic(dp, "StockTrackerExclusive", topicQos);

        /** A dds::sub::Subscriber is created on the domain participant. */
        std::string name = "ContentFilteredTopic example";
        dds::sub::qos::SubscriberQos subQos
            = dp.default_subscriber_qos()
                << dds::core::policy::Partition(name);
        dds::sub::Subscriber sub(dp, subQos);

        /** The dds::sub::qos::DataReaderQos are derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos = topic.qos();

        /** A dds::topic::ContentFilteredTopic is created filtered on the stock ticker supplied to the
         * program as an argument */
        std::stringstream ss;
        ss << "ticker = '" << requested_ticker << "'";
        dds::topic::Filter filter(ss.str());
        dds::topic::ContentFilteredTopic<StockMarket::Stock> cftopic(topic, "CFStockTrackerExclusive", filter);

        /** A dds::sub::DataReader is created on the Subscriber & Topic with the DataReaderQos. */
        dds::sub::DataReader<StockMarket::Stock> dr(sub, cftopic, drqos);

        /** An attempt to take samples is made repeatedly until a stock with a price of -1 is received,
         * or sixty seconds have elapsed. */
        bool terminate_sample_received = false;
        unsigned int correct_quote_count = 0;
        for (unsigned int count = 0; count < write_loop_count * 10 && !terminate_sample_received; ++count)
        {
            dds::sub::LoanedSamples<StockMarket::Stock> samples = dr.take();
            for (dds::sub::LoanedSamples<StockMarket::Stock>::const_iterator sample = samples.begin();
                 sample < samples.end();
                 ++sample)
            {
                if(sample->info().valid())
                {
                    if (sample->data().price() == -1)
                    {
                        terminate_sample_received = true;
                        break;
                    }
                    std::cout << "=== [ContentFilteredTopicDataSubscriber] receives stockQuote :  ("
                    << sample->data().ticker() << ", " << sample->data().price() << ")" << std::endl;

                    if (requested_ticker == sample->data().ticker())
                        ++correct_quote_count;
                    else
                    {
                        std::cerr << "=== [ContentFilteredTopicDataSubscriber] Unexpected quote received for "
                                  << sample->data().ticker() << std::endl;
                        result = 1;
                    }
                }
            }
            exampleSleepMilliseconds(900);
        }

        /** The results are confirmed to be as expected */
        if (terminate_sample_received)
        {
            std::cout << "=== [ContentFilteredTopicDataSubscriber] Market Closed." << std::endl;
            if (correct_quote_count != (requested_ticker == "GE" || requested_ticker == "MSFT" ? write_loop_count : 0))
            {
                std::cerr << "=== [ContentFilteredTopicDataSubscriber] Incorrect quote count." << std::endl;
                result = 1;
            }
        }
        else
        {
            std::cerr << "=== [ContentFilteredTopicDataSubscriber] Timed out waiting for close signal." << std::endl;
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

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_ContentFilteredTopic_publisher, examples::dcps::ContentFilteredTopic::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_ContentFilteredTopic_subscriber, examples::dcps::ContentFilteredTopic::isocpp::subscriber)
