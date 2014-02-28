
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

#include "QueryConditionData_DCPS.hpp"

namespace {
    const unsigned int write_loop_count = 20;
}

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace QueryCondition {

/**
 * @addtogroup examplesdcpsQueryConditionisocpp The ISO C++ DCPS API QueryCondition example
 *
 * This is a reasonably simple case publisher and subscriber
 * example. One sample is published, the sample is then read.
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
        dds::topic::Topic<StockMarket::Stock> topic(dp, "StockTrackerExclusive", topicQos);

        /** A dds::pub::Publisher is created on the domain participant. */
        std::string name = "QueryCondition example";
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
        dds::pub::DataWriter<StockMarket::Stock> dw(pub, topic, dwqos);

        /** Two samples are created */
        StockMarket::Stock geQuote("GE", 12.00f);

        StockMarket::Stock msftQuote("MSFT", 25.00f);

        dds::core::InstanceHandle geHandle = dw.register_instance(geQuote);
        dds::core::InstanceHandle msftHandle = dw.register_instance(msftQuote);

        /** Update sample data and write data each second for 20 seconds */
        for (unsigned int i = 0; i < write_loop_count; i++)
        {
            geQuote.price() += 0.5f;
            msftQuote.price() += 1.5f;
            std::cout << "=== [QueryConditionDataPublisher] sends 2 stockQuotes : (GE, "
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

        dw.dispose_instance(geHandle);
        dw.dispose_instance(msftHandle);

        dw.unregister_instance(geHandle);
        dw.unregister_instance(msftHandle);

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
        // usage : QueryConditionDataQuerySubscriber <query_string>
        std::string requested_ticker;
        if (argc > 1)
        {
            requested_ticker = argv[1];
        }
        else
        {
            std::cerr << "*** [QueryConditionDataQuerySubscriber] Query string not specified" << std::endl;
            std::cerr << "*** usage : QueryConditionDataQuerySubscriber <query_string>" << std::endl;
            return 1;
        }

        /** A domain participant and topic are created identically as in
         the ::publisher */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
        dds::topic::qos::TopicQos topicQos = dp.default_topic_qos()
                                                    << dds::core::policy::Durability::Transient()
                                                    << dds::core::policy::Reliability::Reliable();
        dds::topic::Topic<StockMarket::Stock> topic(dp, "StockTrackerExclusive", topicQos);

        /** A dds::sub::Subscriber is created on the domain participant. */
        std::string name = "QueryCondition example";
        dds::sub::qos::SubscriberQos subQos
            = dp.default_subscriber_qos()
                << dds::core::policy::Partition(name);
        dds::sub::Subscriber sub(dp, subQos);

        /** The dds::sub::qos::DataReaderQos are derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos = topic.qos();

        /** A dds::sub::DataReader is created on the Subscriber & Topic with the DataReaderQos. */
        dds::sub::DataReader<StockMarket::Stock> dr(sub, topic, drqos);

        /** A query and data state are created */
        std::cout << "=== [QueryConditionDataQuerySubscriber] Query : ticker = " << requested_ticker << std::endl;
        std::vector<std::string> params;
        params.push_back(requested_ticker);
        dds::sub::Query query(dr, "ticker=%0", params);
        dds::sub::status::DataState anyDataState;

        /** An attempt to take samples is made repeatedly until it succeeds,
         * or 60 seconds have elapsed. */
        bool closed = false;
        unsigned int count = 0;
        unsigned int correct_quote_count = 0;
        do
        {
            dds::sub::LoanedSamples<StockMarket::Stock> samples;
            /* The below two approaches are equivalent so we can just alternate between them */
            if (correct_quote_count % 2)
            {
                /* take type #2 */
                dr >> dds::sub::content(query)
                   >> dds::sub::state(dds::sub::status::DataState::any())
                   >> dds::sub::take
                   >> samples;
            }
            else
            {
                /* take type #1 */
                samples = dr.select().content(query).state(dds::sub::status::DataState::any()).take();
            }
            for (dds::sub::LoanedSamples<StockMarket::Stock>::const_iterator sample = samples.begin();
                 sample < samples.end();
                 ++sample)
            {
                if (sample->info().valid())
                {
                    if(sample->data().price() == -1.0f)
                    {
                        closed = true;
                        break;
                    }
                    std::cout << "=== [QueryConditionDataSubscriber] receives stockQuote :  ("
                              << sample->data().ticker() << ": " << sample->data().price()
                              << ") using 'take' type #" << (correct_quote_count % 2) + 1 << std::endl;

                    /* Verify the value is as expected */
                    if (requested_ticker == sample->data().ticker())
                        ++correct_quote_count;
                    else
                    {
                        std::cerr << "=== [QueryConditionDataQuerySubscriber] Unexpected quote received for "
                                  << sample->data().ticker() << std::endl;
                        result = 1;
                    }
                }
            }
            exampleSleepMilliseconds(900);
            ++count;
        }
        while (!closed && count < write_loop_count * 30);

        if (!closed)
        {
            std::cerr << "ERROR: Waited for 60 seconds but no sample received" << std::endl;
            result = 1;
        }
        else
        {
            std::cout << "=== [QueryConditionDataQuerySubscriber] Market Closed" << std::endl;
            if (correct_quote_count != (requested_ticker == "GE" || requested_ticker == "MSFT" ? write_loop_count : 0))
            {
                std::cerr << "=== [QueryConditionDataQuerySubscriber] Incorrect quote count." << std::endl;
                result = 1;
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

}
}
}
}

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_QueryCondition_publisher, examples::dcps::QueryCondition::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_QueryCondition_subscriber, examples::dcps::QueryCondition::isocpp::subscriber)
