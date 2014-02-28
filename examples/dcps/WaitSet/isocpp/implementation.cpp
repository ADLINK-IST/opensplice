
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
#include <vector>

#include "WaitSetData_DCPS.hpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace WaitSet {

/**
 * @addtogroup examplesdcpsWaitSetisocpp The ISO C++ DCPS API WaitSet example
 *
 * This example demonstrates how a WaitSet can be used to wait on certain
 * conditions which will then trigger a corresponding handler functor to perform any
 * actions required such as reading data.
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
                << dds::core::policy::Reliability::Reliable()
                << dds::core::policy::History::KeepLast(2);

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<WaitSetData::Msg> topic(dp, "WaitSetData_Msg", topicQos);

        /** A dds::pub::Publisher is created on the domain participant. */
        std::string name = "WaitSet example";
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
        dds::pub::DataWriter<WaitSetData::Msg> dw(pub, topic, dwqos);

        /** A sample is created and then written. */
        WaitSetData::Msg msgInstance(1, "First Hello");
        dw << msgInstance;

        std::cout << "=== [Publisher] written a message containing :" << std::endl;
        std::cout << "    userID  : " << msgInstance.userID() << std::endl;
        std::cout << "    Message : \"" << msgInstance.message() << "\"" << std::endl;

        exampleSleepMilliseconds(500); //Sleep to ensure message is sent

        /** A second sample is created and then written */
        msgInstance.message() = "Hello again";
        dw << msgInstance;

        std::cout << "=== [Publisher] written a message containing :" << std::endl;
        std::cout << "    userID  : " << msgInstance.userID() << std::endl;
        std::cout << "    Message : \"" << msgInstance.message() << "\"" << std::endl;

        /* A short sleep ensures time is allowed for the sample to be written to the network.
        If the example is running in *Single Process Mode* exiting immediately might
        otherwise shutdown the domain services before this could occur */
        exampleSleepMilliseconds(500);
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = 1;
    }
    return result;
}

/**
 * Functor to handle the triggering of a ReadCondition within a WaitSet.
 * When the ReadCondition is triggered it will output the messages that
 * triggered the ReadCondition.
 */
class ReadCondHandler
{
public:
    /**
     * @param dataState The dataState on which to filter the samples
     */
    ReadCondHandler(dds::sub::status::DataState& dataState)
    : dataState(dataState) {}

    void operator() (dds::sub::DataReader<WaitSetData::Msg>& dr)
    {
        dds::sub::LoanedSamples<WaitSetData::Msg> samples = dr.select().state(dataState).take();

        for (dds::sub::LoanedSamples<WaitSetData::Msg>::const_iterator sample = samples.begin();
            sample < samples.end(); ++sample)
        {
            if((*sample).info().valid())
            {
                std::cout << "\n    --- New message received ---" << std::endl;
                std::cout << "    userID  : " << sample->data().userID() << std::endl;
                std::cout << "    Message : \"" << sample->data().message() << "\"" << std::endl;
            }
        }
    }

private:
    dds::sub::status::DataState& dataState;
};

/**
 * Functor to handle the triggering of a QueryCondition within a WaitSet.
 * When the QueryCondition is triggered it will output the messages that
 * triggered the QueryCondition.
 */
class QueryCondHandler
{
public:
    /**
     * @param query The query on which to filter the samples
     * @param dataState The dataState on which to filter the samples
     */
    QueryCondHandler(dds::sub::Query query, dds::sub::status::DataState& dataState)
    : query(query), dataState(dataState) {}

    void operator() (dds::sub::AnyDataReader& adr)
    {
        dds::sub::DataReader<WaitSetData::Msg> dr = adr.get<WaitSetData::Msg>(); //Get DataReader from AnyDataReader
        dds::sub::LoanedSamples<WaitSetData::Msg> samples = dr.select().content(query).state(dataState).take();

        for (dds::sub::LoanedSamples<WaitSetData::Msg>::const_iterator sample = samples.begin();
            sample < samples.end(); ++sample)
        {
            if(sample->info().valid())
            {
                std::cout << "\n    --- message received (with QueryCondition on message field) ---" << std::endl;
                std::cout << "    userID  : " << sample->data().userID() << std::endl;
                std::cout << "    Message : \"" << sample->data().message() << "\"" << std::endl;
            }
        }
    }

private:
    dds::sub::Query query;
    dds::sub::status::DataState& dataState;
};

/**
 * Functor to handle the triggering of a StatusCondition within a WaitSet.
 * When the StatusCondition is triggered it will check if a DataWriter has
 * lost it's liveliness and output a message indicating this or an alternate
 * message if a DataWriter has joined.
 */
class StatusCondHandler
{
public:
    /**
     * @param prevAliveCount The previous number of DataWriters
     * @param writerLeft Indicates if a DataWriter has lost it's liveliness
     * @param guardTriggered Indicates if the GuardCondition should be triggered
     */
    StatusCondHandler(int& prevAliveCount, bool& writerLeft, bool& guardTriggered)
    : prevAliveCount(prevAliveCount), writerLeft(writerLeft), guardTriggered(guardTriggered) {}

