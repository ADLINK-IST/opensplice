
/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
 * @addtogroup examplesstreamsThroughputsacpp The Standalone C++ Streams API Throughput example
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
 * @ingroup examplesstreamssacpp
 */
/** @{*/
/** @dir */
/** @file */

namespace sacpp  {

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
    if (reader) {
        reader->interrupt();
    }
    return true; //Don't let other handlers handle this key
}
#else
static void CtrlHandler(int fdwCtrlType)
{
    terminated->set_trigger_value(true);
    if (reader) {
        reader->interrupt();
    }
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
    DDS::ReturnCode_t status;
    try
    {
        /**
         * Get the program parameters
         * Parameters: publisher [payloadSize (bytes)] [flushMaxSamples (samples)] [flushTimeOut (ms, 0 = infinite)] [programTimeOut (seconds, 0 = infinite)] [partitionName]
         */
        unsigned long payloadSize = 32;
        DDS::Duration_t flushTimeOut = DDS::DURATION_INFINITE;
        unsigned long flushMaxSamples = 32;
        unsigned long programTimeOut = 0;
        DDS::String_var partitionName = DDS::string_dup("Streams Throughput example");
        if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        {
            DDS::String_var exception = DDS::string_dup(
                "Usage (parameters must be supplied in order): \n"
                "./publisher [payloadSize (bytes)] [flushMaxSamples (samples)] [flushTimeOut (ms, 0 = infinite)] [programTimeOut (seconds, 0 = infinite)] [partitionName]\n"
                "Defaults: \n"
                "./publisher 32 32 0 0 \"Streams Throughput example\"");
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
                //The duration before the stream is automatically flushed in ms (0 = infinite)
                flushTimeOut.sec = (unsigned long)(ms / 1000);
                flushTimeOut.nanosec = (unsigned long)((ms % 1000) * 1000000);
            }
        }
        if(argc > 4)
        {
            programTimeOut = atoi(argv[4]); //The number of seconds the publisher should run for (0 = infinite)
        }
        if(argc > 5)
        {
            partitionName = DDS::string_dup(argv[5]); //The name of the partition
        }

        std::cout << "payloadSize: " << payloadSize << " | flushMaxSamples: " << flushMaxSamples
        << " | flushTimeOut: " << ms << " | programTimeOut: " << programTimeOut
        << " | partitionName: " << partitionName << "\n" << std::endl;

        /** Initialise entities */
        PubEntities e(partitionName, flushTimeOut, flushMaxSamples);

        /** Fill the sample payload with data */
        ThroughputModule::DataType sample;
        sample.count = 0;
        sample.payload.length(payloadSize);
        for(unsigned long i = 0; i < payloadSize; i++)
        {
            sample.payload[i] = 'a';
        }

        /** Register the sample instance and write samples repeatedly or until time out */
        std::cout << "Writing samples..." << std::endl;
        bool timedOut = false;
        bool flushNeeded = false;
        timeval pubStart = exampleGetTime();
        while(!terminated->get_trigger_value() && !timedOut)
        {
            if (!flushNeeded) {
                status = e.writer->append(0, sample);
                if (status != DDS::RETCODE_TIMEOUT) {
                    CHECK_STATUS_MACRO(status);
                    sample.count++;
                } else {
                    flushNeeded = true;
                }
            } else {
                status = e.writer->flush(0);
                if (status != DDS::RETCODE_TIMEOUT) {
                    CHECK_STATUS_MACRO(status);
                    flushNeeded = false;
                }
            }

            if(programTimeOut)
            {
                if((exampleTimevalToMicroseconds(exampleGetTime() - pubStart) / US_IN_ONE_SEC) > programTimeOut)
                {
                    timedOut = true;
                }
            }
        }

        if(terminated->get_trigger_value())
        {
            std::cout << "Terminated, " << sample.count << " samples written." << std::endl;
        }
        else
        {
            std::cout << "Timed out, " << sample.count << " samples written." << std::endl;
        }
        reader = NULL;
    }
    catch(const DDS::String_var& e)
    {
        std::cerr << e.in() << std::endl;
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
    DDS::ReturnCode_t status;
    try
    {
        /**
         * Get the program parameters
         * Parameters: publisher [maxSamplesPerRead (samples)] [maxCycles (0 = infinite)] [pollingDelay (ms, 0 = event based)] [partitionName]
         */
        unsigned long maxSamplesPerRead = 32;
        unsigned long long maxCycles = 0;
        unsigned long pollingDelay = 0;
        DDS::String_var partitionName = DDS::string_dup("Streams Throughput example");
        if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        {
            DDS::String_var exception = DDS::string_dup(
                "Usage (parameters must be supplied in order): \n"
                "./subscriber [maxSamplesPerRead (samples)] [maxCycles (0 = infinite)] [pollingDelay (ms, 0 = event based)] [partitionName]\n"
                "Defaults: \n"
                "./subscriber 32 0 0 \"Streams Throughput example\"");
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
            partitionName = DDS::string_dup(argv[4]); //The name of the partition
        }

        std::cout << "maxSamplesPerRead: " << maxSamplesPerRead << " | maxCycles: " << maxCycles << " | pollingDelay: " << pollingDelay
        << " | partitionName: " << partitionName << "\n" << std::endl;

        /** Initialise entities */
        SubEntities e(partitionName);

        /** In case of event-based reading, store a ref to the reader so it can be interrupted */
        if (pollingDelay == 0) {
            reader = e.reader.in();
        }

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
        ThroughputModule::DataTypeStreamBuf_var samples = new ThroughputModule::DataTypeStreamBuf(maxSamplesPerRead);
        while(!terminated->get_trigger_value() && (maxCycles == 0 || cycles < maxCycles))
        {
            /** If polling delay is set */
            if(pollingDelay)
            {
                /** Sleep before polling again */
                exampleSleepMilliseconds(pollingDelay);

                status = e.reader->get(0, samples, maxSamplesPerRead, DDS::DURATION_ZERO);
                CHECK_STATUS_MACRO(status);
            }
            else
            {
                /** Wait for samples */
                status = e.reader->get(0, samples, maxSamplesPerRead, DDS::DURATION_INFINITE);
                CHECK_STATUS_MACRO(status);
            }

            /** Iterate through samples */
            for (DDS::ULong i = 0; !terminated->get_trigger_value() && i < samples->length(); i++)
            {
                /** Check that the sample is the next one expected */
                if(!startCount && !count)
                {
                    count = samples[i].count;
                    prevCount = samples[i].count;
                    startCount = samples[i].count;
                }
                if(samples[i].count != count)
                {
                    outOfOrder++;
                }
                count = samples[i].count + 1;

                /** Add the sample payload size to the total received */
                payloadSize = samples[i].payload.length();
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

//        reader = NULL;
    }
    catch(const DDS::String_var& e)
    {
        std::cerr << e.in() << std::endl;
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

EXAMPLE_ENTRYPOINT(DCPS_SACPP_Streams_Throughput_publisher, examples::streams::Throughput::sacpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_SACPP_Streams_Throughput_subscriber, examples::streams::Throughput::sacpp::subscriber)
