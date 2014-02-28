
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

#include "RoundTrip_DCPS.hpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace RoundTrip {

/**
 * @addtogroup examplesdcpsRoundTripisocpp The ISO C++ DCPS API RoundTrip example
 *
 * The RoundTrip example consists of a Ping and a Pong application. Ping sends sample
 * to Pong by writing to the Ping partition which Pong subscribes to. Pong them sends them
 * back to Ping by writing on the Pong partition which Ping subscribes to. Ping measure
 * the amount of time taken to write and read each sample as well as the total round trip
 * time to send a sample to Pong and receive it back.
 * @ingroup examplesdcpsisocpp
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp  {

static dds::core::cond::GuardCondition terminated;
#ifndef _WIN32
struct sigaction oldAction;
#endif
/*
 * Function to handle Ctrl-C presses.
 * @param fdwCtrlType Ctrl signal type
 */
#ifdef _WIN32
static bool CtrlHandler(DWORD fdwCtrlType)
{
    terminated.trigger_value(true);
    return true; //Don't let other handlers handle this key
}
#else
static void CtrlHandler(int fdwCtrlType)
{
    terminated.trigger_value(true);
}
#endif

/**
 * This class serves as a container holding initialised entities used by ping and pong.
 */
class Entities
{
public:
    /**
     * This constructor initialises the entities used by ping and pong
     */
    Entities(std::string pubPartition, std::string subPartition) :
        writer(dds::core::null), reader(dds::core::null),
        waitSet(dds::core::null)
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant participant = dds::domain::DomainParticipant(org::opensplice::domain::default_id());

        /** A dds::topic::Topic is created for our sample type on the domain participant. */
        dds::topic::Topic<RoundTripModule::Sample> topic = dds::topic::Topic<RoundTripModule::Sample>(participant, "RoundTrip");

        /** A dds::pub::Publisher is created on the domain participant. */
        dds::pub::qos::PublisherQos pubQos
            = participant.default_publisher_qos()
                << dds::core::policy::Partition(pubPartition);
        dds::pub::Publisher publisher(participant, pubQos);

        /** A dds::pub::DataWriter is created on the Publisher & Topic with a modififed QoS. */
        dds::pub::qos::DataWriterQos dwqos = topic.qos();
        dwqos << dds::core::policy::Reliability::Reliable(dds::core::Duration(10, 0))
            << dds::core::policy::History::KeepAll()
            << dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances();
        writer = dds::pub::DataWriter<RoundTripModule::Sample>(publisher, topic, dwqos);

        /** A dds::sub::Subscriber is created on the domain participant. */
        dds::sub::qos::SubscriberQos subQos
            = participant.default_subscriber_qos()
                << dds::core::policy::Partition(subPartition);
        dds::sub::Subscriber subscriber(participant, subQos);

        /** A dds::sub::DataReader is created on the Subscriber & Topic with a modified QoS. */
        dds::sub::qos::DataReaderQos drqos = topic.qos();
        drqos << dds::core::policy::Reliability::Reliable(dds::core::Duration(10, 0))
            << dds::core::policy::History::KeepAll();
        reader = dds::sub::DataReader<RoundTripModule::Sample>(subscriber, topic, drqos);

        /** A StatusCondition is created which is triggered when data is available to read. */
        dds::core::cond::StatusCondition dataAvailable(reader);
        dds::core::status::StatusMask statusMask;
        statusMask << dds::core::status::StatusMask::data_available();
        dataAvailable.enabled_statuses(statusMask);

        /** A WaitSet is created and the data available status condition is attached. */
        waitSet = dds::core::cond::WaitSet();
        waitSet += dataAvailable;

        waitSet += terminated;

        /** Initialise ExampleTimeStats used to track timing */
        roundTrip = exampleInitTimeStats();
        writeAccess = exampleInitTimeStats();
        readAccess = exampleInitTimeStats();
        roundTripOverall = exampleInitTimeStats();
        writeAccessOverall = exampleInitTimeStats();
        readAccessOverall = exampleInitTimeStats();
    }

    ~Entities()
    {
        exampleDeleteTimeStats(roundTrip);
        exampleDeleteTimeStats(writeAccess);
        exampleDeleteTimeStats(readAccess);
        exampleDeleteTimeStats(roundTripOverall);
        exampleDeleteTimeStats(writeAccessOverall);
        exampleDeleteTimeStats(readAccessOverall);
    }

public:
    /** The DataWriter used by ping and pong */
    dds::pub::DataWriter<RoundTripModule::Sample> writer;
    /** The DataReader used by ping and pong */
    dds::sub::DataReader<RoundTripModule::Sample> reader;
    /** The WaitSet used by ping and pong */
    dds::core::cond::WaitSet waitSet;

    ExampleTimeStats roundTrip;
    ExampleTimeStats writeAccess;
    ExampleTimeStats readAccess;
    ExampleTimeStats roundTripOverall;
    ExampleTimeStats writeAccessOverall;
    ExampleTimeStats readAccessOverall;
};

