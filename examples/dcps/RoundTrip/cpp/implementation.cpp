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
#include "common/example_error_sacpp.hpp"

#include <iostream>

#include "ccpp_RoundTrip.h"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace RoundTrip {

/**
 * @addtogroup examplesdcpsRoundTripsacpp The Standalone C++ DCPS API RoundTrip example
 *
 * The RoundTrip example consists of a Ping and a Pong application. Ping sends sample
 * to Pong by writing to the Ping partition which Pong subscribes to. Pong them sends them
 * back to Ping by writing on the Pong partition which Ping subscribes to. Ping measure
 * the amount of time taken to write and read each sample as well as the total round trip
 * time to send a sample to Pong and receive it back.
 * @ingroup examplesdcpssacpp
 */
/** @{*/
/** @dir */
/** @file */

namespace sacpp  {

static DDS::GuardCondition_var terminated = new DDS::GuardCondition();
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
    terminated->set_trigger_value(true);
    return true; //Don't let other handlers handle this key
}
#else
static void CtrlHandler(int fdwCtrlType)
{
    terminated->set_trigger_value(true);
}
#endif

DDS::ReturnCode_t status;

/**
 * This class serves as a container holding initialised entities used by ping and pong.
 */
class Entities
{
public:
    /**
     * This constructor initialises the entities used by ping and pong
     */
    Entities(const char *pubPartition, const char *subPartition)
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
        typeSupport = new RoundTripModule::SampleTypeSupport();
        status = typeSupport.in()->register_type(participant.in(), typeSupport.in()->get_type_name());
        CHECK_STATUS_MACRO(status);

        /** A DDS::Topic is created for our sample type on the domain participant. */
        topic = participant.in()->create_topic(
            "RoundTrip", typeSupport.in()->get_type_name(), TOPIC_QOS_DEFAULT, 0, DDS::STATUS_MASK_NONE);
        CHECK_HANDLE_MACRO(topic.in());

        /** A DDS::Publisher is created on the domain participant. */
        DDS::PublisherQos pubQos;
        status = participant->get_default_publisher_qos(pubQos);
        CHECK_STATUS_MACRO(status);
        pubQos.partition.name.length(1);
        pubQos.partition.name[0] = DDS::string_dup(pubPartition);
        publisher = participant->create_publisher(pubQos, 0, DDS::STATUS_MASK_NONE);
        CHECK_HANDLE_MACRO(publisher.in());

        /** A DDS::DataWriter is created on the Publisher & Topic with a modififed Qos. */
        DDS::DataWriterQos dwQos;
        status = publisher->get_default_datawriter_qos(dwQos);
        CHECK_STATUS_MACRO(status);
        dwQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
        dwQos.reliability.max_blocking_time.sec = 10;
        dwQos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
        dwQos.writer_data_lifecycle.autodispose_unregistered_instances = false;
        DDS::DataWriter_var tmpWriter = publisher->create_datawriter(topic.in(), dwQos, 0, DDS::STATUS_MASK_NONE);
        writer = RoundTripModule::SampleDataWriter::_narrow(tmpWriter.in());
        CHECK_HANDLE_MACRO(writer.in());

        /** A DDS::Subscriber is created on the domain participant. */
        DDS::SubscriberQos subQos;
        status = participant->get_default_subscriber_qos(subQos);
        CHECK_STATUS_MACRO(status);
        subQos.partition.name.length(1);
        subQos.partition.name[0] = DDS::string_dup(subPartition);
        subscriber = participant->create_subscriber(subQos, 0, DDS::STATUS_MASK_NONE);
        CHECK_HANDLE_MACRO(subscriber.in());

        /** A DDS::DataReader is created on the Subscriber & Topic with a modified QoS. */
        DDS::DataReaderQos drQos;
        status = subscriber->get_default_datareader_qos(drQos);
        CHECK_STATUS_MACRO(status);
        drQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
        drQos.reliability.max_blocking_time.sec = 10;
        drQos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
        DDS::DataReader_var tmpReader = subscriber->create_datareader(topic.in(), drQos, 0, DDS::STATUS_MASK_NONE);
        reader = RoundTripModule::SampleDataReader::_narrow(tmpReader.in());
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

        status = waitSet->attach_condition(terminated);
        CHECK_STATUS_MACRO(status);

        /** Initialise ExampleTimeStats used to track timing */
        roundTrip = exampleInitTimeStats();
        writeAccess = exampleInitTimeStats();
        readAccess = exampleInitTimeStats();
        roundTripOverall = exampleInitTimeStats();
        writeAccessOverall = exampleInitTimeStats();
        readAccessOverall = exampleInitTimeStats();
    }

    /**
     * This destructor cleans up after the application has finished running
     */
    ~Entities()
    {
#ifdef _WIN32
        SetConsoleCtrlHandler(0, false);
#else
        sigaction(SIGINT,&oldAction, 0);
#endif

        status = waitSet->detach_condition(dataAvailable.in());
        CHECK_STATUS_MACRO(status);
        status = participant->delete_contained_entities();
        CHECK_STATUS_MACRO(status);
        status = domainParticipantFactory->delete_participant(participant);
        CHECK_STATUS_MACRO(status);

        exampleDeleteTimeStats(roundTrip);
        exampleDeleteTimeStats(writeAccess);
        exampleDeleteTimeStats(readAccess);
        exampleDeleteTimeStats(roundTripOverall);
        exampleDeleteTimeStats(writeAccessOverall);
        exampleDeleteTimeStats(readAccessOverall);
    }

