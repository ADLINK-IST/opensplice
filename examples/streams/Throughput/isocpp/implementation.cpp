/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "implementation.hpp"
#include "entities.cpp"

#include "common/example_utilities.h"

#include <iomanip>

namespace examples {
#ifdef GENERATING_STREAMS_EXAMPLE_DOXYGEN
GENERATING_STREAMS_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace streams { namespace Throughput {

/**
* @addtogroup examplesstreamsThroughputisocpp The ISO C++ Streams API Throughput example
*
* The Streams Throughput example measures data throughput in bytes per second using
* the Streams API to reduce overhead and maximise the transfer rate.
* The publisher allows you to specify a payload size in bytes as well as allowing
* you to specify when to flush the stream after either a specified number of samples
* has been sent or after a time out duration. The publisher will continue to send data
* forever unless a program time out is specified. The subscriber will receive data and
* output the total amount received and the data rate in bytes per second. It will also
* indicate if any samples were received out of order. The maximum samples per read can
* be varied, for best perormance this should match the flush max samples. A maximum number
* of cycles can be specified and once this has been reached the subscriber will terminate
* and output totals and averages.
* @ingroup examplesstreamsisocpp
*/
/** @{*/
/** @dir */
/** @file */
namespace isocpp  {

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

#define BYTES_PER_SEC_TO_MEGABITS_PER_SEC 125000

/**
* This function performs the publisher role in this example.
* @return 0 if a sample is successfully written, 1 otherwise.
*/
int publisher(int argc, char *argv[])
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
        /**
        * Get the program parameters
        * Parameters: publisher [payloadSize (bytes)] [flushMaxSamples (samples)] [flushTimeOut (ms, 0 = infinite)] [programTimeOut (seconds, 0 = infinite)] [partitionName]
        */
        unsigned long payloadSize = 32;
        dds::core::Duration flushTimeOut = dds::core::Duration::infinite();
        unsigned long flushMaxSamples = 32;
        unsigned long programTimeOut = 0;
        std::string partitionName = "Streams Throughput example";
        if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        {
            std::string exception
            = "Usage (parameters must be supplied in order): \n"
            "./publisher [payloadSize (bytes)] [flushMaxSamples (samples)] [flushTimeOut (ms, 0 = infinite)] [programTimeOut (seconds, 0 = infinite)] [partitionName]\n"
            "Defaults: \n"
            "./publisher 32 32 0 0 \"Streams Throughput example\"";
            throw exception;
        }
        if(argc > 1)
        {
            payloadSize = atoi(argv[1]); //The size of the payload in bytes
        }
        unsigned long long ms = 0;
        if(argc > 2)
        {
            flushMaxSamples = atoi(argv[2]); //The number of samples to cache before the stream is automatically flushed
        }
        if(argc > 3)
        {
            ms = atoi(argv[3]);
            if(ms != 0)
            {
                flushTimeOut = dds::core::Duration::from_millisecs(ms); //The duration before the stream is automatically flushed in ms (0 = infinite)
            }
        }
        if(argc > 4)
        {
            programTimeOut = atoi(argv[4]); //The number of seconds the publisher should run for (0 = infinite)
        }
        if(argc > 5)
        {
            partitionName = argv[5]; //The name of the partition
        }

        std::cout << "payloadSize: " << payloadSize << " | flushMaxSamples: " << flushMaxSamples
        << " | flushTimeOut: " << ms << " | programTimeOut: " << programTimeOut
        << " | partitionName: " << partitionName << "\n" << std::endl;

        /** Initialise entities */
        PubEntities e(partitionName, flushTimeOut, flushMaxSamples);

        /** Fill the sample payload with data */
        ThroughputModule::DataType sample;
        sample.count() = 0;
        for(unsigned long i = 0; i < payloadSize; i++)
        {
            sample.payload().push_back('a');
        }

        /** Register the sample instance and write samples repeatedly or until time out */
        std::cout << "Writing samples..." << std::endl;
        bool timedOut = false;
        timeval pubStart = exampleGetTime();
        bool doFlush = false;

        while(!terminated.trigger_value() && !timedOut)
        {
            try {
                if (!doFlush) {
                    e.writer.append(sample);
                    sample.count()++;
                } else {
                    e.writer.flush();
                    doFlush = false;
                }
            } catch(const dds::core::TimeoutError&) {
                std::cout << "Timeout occurred" << std::endl;
                doFlush = true;
            }

            if(programTimeOut)
            {
                if((exampleTimevalToMicroseconds(exampleGetTime() - pubStart) / US_IN_ONE_SEC) > programTimeOut)
                {
                    timedOut = true;
                }
            }
        }

        if(terminated.trigger_value())
        {
            std::cout << "Terminated, " << sample.count() << " samples written." << std::endl;
        }
        else
        {
            std::cout << "Timed out, " << sample.count() << " samples written." << std::endl;
        }
    }
    catch(const dds::core::Exception& e)
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
* This function performs the subscriber role in this example.
* @return 0 if a sample is successfully read, 1 otherwise.
*/
int subscriber(int argc, char *argv[])
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
        /**
        * Get the program parameters
        * Parameters: publisher [maxSamplesPerRead (samples)] [maxCycles (0 = infinite)] [pollingDelay (ms, 0 = event based)] [partitionName]
        */
        unsigned long maxSamplesPerRead = 32;
        unsigned long long maxCycles = 0;
        unsigned long pollingDelay = 0;
        std::string partitionName = "Streams Throughput example";
        if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        {
            std::string exception
            = "Usage (parameters must be supplied in order): \n"
            "./subscriber [maxSamplesPerRead (samples)] [maxCycles (0 = infinite)] [pollingDelay (ms, 0 = event based)] [partitionName]\n"
            "Defaults: \n"
            "./subscriber 32 0 0 \"Streams Throughput example\"";
            throw exception;
        }
        if(argc > 1)
        {
            maxSamplesPerRead = atoi(argv[1]); //The number of samples to read
        }
        if(argc > 2)
        {
            maxCycles = atoi(argv[2]); //The number of times to output statistics before terminating (0 = infinite)
        }
        if(argc > 3)
        {
            pollingDelay = atoi(argv[3]); //The number of ms to wait between reads (0 = event based)
        }
        if(argc > 4)
        {
            partitionName = argv[4]; //The name of the partition
        }

        std::cout << "maxSamplesPerRead: " << maxSamplesPerRead << " | maxCycles: " << maxCycles << " | pollingDelay: " << pollingDelay
        << " | partitionName: " << partitionName << "\n" << std::endl;

        /** Initialise entities */
        SubEntities e(partitionName);

        unsigned long long count = 0;
        unsigned long long startCount = 0;
        unsigned long long prevCount = 0;
        unsigned long long outOfOrder = 0;
        unsigned long long received = 0;
        unsigned long long prevReceived = 0;
        timeval time = {0, 0};
        timeval startTime = {0, 0};
        timeval prevTime = {0, 0};
        unsigned long payloadSize = 0;

        std::cout << "Waiting for samples..." << std::endl;

        /** Loop through until the maxCycles has been reached (0 = infinite) */
        unsigned long long cycles = 0;
        dds::streams::sub::StreamLoanedSamples<ThroughputModule::DataType> samples;
        while(!terminated.trigger_value() && (maxCycles == 0 || cycles < maxCycles))
        {
            /** If polling delay is set */
            if(pollingDelay)
            {
                /** Sleep before polling again */
                exampleSleepMilliseconds(pollingDelay);

                samples = e.reader.select().timeout(dds::core::Duration::zero()).max_samples(maxSamplesPerRead).get();
            }
            else
            {
                /** Wait for samples */
                samples = e.reader.select().timeout(dds::core::Duration::infinite()).max_samples(maxSamplesPerRead).get();
            }

            /** Iterate through samples */
            for (dds::streams::sub::StreamLoanedSamples<ThroughputModule::DataType>::const_iterator sample
                = samples.begin(); !terminated.trigger_value() && sample < samples.end(); ++sample)
            {
                /** Check that the sample is the next one expected */
                if(!startCount && !count)
                {
                    count = sample->data().count();
                    prevCount = sample->data().count();
                    startCount = sample->data().count();
                }
                if(sample->data().count() != count)
                {
                    outOfOrder++;
                }
                count = sample->data().count() + 1;

                /** Add the sample payload size to the total received */
                payloadSize = sample->data().payload().size();
                received += payloadSize + 8;
            }

            /** Check that at least one second has passed since the last output */
            time = exampleGetTime();
            if(exampleTimevalToMicroseconds(time) >
                exampleTimevalToMicroseconds(prevTime) + US_IN_ONE_SEC)
            {
                /** If not the first iteration */
                if(exampleTimevalToMicroseconds(prevTime))
                {
                    /**
                    * Calculate the samples and bytes received and the time passed since the
                    * last iteration and output
                    */
                    unsigned long long deltaReceived = received - prevReceived;
                    double deltaTime = (double)exampleTimevalToMicroseconds(time - prevTime) / US_IN_ONE_SEC;

                    std::cout << std::fixed << std::setprecision(2)
                    << "Payload size: " << payloadSize << " | "
                    << "Total received: " << count - startCount << " samples, "
                    << received << " bytes | "
                    << "Out of order: " << outOfOrder << " samples | "
                    << "Transfer rate: " << (double)(count - prevCount) / deltaTime << " samples/s, "
                    << ((double)deltaReceived / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime << " Mbit/s"
                    << std::endl;

                    cycles++;
                }
                else
                {
                    /** Set the start time if it is the first iteration */
                    startTime = time;
                }
                /** Update the previous values for next iteration */
                prevCount = count;
                prevReceived = received;
                prevTime = time;
            }
        }

        /** Output totals and averages */
        double deltaTime = (double)exampleTimevalToMicroseconds(time - startTime) / US_IN_ONE_SEC;
        std::cout << std::fixed << std::setprecision(2)
        << "\nTotal received: " << count - startCount << " samples, "
        << received << " bytes\n"
        << "Out of order: " << outOfOrder << " samples\n"
        << "Average transfer rate: " << (double)(count - startCount) / deltaTime << " samples/s, "
        << ((double)received / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime << " Mbit/s"
        << std::endl;
    }
    catch(const dds::core::Exception& e)
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

}
}
}
}

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Streams_Throughput_publisher, examples::streams::Throughput::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Streams_Throughput_subscriber, examples::streams::Throughput::isocpp::subscriber)