/**
 * This function performs the Ping role in this example.
 * @return 0 if a sample is successfully written, 1 otherwise.
 */
int ping(int argc, char *argv[])
{
    /* Register handler for Ctrl-C */
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true);
#else
    struct sigaction sat;
    sat.sa_handler = CtrlHandler;
    sigemptyset(&sat.sa_mask);
    sat.sa_flags = 0;
    sigaction(SIGINT,&sat,&oldAction);
#endif

    int result = 0;
	setbuf(stdout, NULL);

    try
    {
        /** Initialise entities */
        Entities e("ping", "pong");

        unsigned long payloadSize = 0;
        unsigned long numSamples = 0;
        unsigned long timeOut = 0;

        /** Interpret arguments */
        if(argc == 2 && strcmp(argv[1], "quit") == 0)
        {
            /** Wait for pong to run */
            std::cout << "Waiting for pong to run..." << std::endl;
            while(matched_subscriptions(e.writer).size() == 0) { };
            std::cout << "Sending termination request." << std::endl;
            /** Send quit signal to pong if "quit" is supplied as an argument to ping */
            RoundTripModule::Sample data;
            dds::core::InstanceHandle ih = e.writer.register_instance(data);
            e.writer.dispose_instance(ih);
            exampleSleepMilliseconds(1000);
            return 0;
        }
        bool invalid = false;
        if(argc >= 2)
        {
            payloadSize = atoi(argv[1]);

            if(payloadSize > 655536)
            {
                invalid = true;
            }
        }
        if(argc >= 3)
        {
            numSamples = atoi(argv[2]);
        }
        if(argc >= 4)
        {
            timeOut = atoi(argv[3]);
        }
        if(invalid || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)))
        {
            std::string exception
                       = "Usage (parameters must be supplied in order):\n"
                            "./ping [payloadSize (bytes, 0 - 655536)] [numSamples (0 = infinite)] [timeOut (seconds, 0 = infinite)]\n"
                            "./ping quit - ping sends a quit signal to pong.\n"
                            "Defaults:\n"
                            "./ping 0 0 0";
            throw exception;
        }

        std::cout << "# payloadSize: " << payloadSize << " | numSamples: " << numSamples << " | timeOut: " << timeOut << "\n" << std::endl;

        timeval startTime;
        timeval preWriteTime;
        timeval postWriteTime;
        timeval preTakeTime;
        timeval postTakeTime;
        dds::core::cond::WaitSet::ConditionSeq conditions;
        dds::core::Duration waitTimeout(1, 0);
        RoundTripModule::Sample data;
        for(unsigned long i = 0; i < payloadSize; i++)
        {
            data.payload().push_back('a');
        }

        startTime = exampleGetTime();
        std::cout << "# Warming up..." << std::endl;
        bool warmUp = true;
        while(!terminated.trigger_value() && (exampleTimevalToMicroseconds(exampleGetTime() - startTime) / US_IN_ONE_SEC < 5))
        {
            e.writer << data;
            try
            {
                e.waitSet.wait(conditions, waitTimeout);
                e.reader.take();
            }
            catch(dds::core::TimeoutError& e)
            {
                (void)e;
            }
        }
        if(!terminated.trigger_value())
        {
            warmUp = false;
            std::cout << "# Warm up complete.\n" << std::endl;

            std::cout << "# Round trip measurements (in us)" << std::endl;
            std::cout << "#             Round trip time [us]         Write-access time [us]       Read-access time [us]" << std::endl;
            std::cout << "# Seconds     Count   median      min      Count   median      min      Count   median      min" << std::endl;
        }

        startTime = exampleGetTime();
        unsigned long elapsed = 0;
        for(unsigned long i = 0; !terminated.trigger_value() && (!numSamples || i < numSamples); i++)
        {
            /** Write a sample that pong can send back */
            preWriteTime = exampleGetTime();
            e.writer << data;
            postWriteTime = exampleGetTime();

            try
            {
                /** Wait for response from pong */
                e.waitSet.wait(conditions, waitTimeout);

                /** Take sample and check that it is valid */
                preTakeTime = exampleGetTime();
                dds::sub::LoanedSamples<RoundTripModule::Sample> samples = e.reader.take();
                postTakeTime = exampleGetTime();

                if(!terminated.trigger_value())
                {
                    if(samples.length() != 1)
                    {
                        std::stringstream exception;
                        exception << "ERROR: Ping received " << samples.length() << " samples but was expecting 1. Are multiple pong applications running?";
                        throw exception.str();
                    }
                    else if(!samples.begin()[0]->info().valid())
                    {
                        std::string exception = "ERROR: Ping received an invalid sample. Has pong terminated already?";
                        throw exception;
                    }
                }

                /** Update stats */
                e.writeAccess += exampleTimevalToMicroseconds(postWriteTime - preWriteTime);
                e.readAccess += exampleTimevalToMicroseconds(postTakeTime - preTakeTime);
                e.roundTrip += exampleTimevalToMicroseconds(postTakeTime - preWriteTime);
                e.writeAccessOverall += exampleTimevalToMicroseconds(postWriteTime - preWriteTime);
                e.readAccessOverall += exampleTimevalToMicroseconds(postTakeTime - preTakeTime);
                e.roundTripOverall += exampleTimevalToMicroseconds(postTakeTime - preWriteTime);

                /** Print stats each second */
                if((exampleTimevalToMicroseconds(postTakeTime - startTime) > US_IN_ONE_SEC) || (i && i == numSamples))
                {
                    /* Print stats */
                    printf ("%9lu %9lu %8.0f %8lu %10lu %8.0f %8lu %10lu %8.0f %8lu\n",
                        elapsed + 1,
                        e.roundTrip.count,
                        exampleGetMedianFromTimeStats(e.roundTrip),
                        e.roundTrip.min,
                        e.writeAccess.count,
                        exampleGetMedianFromTimeStats(e.writeAccess),
                        e.writeAccess.min,
                        e.readAccess.count,
                        exampleGetMedianFromTimeStats(e.readAccess),
                        e.readAccess.min);

                    /* Reset stats for next run */
                    exampleResetTimeStats(e.roundTrip);
                    exampleResetTimeStats(e.writeAccess);
                    exampleResetTimeStats(e.readAccess);

                    /** Set values for next run */
                    startTime = exampleGetTime();
                    elapsed++;
                }
            }
            catch(dds::core::TimeoutError& e)
            {
                (void)e;
                elapsed += (unsigned long)waitTimeout.sec();
            }

            if(timeOut && elapsed == timeOut)
            {
                terminated.trigger_value(true);
            }
        }

        if(!warmUp)
        {
            /** Print overall stats */
            printf ("\n%9s %9lu %8.0f %8lu %10lu %8.0f %8lu %10lu %8.0f %8lu\n",
                        "# Overall",
                        e.roundTripOverall.count,
                        exampleGetMedianFromTimeStats(e.roundTripOverall),
                        e.roundTripOverall.min,
                        e.writeAccessOverall.count,
                        exampleGetMedianFromTimeStats(e.writeAccessOverall),
                        e.writeAccessOverall.min,
                        e.readAccessOverall.count,
                        exampleGetMedianFromTimeStats(e.readAccessOverall),
                        e.readAccessOverall.min);
        }
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = 1;
    }
    catch(const std::string& s)
    {
        std::cerr << s << std::endl;
        result = 1;
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(0, false);
#else
    sigaction(SIGINT,&oldAction, 0);
#endif
    return result;
}