public:
    /** The DomainParticipantFactory used by ping and pong */
    DDS::DomainParticipantFactory_var domainParticipantFactory;
    /** The DomainParticipant used by ping and pong */
    DDS::DomainParticipant_var participant;
    /** The TypeSupport for the sample */
    RoundTripModule::SampleTypeSupport_var typeSupport;
    /** The Topic used by ping and pong */
    DDS::Topic_var topic;
    /** The Publisher used by ping and pong */
    DDS::Publisher_var publisher;
    /** The DataWriter used by ping and pong */
    RoundTripModule::SampleDataWriter_var writer;
    /** The Subscriber used by ping and pong */
    DDS::Subscriber_var subscriber;
    /** The DataReader used by ping and pong */
    RoundTripModule::SampleDataReader_var reader;
    /** The WaitSet used by ping and pong */
    DDS::WaitSet_var waitSet;
    /** The StatusCondition used by ping and pong,
     * triggered when data is available to read */
    DDS::StatusCondition_var dataAvailable;

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
    int result = 0;
	setbuf(stdout, NULL);

    try
    {
        /** Initialise entities */
        Entities e("ping", "pong");

        unsigned long payloadSize = 0;
        unsigned long long numSamples = 0;
        unsigned long timeOut = 0;

        /** Interpret arguments */
        if(argc == 2 && strcmp(argv[1], "quit") == 0)
        {
            /** Wait for pong to run */
            std::cout << "Waiting for pong to run..." << std::endl;
            bool pongRunning = false;
            while(!pongRunning)
            {
                DDS::InstanceHandleSeq ihs;
                e.writer->get_matched_subscriptions(ihs);
                if(ihs.length() != 0)
                {
                    pongRunning = true;
                }
            };
            std::cout << "Sending termination request." << std::endl;
            /** Send quit signal to pong if "quit" is supplied as an argument to ping */
            RoundTripModule::Sample data;
            e.writer->dispose(data, DDS::HANDLE_NIL);
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
            DDS::String_var exception = DDS::string_dup(
                "Usage (parameters must be supplied in order):\n"
                "./ping [payloadSize (bytes, 0 - 655536)] [numSamples (0 = infinite)] [timeOut (seconds, 0 = infinite)]\n"
                "./ping quit - ping sends a quit signal to pong.\n"
                "Defaults:\n"
                "./ping 0 0 0");
            throw exception;
        }

        std::cout << "# payloadSize: " << payloadSize << " | numSamples: " << numSamples << " | timeOut: " << timeOut << "\n" << std::endl;

        timeval startTime;
        timeval preWriteTime;
        timeval postWriteTime;
        timeval preTakeTime;
        timeval postTakeTime;
        DDS::Duration_t waitTimeout = {1, 0};
        RoundTripModule::Sample data;
        data.payload.length(payloadSize);
        for(unsigned long i = 0; i < payloadSize; i++)
        {
            data.payload[i] = 'a';
        }
        DDS::ConditionSeq conditions;
        RoundTripModule::SampleSeq samples;
        DDS::SampleInfoSeq info;

        startTime = exampleGetTime();
        std::cout << "# Warming up..." << std::endl;
        bool warmUp = true;
        while(!terminated->get_trigger_value() && exampleTimevalToMicroseconds(exampleGetTime() - startTime) / US_IN_ONE_SEC < 5)
        {
            status = e.writer->write(data, DDS::HANDLE_NIL);
            CHECK_STATUS_MACRO(status);
            status = e.waitSet->wait(conditions, waitTimeout);
            if(status != DDS::RETCODE_TIMEOUT)
            {
                CHECK_STATUS_MACRO(status);
                status = e.reader->take(samples, info, DDS::LENGTH_UNLIMITED,
                                            DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
                CHECK_STATUS_MACRO(status);
                status = e.reader->return_loan(samples, info);
                CHECK_STATUS_MACRO(status);
            }
        }
        if(!terminated->get_trigger_value())
        {
            warmUp = false;
            std::cout << "# Warm up complete.\n" << std::endl;

            std::cout << "# Round trip measurements (in us)" << std::endl;
            std::cout << "#             Round trip time [us]         Write-access time [us]       Read-access time [us]" << std::endl;
            std::cout << "# Seconds     Count   median      min      Count   median      min      Count   median      min" << std::endl;
        }

        startTime = exampleGetTime();
        unsigned long elapsed = 0;
        for(unsigned long long i = 0; !terminated->get_trigger_value() && (!numSamples || i < numSamples); i++)
        {
            /** Write a sample that pong can send back */
            preWriteTime = exampleGetTime();
            status = e.writer->write(data, DDS::HANDLE_NIL);
            postWriteTime = exampleGetTime();
            CHECK_STATUS_MACRO(status);

            /** Wait for response from pong */
            status = e.waitSet->wait(conditions, waitTimeout);
            if(status != DDS::RETCODE_TIMEOUT)
            {
                CHECK_STATUS_MACRO(status);

                /** Take sample and check that it is valid */
                preTakeTime = exampleGetTime();
                status = e.reader->take(samples, info, DDS::LENGTH_UNLIMITED,
                                            DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
                postTakeTime = exampleGetTime();
                CHECK_STATUS_MACRO(status);

                if(!terminated->get_trigger_value())
                {
                    if(samples.length() != 1)
                    {
                        int length = 1024;
                        char* buffer = DDS::string_alloc(length);
                        DDS::String_var exception = buffer;
                        snprintf(buffer, length + 1, "%s%d%s", "ERROR: Ping received ", samples.length(),
                                " samples but was expecting 1. Are multiple pong applications running?");
                        throw exception;
                    }
                    else if(!info[0].valid_data)
                    {
                        DDS::String_var exception = DDS::string_dup("ERROR: Ping received an invalid sample. Has pong terminated already?");
                        throw exception;
                    }
                }
                status = e.reader->return_loan(samples, info);
                CHECK_STATUS_MACRO(status);

                /** Update stats */
                e.writeAccess += exampleTimevalToMicroseconds(postWriteTime - preWriteTime);
                e.readAccess += exampleTimevalToMicroseconds(postTakeTime - preTakeTime);
                e.roundTrip += exampleTimevalToMicroseconds(postTakeTime - preWriteTime);
                e.writeAccessOverall += exampleTimevalToMicroseconds(postWriteTime - preWriteTime);
                e.readAccessOverall += exampleTimevalToMicroseconds(postTakeTime - preTakeTime);
                e.roundTripOverall += exampleTimevalToMicroseconds(postTakeTime - preWriteTime);

                /** Print stats each second */
                if(exampleTimevalToMicroseconds(postTakeTime - startTime) > US_IN_ONE_SEC || (i && i == numSamples))
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
            else
            {
                elapsed += waitTimeout.sec;
            }
            if(timeOut && elapsed == timeOut)
            {
                status = terminated->set_trigger_value(true);
                CHECK_STATUS_MACRO(status);
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
    catch (const DDS::String_var& e)
    {
        std::cerr << e.in() << std::endl;
        result = 1;
    }
    return result;
}

/**
 * Runs the Pong role in this example.
 * @return 0 if a sample is successfully read, 1 otherwise.
 */
int pong(int argc, char *argv[])
{
    int result = 0;

    try
    {
        /** Initialise entities */
        Entities e("pong", "ping");

        std::cout << "Waiting for samples from ping to send back..." << std::endl;

        DDS::Duration_t waitTimeout = DDS::DURATION_INFINITE;
        DDS::ConditionSeq conditions;
        RoundTripModule::SampleSeq samples;
        DDS::SampleInfoSeq info;
        RoundTripModule::Sample data;
        while(!terminated->get_trigger_value())
        {
            /** Wait for a sample from ping */
            status = e.waitSet->wait(conditions, waitTimeout);
            CHECK_STATUS_MACRO(status);

            /** Take samples */
            status = e.reader->take(samples, info, DDS::LENGTH_UNLIMITED,
                                        DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
            CHECK_STATUS_MACRO(status);
            for (DDS::ULong i = 0; !terminated->get_trigger_value() && i < samples.length(); i++)
            {
                /** If writer has been disposed terminate pong */
                if(info[i].instance_state == DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE)
                {
                    std::cout << "Received termination request. Terminating." << std::endl;
                    terminated->set_trigger_value(true);
                    break;
                }
                /** If sample is valid, send it back to ping */
                else if(info[i].valid_data)
                {
                    status = e.writer->write(samples[i], DDS::HANDLE_NIL);
                    CHECK_STATUS_MACRO(status);
                }
            }
            status = e.reader->return_loan(samples, info);
            CHECK_STATUS_MACRO(status);
        }
    }
    catch (const DDS::String_var& e)
    {
        std::cerr << e.in() << std::endl;
        result = 1;
    }
    return result;
}

}
}
}
}

EXAMPLE_ENTRYPOINT(DCPS_SACPP_RoundTrip_ping, examples::dcps::RoundTrip::sacpp::ping)
EXAMPLE_ENTRYPOINT(DCPS_SACPP_RoundTrip_pong, examples::dcps::RoundTrip::sacpp::pong)
