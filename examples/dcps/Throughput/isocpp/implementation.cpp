
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
#include "entities.cpp"

#include "common/example_utilities.h"

#include <iomanip>

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace Throughput {

/**
 * @addtogroup examplesdcpsThroughputisocpp The ISO C++ DCPS API Throughput example
 *
 * The Throughput example measures data throughput in bytes per second. The publisher
 * allows you to specify a payload size in bytes as well as allowing you to specify
 * whether to send data in bursts. The publisher will continue to send data forever
 * unless a time out is specified. The subscriber will receive data and output the
 * total amount received and the data rate in bytes per second. It will also indicate
 * if any samples were received out of order. A maximum number of cycles can be
 * specified and once this has been reached the subscriber will terminate and output
 * totals and averages.
 * @ingroup examplesdcpsisocpp
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
         * Parameters: publisher [payloadSize] [burstInterval] [burstSize] [timeOut] [partitionName]
         */
        unsigned long payloadSize = 8192;
        unsigned long burstInterval = 0;
        unsigned long timeOut = 0;
        int burstSize = 1;
        std::string partitionName = "Throughput example";
        if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        {
            std::string exception
                       = "Usage (parameters must be supplied in order): \n"
                         "./publisher [payloadSize (bytes)] [burstInterval (ms)] [burstSize (samples)] [timeOut (seconds)] [partitionName]\n"
                         "Defaults: \n"
                         "./publisher 8192 0 1 0 \"Throughput example\"";
            throw exception;
        }
        if(argc > 1)
        {
            payloadSize = atoi(argv[1]); //The size of the payload in bytes
        }
        if(argc > 2)
        {
            burstInterval = atoi(argv[2]); //The time interval between each burst in ms
        }
        if(argc > 3)
        {
            burstSize = atoi(argv[3]); //The number of samples to send each burst
        }
        if(argc > 4)
        {
            timeOut = atoi(argv[4]); //The number of seconds the publisher should run for (0 = infinite)
        }
        if(argc > 5)
        {
            partitionName = argv[5]; //The name of the partition
        }

        std::cout << "payloadSize: " << payloadSize << " | burstInterval: " << burstInterval
            << " | burstSize: " << burstSize << " | timeOut: " << timeOut
            << " | partitionName: " << partitionName << "\n" << std::endl;

        /** Initialise entities */
        PubEntities e(partitionName);

        /** Fill the sample payload with data */
        ThroughputModule::Sample sample;
        sample.count() = 0;
        for(unsigned long i = 0; i < payloadSize; i++)
        {
            sample.payload().push_back('a');
        }

        /** Register the sample instance and write samples repeatedly or until time out */
        dds::core::InstanceHandle handle = e.writer.register_instance(sample);
        std::cout << "Writing samples..." << std::endl;
        int burstCount = 0;
        bool timedOut = false;
        timeval pubStart = exampleGetTime();
        timeval burstStart = exampleGetTime();
        while(!terminated.trigger_value() && !timedOut)
        {
            /** Write data until burst size has been reached */
            if(burstCount < burstSize)
            {
                e.writer.write(sample, handle);
                sample.count()++;
                burstCount++;
            }
            /** Sleep until burst interval has passed */
            else if(burstInterval)
            {
                timeval time = exampleGetTime();
                unsigned long deltaTime =
                    exampleTimevalToMicroseconds(time - burstStart) / US_IN_ONE_MS;
                if(deltaTime < burstInterval)
                {
                    exampleSleepMilliseconds(burstInterval - deltaTime);
                }
                burstStart = time + exampleMicrosecondsToTimeval((burstInterval - deltaTime) * US_IN_ONE_MS);
                burstCount = 0;
            }
            else
            {
                burstCount = 0;
            }

            if(timeOut)
            {
                if((exampleTimevalToMicroseconds(exampleGetTime() - pubStart) / US_IN_ONE_SEC) > timeOut)
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
 * This function calculates the number of samples received
 *
 * @param count the map tracking sample count values
 * @param startCount the map tracking sample count start or previous values
 * @param prevCount if set to true, count2's value should be set to count1 after adding to total
 * @return the number of samples received
 */
unsigned long long samplesReceived(std::map<dds::core::InstanceHandle, unsigned long long>& count1,
    std::map<dds::core::InstanceHandle, unsigned long long>& count2, bool prevCount = false)
{
    unsigned long long total = 0;
    for(std::map<dds::core::InstanceHandle, unsigned long long>::iterator it = count1.begin(); it != count1.end(); it++)
    {
        total += it->second - count2[it->first];
        if(prevCount)
        {
            count2[it->first] = it->second;
        }
    }
    return total;
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
         * Parameters: publisher [maxCycles] [pollingDelay] [partitionName]
         */
        unsigned long long maxCycles = 0;
        unsigned long pollingDelay = 0;
        std::string partitionName = "Throughput example";
        if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        {
            std::string exception
                       = "Usage (parameters must be supplied in order): \n"
                         "./subscriber [maxCycles (0 = infinite)] [pollingDelay (ms, 0 = event based)] [partitionName]\n"
                         "Defaults: \n"
                         "./subscriber 0 0 \"Throughput example\"";
            throw exception;
        }
        if(argc > 1)
        {
            maxCycles = atoi(argv[1]); //The number of times to output statistics before terminating (0 = infinite)
        }
        if(argc > 2)
        {
            pollingDelay = atoi(argv[2]); //The number of ms to wait between reads (0 = event based)
        }
        if(argc > 3)
        {
            partitionName = argv[3]; //The name of the partition
        }

        std::cout << "maxCycles: " << maxCycles << " | pollingDelay: " << pollingDelay
            << " | partitionName: " << partitionName << "\n" << std::endl;

        /** Initialise entities */
        SubEntities e(partitionName);

        std::map<dds::core::InstanceHandle, unsigned long long> count;
        std::map<dds::core::InstanceHandle, unsigned long long> startCount;
        std::map<dds::core::InstanceHandle, unsigned long long> prevCount;
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
        while(!terminated.trigger_value() && (maxCycles == 0 || cycles < maxCycles))
        {
            /** If polling delay is set */
            if(pollingDelay)
            {
                /** Sleep before polling again */
                exampleSleepMilliseconds(pollingDelay);
            }
            else
            {
                /** Wait for samples */
                e.waitSet.wait();
            }

            /** Take samples and iterate through them */
            dds::sub::LoanedSamples<ThroughputModule::Sample> samples
                = e.reader.take();
            for (dds::sub::LoanedSamples<ThroughputModule::Sample>::const_iterator sample
                    = samples.begin(); !terminated.trigger_value() && sample < samples.end(); ++sample)
            {
                if(sample->info().valid())
                {
                    dds::core::InstanceHandle ph = sample->info().publication_handle();
                    /** Check that the sample is the next one expected */
                    if(!startCount[ph] && !count[ph])
                    {
                        count[ph] = sample->data().count();
                        startCount[ph] = sample->data().count();
                    }
                    if(sample->data().count() != count[ph])
                    {
                        outOfOrder++;
                    }
                    count[ph] = sample->data().count() + 1;

                    /** Add the sample payload size to the total received */
                    payloadSize = sample->data().payload().size();
                    received += payloadSize + 8;
                }
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
                                << "Total received: " << samplesReceived(count, startCount) << " samples, "
                                << received << " bytes | "
                                << "Out of order: " << outOfOrder << " samples | "
                                << "Transfer rate: " << (double)samplesReceived(count, prevCount, true) / deltaTime << " samples/s, "
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
                prevReceived = received;
                prevTime = time;
            }
        }

        /** Output totals and averages */
        double deltaTime = (double)exampleTimevalToMicroseconds(time - startTime) / US_IN_ONE_SEC;
        std::cout << std::fixed << std::setprecision(2)
                  << "\nTotal received: " << samplesReceived(count, startCount) << " samples, "
                  << received << " bytes\n"
                  << "Out of order: " << outOfOrder << " samples\n"
                  << "Average transfer rate: " << (double)samplesReceived(count, startCount) / deltaTime << " samples/s, "
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

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Throughput_publisher, examples::dcps::Throughput::isocpp::publisher)
EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Throughput_subscriber, examples::dcps::Throughput::isocpp::subscriber)