/**
 * Runs the Pong role in this example.
 * @return 0 if a sample is successfully read, 1 otherwise.
 */
int pong(int argc, char *argv[])
{
    /* Register handler for Ctrl-C */
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true);
#else
    struct sigaction sat;
    sat.sa_handler = CtrlHandler;
    sigemptyset(&sat.sa_mask);
    sat.sa_flags = 0;
    sigaction(SIGINT,&sat,&oldAction);
#endif

    int result = 0;

    try
    {
        /** Initialise entities */
        Entities e("pong", "ping");

        std::cout << "Waiting for samples from ping to send back..." << std::endl;

        dds::core::cond::WaitSet::ConditionSeq conditions;
        while(!terminated.trigger_value())
        {
            /** Wait for a sample from ping */
            e.waitSet.wait(conditions);

            /** Take samples */
            dds::sub::LoanedSamples<RoundTripModule::Sample> samples = e.reader.take();
            for(dds::sub::LoanedSamples<RoundTripModule::Sample>::const_iterator sample = samples.begin();
                 !terminated.trigger_value() && sample < samples.end();
                 ++sample)
            {
                /** If writer has been disposed terminate pong */
                if(sample->info().state().instance_state() == dds::sub::status::InstanceState::not_alive_disposed())
                {
                    std::cout << "Received termination request. Terminating." << std::endl;
                    terminated.trigger_value(true);
                    break;
                }
                /** If sample is valid, send it back to ping */
                else if(sample->info().valid())
                {
                    e.writer << sample->data();
                }
            }
        }
    }
    catch(const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = 1;
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(0, false);
#else
    sigaction(SIGINT,&oldAction, 0);
#endif
    return result;
}

}
}
}
}

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_RoundTrip_ping, examples::dcps::RoundTrip::isocpp::ping)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_RoundTrip_pong, examples::dcps::RoundTrip::isocpp::pong)