    void operator() (dds::core::Entity& e)
    {
        const dds::core::status::LivelinessChangedStatus livChangedStatus = ((dds::sub::DataReader<WaitSetData::Msg>&)e).liveliness_changed_status();

        if(livChangedStatus.alive_count() < prevAliveCount)
        {
            std::cout << "\n!!! a DataWriter lost its liveliness" << std::endl;
            writerLeft = true;
            std::cout << "\n=== Triggering escape condition " << std::endl;
            guardTriggered = true;
        }
        else if(livChangedStatus.alive_count() > prevAliveCount)
        {
            std::cout << "\n!!! a DataWriter joined" << std::endl;
        }
        prevAliveCount = livChangedStatus.alive_count();
    }

private:
    int& prevAliveCount;
    bool& writerLeft;
    bool& guardTriggered;
};

/**
 * Functor to handle the triggering of a GuardCondition within a WaitSet.
 * When the GuardCondition is triggered it will set a boolean to indicate
 * that the example should exit.
 */
class GuardCondHandler
{
public:
    /**
     * @param escaped Indicates that the example should exit
     * @param count Indicates the number of times the WaitSet waited
     */
    GuardCondHandler(bool& escaped, int& count)
    : escaped(escaped), count(count) {}

    void operator() (void)
    {
        std::cout << std::endl << "!!! escape condition triggered - count = " << count << std::endl;
        escaped = true;
    }

private:
    bool& escaped;
    int& count;
};

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
                                                    << dds::core::policy::History::KeepLast(2);
        dds::topic::Topic<WaitSetData::Msg> topic(dp, "WaitSetData_Msg", topicQos);

        /** A dds::sub::Subscriber is created on the domain participant. */
        std::string name = "WaitSet example";
        dds::sub::qos::SubscriberQos subQos
            = dp.default_subscriber_qos()
                << dds::core::policy::Partition(name);
        dds::sub::Subscriber sub(dp, subQos);

        /** The dds::sub::qos::DataReaderQos are derived from the topic qos */
        dds::sub::qos::DataReaderQos drqos = topic.qos();

        /** A dds::sub::DataReader is created on the Subscriber & Topic with the DataReaderQos. */
        dds::sub::DataReader<WaitSetData::Msg> dr(sub, topic, drqos);

        //Logic variables
        int prevAliveCount = 0;
        bool closed = false;
        bool escaped = false;
        bool writerLeft = false;
        bool guardTriggered = false;
        int count = 0;

        /** A ReadCondition is created which is triggered when a new message is received */
        dds::sub::status::DataState newDataState;
        newDataState << dds::sub::status::SampleState::not_read()
                << dds::sub::status::ViewState::new_view()
                << dds::sub::status::InstanceState::any();
        ReadCondHandler readCondHandler(newDataState);
        dds::sub::cond::ReadCondition readCond(dr, newDataState, readCondHandler);

        /** A query condition is created which is triggered when a message with the string "Hello again" is received */
        std::cout << "=== [WaitSetDataSubscriber] Query : message = \"Hello again\"" << std::endl;
        dds::sub::Query query(dr, "message='Hello again'");
        dds::sub::status::DataState anyDataState;
        QueryCondHandler queryCondHandler(query, anyDataState);
        dds::sub::cond::QueryCondition queryCond(query, anyDataState, queryCondHandler);

        /** A StatusCondition is created which is triggered when the DataWriter changes it's liveliness */
        StatusCondHandler statusCondHandler(prevAliveCount, writerLeft, guardTriggered);
        dds::core::cond::StatusCondition statusCond(dr, statusCondHandler);
        dds::core::status::StatusMask statusMask;
        statusMask << dds::core::status::StatusMask::liveliness_changed();
        statusCond.enabled_statuses(statusMask);

        /** A GuardCondition is created which will be used to close the subscriber */
        GuardCondHandler guardCondHandler(escaped, count);
        dds::core::cond::GuardCondition guardCond;
        guardCond.handler(guardCondHandler);

        /** A WaitSet is created and the four conditions created above are attached to it */
        dds::core::cond::WaitSet waitSet;
        waitSet += readCond;
        waitSet += queryCond;
        waitSet += statusCond;
        waitSet += guardCond;

        dds::core::Duration waitTimeout(20, 0);

        std::cout << "=== [WaitSetDataSubscriber] Ready ..." << std::endl;

        while(!closed && count < 20)
        {
            /** Wait until at least one of the conditions in the WaitSet triggers and dispatch the corresponding functor*/
            try
            {
                waitSet.dispatch(waitTimeout);
                /** The GuardCondition is triggered if the StatusCondition determines that a DataWriter has lost it's liveliness */
                guardCond.trigger_value(guardTriggered);
            }
            catch(const dds::core::TimeoutError e)
            {
                std::cout << std::endl << "!!! [INFO] WaitSet timed out - count = " << count << std::endl;
            }
            ++count;
            /** The example will exit if the GuardCondition has been triggered */
            closed = escaped && writerLeft;
        }
        if(count >= 20)
        {
            std::cout << std::endl << "*** Error : Timed out - count = " << count << " ***" << std::endl;
            result = 1;
        }
        else
        {
            std::cout << "=== [Subscriber] Closed" << std::endl;
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

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_WaitSet_publisher, examples::dcps::WaitSet::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_WaitSet_subscriber, examples::dcps::WaitSet::isocpp::subscriber)
