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
#include "implementation.h"

#include "common/example_utilities.h"
#include "common/example_error_sac.h"

#include <stdio.h>
#include <stdlib.h>

#include "Throughput.h"

#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif

/**
 * @addtogroup examplesdcpsThroughputsac The Standalone C DCPS API Throughput example
 *
 * The Throughput example measures data throughput in bytes per second. The publisher
 * allows you to specify a payload size in bytes as well as allowing you to specify
 * whether to send data in bursts. The publisher will continue to send data forever
 * unless a time out is specified. The subscriber will receive data and output the
 * total amount received and the data rate in bytes per second. It will also indicate
 * if any samples were received out of order. A maximum number of cycles can be
 * specified and once this has been reached the subscriber will terminate and output
 * totals and averages.
 * @ingroup examplesdcpssac
 */
/** @{*/
/** @dir */
/** @file */
DDS_ReturnCode_t status;
static DDS_GuardCondition terminated;

#ifndef _WIN32
struct sigaction oldAction;
#endif

/*
 * Function to handle Ctrl-C presses.
 * @param fdwCtrlType Ctrl signal type
 */
#ifdef _WIN32
static int CtrlHandler(DWORD fdwCtrlType)
{
    status = DDS_GuardCondition_set_trigger_value(terminated, TRUE);
    CHECK_STATUS_MACRO(status);
    return TRUE; /* Don't let other handlers handle this key */
}
#else
static void CtrlHandler(int fdwCtrlType)
{
    status = DDS_GuardCondition_set_trigger_value(terminated, TRUE);
    CHECK_STATUS_MACRO(status);
}
#endif

/**
 * This struct serves as a container holding initialised entities used by publisher
 */
typedef struct PubEntities {
    /** The DomainParticipantFactory used by the publisher */
    DDS_DomainParticipantFactory domainParticipantFactory;
    /** The DomainParticipant used by the publisher */
    DDS_DomainParticipant participant;
    /** The TypeSupport for the sample */
    ThroughputModule_DataTypeTypeSupport typeSupport;
    /** The Topic used by the publisher */
    DDS_Topic topic;
    /** The Publisher used by the publisher */
    DDS_Publisher publisher;
    /** The DataWriter used by the publisher */
    ThroughputModule_DataTypeDataWriter writer;
} PubEntities;

/**
 * This struct serves as a container holding initialised entities used by publisher
 */
typedef struct SubEntities {
    /** The DomainParticipantFactory used by the subscriber */
    DDS_DomainParticipantFactory domainParticipantFactory;
    /** The DomainParticipant used by the subscriber */
    DDS_DomainParticipant participant;
    /** The TypeSupport for the sample */
    ThroughputModule_DataTypeTypeSupport typeSupport;
    /** The Topic used by the subscriber */
    DDS_Topic topic;
    /** The Subscriber used by the subscriber */
    DDS_Subscriber subscriber;
    /** The DataReader used by the subscriber */
    ThroughputModule_DataTypeDataReader reader;
    /** The WaitSet used by the subscriber */
    DDS_WaitSet waitSet;
    /** The StatusCondition used by the subscriber,
     * triggered when data is available to read */
    DDS_StatusCondition dataAvailable;
} SubEntities;

typedef struct HandleEntry {
    DDS_InstanceHandle_t handle;
    DDS_unsigned_long_long count;
} HandleEntry;

typedef struct HandleMap {
    unsigned long size;
    unsigned long max_size;
    HandleEntry *entries;
} HandleMap;

static HandleMap* HandleMap__alloc()
{
    HandleMap *map = malloc(sizeof(*map));
    CHECK_ALLOC_MACRO(map);
    map->size = 0;
    map->max_size = 10;
    map->entries = malloc(sizeof(HandleEntry) * map->max_size);
    CHECK_ALLOC_MACRO(map->entries);

    return map;
}

static void HandleMap__free(HandleMap *map)
{
    free(map->entries);
    free(map);
}

static HandleEntry* store_handle(HandleMap *map, DDS_InstanceHandle_t key)
{
    if (map->size == map->max_size) {
        map->entries = realloc(map->entries, sizeof(HandleEntry) * (map->max_size + 10));
        CHECK_ALLOC_MACRO(map->entries);
        map->max_size += 10;
    }

    map->entries[map->size].handle = key;
    map->entries[map->size].count = 0;
    map->size++;

    return &(map->entries[map->size - 1]);
}

static HandleEntry* retrieve_handle(HandleMap *map, DDS_InstanceHandle_t key)
{
    HandleEntry *entry = NULL;
    unsigned long i;

    for (i = 0; i < map->size; i++) {
        entry = &(map->entries[i]);
        if (entry->handle == key) {
            break;
        }
    }
    return entry;
}

static void copy_handles(HandleMap *from, HandleMap *to)
{
    unsigned long i;
    HandleEntry *fromHandle, *toHandle;

    for (i=0; i < from->size; i++) {
        fromHandle = &(from->entries[i]);
        toHandle = retrieve_handle(to, fromHandle->handle);
        if (!toHandle) {
            toHandle = store_handle(to, fromHandle->handle);
        }
        toHandle->count = fromHandle->count;
    }
}

#define BYTES_PER_SEC_TO_MEGABITS_PER_SEC 125000

/**
 * This function performs the publisher role in this example.
 * @return 0 if a sample is successfully written, 1 otherwise.
 */
int publisher(int argc, char **argv)
{
    int result = EXIT_SUCCESS;
    unsigned long payloadSize = 4096;
    unsigned long burstInterval = 0;
    unsigned long timeOut = 0;
    int burstSize = 1;
    char *partitionName = "Throughput example";
    PubEntities *e = malloc(sizeof(*e));
    ThroughputModule_DataType sample;

    /* Register handler for Ctrl-C */
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
#else
    struct sigaction sat;
    sat.sa_handler = CtrlHandler;
    sigemptyset(&sat.sa_mask);
    sat.sa_flags = 0;
    sigaction(SIGINT, &sat, &oldAction);
#endif
    terminated = DDS_GuardCondition__alloc();

    sample.payload._buffer = NULL;
    /**
     * Get the program parameters
     * Parameters: publisher [payloadSize] [burstInterval] [burstSize] [timeOut] [partitionName]
     */
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage (parameters must be supplied in order):\n");
        printf("./publisher [payloadSize (bytes)] [burstInterval (ms)] [burstSize (samples)] [timeOut (seconds)] [partitionName]\n");
        printf("Defaults:\n");
        printf("./publisher 4096 0 1 0 \"Throughput example\"\n");
        free(e);
        DDS_free(terminated);
        return result;
    }
    if (argc > 1) {
        payloadSize = atoi(argv[1]); /* The size of the payload in bytes */
    }
    if (argc > 2) {
        burstInterval = atoi(argv[2]); /* The time interval between each burst in ms */
    }
    if (argc > 3) {
        burstSize = atoi(argv[3]); /* The number of samples to send each burst */
    }
    if (argc > 4) {
        timeOut = atoi(argv[4]); /* The number of seconds the publisher should run for (0 = infinite) */
    }
    if (argc > 5) {
        partitionName = argv[5]; /* The name of the partition */
    }

    printf("payloadSize: %lu | burstInterval: %lu | burstSize: %d | timeOut: %lu | partitionName: %s\n",
        payloadSize, burstInterval, burstSize, timeOut, partitionName);

    /** Initialise entities */
    {
        DDS_PublisherQos *pubQos;
        DDS_DataWriterQos *dwQos;
        DDS_string typename;

        /** A DDS_DomainParticipant is created for the default domain. */
        e->domainParticipantFactory = DDS_DomainParticipantFactory_get_instance();
        CHECK_HANDLE_MACRO(e->domainParticipantFactory);
        e->participant = DDS_DomainParticipantFactory_create_participant(
            e->domainParticipantFactory, DDS_DOMAIN_ID_DEFAULT, DDS_PARTICIPANT_QOS_DEFAULT, NULL, 0);
        CHECK_HANDLE_MACRO(e->participant);

        /** The sample type is created and registered */
        e->typeSupport = ThroughputModule_DataTypeTypeSupport__alloc();
        typename = ThroughputModule_DataTypeTypeSupport_get_type_name(e->typeSupport);
        status = ThroughputModule_DataTypeTypeSupport_register_type(e->typeSupport, e->participant, typename);
        CHECK_STATUS_MACRO(status);

        /** A DDS_Topic is created for our sample type on the domain participant. */
        e->topic = DDS_DomainParticipant_create_topic(
            e->participant, "Throughput", typename, DDS_TOPIC_QOS_DEFAULT, NULL, 0);
        CHECK_HANDLE_MACRO(e->topic);
        DDS_free(typename);

        /** A DDS_Publisher is created on the domain participant. */
        pubQos = DDS_PublisherQos__alloc();
        status = DDS_DomainParticipant_get_default_publisher_qos(e->participant, pubQos);
        CHECK_STATUS_MACRO(status);
        pubQos->partition.name._release = TRUE;
        pubQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
        pubQos->partition.name._length = 1;
        pubQos->partition.name._maximum = 1;
        pubQos->partition.name._buffer[0] = DDS_string_dup(partitionName);
        e->publisher = DDS_DomainParticipant_create_publisher(e->participant, pubQos, NULL, 0);
        CHECK_HANDLE_MACRO(e->publisher);
        DDS_free(pubQos);

        /** A DDS_DataWriter is created on the Publisher & Topic with a modififed Qos. */
        dwQos = DDS_DataWriterQos__alloc();
        status = DDS_Publisher_get_default_datawriter_qos(e->publisher, dwQos);
        CHECK_STATUS_MACRO(status);
        dwQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        dwQos->reliability.max_blocking_time.sec = 10;
        dwQos->reliability.max_blocking_time.nanosec = 0;
        dwQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        dwQos->resource_limits.max_samples = 100;
        e->writer = DDS_Publisher_create_datawriter(e->publisher, e->topic, dwQos, NULL, 0);
        CHECK_HANDLE_MACRO(e->writer);
        DDS_free(dwQos);
    }

    /** Fill the sample payload with data */
    {
        unsigned long i;

        sample.count = 0;
        sample.payload._buffer = DDS_sequence_octet_allocbuf(payloadSize);
        sample.payload._length = payloadSize;
        sample.payload._maximum = payloadSize;
        for (i = 0; i < payloadSize; i++) {
            sample.payload._buffer[i] = 'a';
        }
    }

    /* Register the sample instance and write samples repeatedly or until time out */
    {
        DDS_InstanceHandle_t handle;
        struct timeval pubStart, burstStart;
        int burstCount = 0;
        int timedOut = FALSE;

        handle = ThroughputModule_DataTypeDataWriter_register_instance(e->writer, &sample);
        printf("Writing samples...\n");
        pubStart = exampleGetTime();
        burstStart = exampleGetTime();

        while (!DDS_GuardCondition_get_trigger_value(terminated) && !timedOut) {
            /** Write data until burst size has been reached */
            if (burstCount < burstSize) {
                status = ThroughputModule_DataTypeDataWriter_write(e->writer, &sample, handle);
                if (!DDS_GuardCondition_get_trigger_value(terminated)) {
                    CHECK_STATUS_MACRO(status);
                }
                sample.count++;
                burstCount++;
            }
            /** Sleep until burst interval has passed */
            else if(burstInterval) {
                struct timeval time = exampleGetTime();
                struct timeval deltaTv = exampleSubtractTimevalFromTimeval(&time, &burstStart);
                unsigned long deltaTime = exampleTimevalToMicroseconds(&deltaTv) / US_IN_ONE_MS;
                if (deltaTime < burstInterval) {
                    exampleSleepMilliseconds(burstInterval - deltaTime);
                }
                burstStart = exampleGetTime();
                burstCount = 0;
            }
            else {
                burstCount = 0;
            }

            if (timeOut) {
                struct timeval now = exampleGetTime();
                struct timeval deltaTv = exampleSubtractTimevalFromTimeval(&now, &pubStart);
                if ((exampleTimevalToMicroseconds(&deltaTv) / US_IN_ONE_SEC) > timeOut) {
                    timedOut = TRUE;
                }
            }
        }

        if (DDS_GuardCondition_get_trigger_value(terminated)) {
            printf("Terminated, %llu samples written.\n", sample.count);
        } else {
            printf("Timed out, %llu samples written.\n", sample.count);
        }
    }

    /** Cleanup entities */
    if (e->participant) {
        status = DDS_DomainParticipant_delete_contained_entities(e->participant);
        CHECK_STATUS_MACRO(status);
        status = DDS_DomainParticipantFactory_delete_participant(e->domainParticipantFactory, e->participant);
        CHECK_STATUS_MACRO(status);
        DDS_free(e->typeSupport);
    }
    DDS_free(sample.payload._buffer);
    DDS_free(terminated);
    free(e);

#ifdef _WIN32
    SetConsoleCtrlHandler(0, FALSE);
#else
    sigaction(SIGINT,&oldAction, 0);
#endif
    return result;
}

/**
 * This function calculates the number of samples received
 *
 * @param count1 the map tracking sample count values
 * @param count2 the map tracking sample count start or previous values
 * @param prevCount if set to true, count2's value should be set to count1 after adding to total
 * @return the number of samples received
 */
unsigned long long samplesReceived(HandleMap *count1, HandleMap *count2, int prevCount)
{
    unsigned long long total = 0;
    unsigned long i;
    HandleEntry *pubCount1, *pubCount2;

    for (i = 0; i < count1->size; i++) {
        pubCount1 = &(count1->entries[i]);
        pubCount2 = retrieve_handle(count2, pubCount1->handle);
        if (!pubCount2) {
            pubCount2 = store_handle(count2, pubCount1->handle);
        }
        total += pubCount1->count - pubCount2->count;
        if (prevCount) {
            pubCount2->count = pubCount1->count;
        }
    }
    return total;
}

int subscriber(int argc, char **argv)
{
    int result = EXIT_SUCCESS;
    unsigned long long maxCycles = 0;
    unsigned long pollingDelay = 0;
    char *partitionName = "Throughput example";
    SubEntities *e = malloc(sizeof(*e));

    /* Register handler for Ctrl-C */
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
#else
    struct sigaction sat;
    sat.sa_handler = CtrlHandler;
    sigemptyset(&sat.sa_mask);
    sat.sa_flags = 0;
    sigaction(SIGINT, &sat, &oldAction);
#endif
    terminated = DDS_GuardCondition__alloc();

    /**
     * Get the program parameters
     * Parameters: subscriber [maxCycles] [pollingDelay] [partitionName]
     */
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage (parameters must be supplied in order):\n");
        printf("./subscriber [maxCycles (0 = infinite)] [pollingDelay (ms, 0 = event based)] [partitionName]\n");
        printf("Defaults:\n");
        printf("./subscriber 0 0 \"Throughput example\"\n");
        free(e);
        DDS_free(terminated);
        return result;
    }

    if (argc > 1) {
        maxCycles = atoi(argv[1]); /* The number of times to output statistics before terminating */
    }
    if (argc > 2) {
        pollingDelay = atoi(argv[2]); /* The number of ms to wait between reads (0 = event based) */
    }
    if (argc > 3) {
        partitionName = argv[3]; /* The name of the partition */
    }

    printf("maxCycles: %llu | pollingDelay: %lu | partitionName: %s\n",
        maxCycles, pollingDelay, partitionName);

    /** Initialise entities */
    {
        DDS_SubscriberQos *subQos;
        DDS_DataReaderQos *drQos;
        DDS_string typename;

        /** A DDS_DomainParticipant is created for the default domain. */
        e->domainParticipantFactory = DDS_DomainParticipantFactory_get_instance();
        CHECK_HANDLE_MACRO(e->domainParticipantFactory);
        e->participant = DDS_DomainParticipantFactory_create_participant(
            e->domainParticipantFactory, DDS_DOMAIN_ID_DEFAULT, DDS_PARTICIPANT_QOS_DEFAULT, NULL, 0);
        CHECK_HANDLE_MACRO(e->participant);

        /** The sample type is created and registered */
        e->typeSupport = ThroughputModule_DataTypeTypeSupport__alloc();
        typename = ThroughputModule_DataTypeTypeSupport_get_type_name(e->typeSupport);
        status = ThroughputModule_DataTypeTypeSupport_register_type(
            e->typeSupport, e->participant, typename);
        CHECK_STATUS_MACRO(status);

        /** A DDS_Topic is created for our sample type on the domain participant. */
        e->topic = DDS_DomainParticipant_create_topic(
            e->participant, "Throughput", typename, DDS_TOPIC_QOS_DEFAULT, NULL, 0);
        CHECK_HANDLE_MACRO(e->topic);
        DDS_free(typename);

        /** A DDS_Subscriber is created on the domain participant. */
        subQos = DDS_SubscriberQos__alloc();
        status = DDS_DomainParticipant_get_default_subscriber_qos(e->participant, subQos);
        CHECK_STATUS_MACRO(status);
        subQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
        subQos->partition.name._length = 1;
        subQos->partition.name._maximum = 1;
        subQos->partition.name._release = TRUE;
        subQos->partition.name._buffer[0] = DDS_string_dup(partitionName);
        e->subscriber = DDS_DomainParticipant_create_subscriber(e->participant, subQos, NULL, 0);
        CHECK_HANDLE_MACRO(e->subscriber);
        DDS_free(subQos);

        /** A DDS_DataReader is created on the Publisher & Topic with a modififed Qos. */
        drQos = DDS_DataReaderQos__alloc();
        status = DDS_Subscriber_get_default_datareader_qos(e->subscriber, drQos);
        CHECK_STATUS_MACRO(status);
        drQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        drQos->reliability.max_blocking_time.sec = 10;
        drQos->reliability.max_blocking_time.nanosec = 0;
        drQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        drQos->resource_limits.max_samples = 400;
        e->reader = DDS_Subscriber_create_datareader(e->subscriber, e->topic, drQos, NULL, 0);
        CHECK_HANDLE_MACRO(e->reader);
        DDS_free(drQos);

        /** A DDS_StatusCondition is created which is triggered when data is available to read */
        e->dataAvailable = ThroughputModule_DataTypeDataReader_get_statuscondition(e->reader);
        CHECK_HANDLE_MACRO(e->dataAvailable);
        status = DDS_StatusCondition_set_enabled_statuses(e->dataAvailable, DDS_DATA_AVAILABLE_STATUS);
        CHECK_STATUS_MACRO(status);

        /** A DDS_WaitSet is created and the data available status condition is attached */
        e->waitSet = DDS_WaitSet__alloc();
        status = DDS_WaitSet_attach_condition(e->waitSet, e->dataAvailable);
        CHECK_STATUS_MACRO(status);

        status = DDS_WaitSet_attach_condition(e->waitSet, terminated);
        CHECK_STATUS_MACRO(status);
    }

    /* Read samples until the maxCycles has been reached (0 = infinite) */
    {
        unsigned long long cycles = 0;

        DDS_unsigned_long i;
        DDS_InstanceHandle_t ph;

        DDS_Duration_t infinite = DDS_DURATION_INFINITE;
        HandleMap *count = HandleMap__alloc();
        HandleMap *startCount = HandleMap__alloc();
        HandleMap *prevCount = HandleMap__alloc();
        HandleEntry *pubCount = NULL;
        HandleEntry *pubStartCount = NULL;
        unsigned long long outOfOrder = 0;
        unsigned long long received = 0;
        unsigned long long prevReceived = 0;
        unsigned long long deltaReceived = 0;

        struct timeval time = { 0, 0} ;
        struct timeval startTime = { 0, 0 };
        struct timeval prevTime = { 0, 0 };
        struct timeval deltaTv;

        DDS_ConditionSeq *conditions = DDS_ConditionSeq__alloc();
        DDS_sequence_ThroughputModule_DataType *samples = DDS_sequence_ThroughputModule_DataType__alloc();
        DDS_SampleInfoSeq *info = DDS_SampleInfoSeq__alloc();
        unsigned long payloadSize = 0;
        double deltaTime = 0;

        printf("Waiting for samples...\n");

        while (!DDS_GuardCondition_get_trigger_value(terminated) && (maxCycles == 0 || cycles < maxCycles)) {
            /** If polling delay is set */
            if (pollingDelay) {
                /** Sleep before polling again */
                exampleSleepMilliseconds(pollingDelay);
            } else {
                /** Wait for samples */
                status = DDS_WaitSet_wait(e->waitSet, conditions, &infinite);
                CHECK_STATUS_MACRO(status);
            }

            /** Take samples and iterate through them */
            status = ThroughputModule_DataTypeDataReader_take(e->reader, samples, info, DDS_LENGTH_UNLIMITED,
                DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
            CHECK_STATUS_MACRO(status);
            for (i = 0; !DDS_GuardCondition_get_trigger_value(terminated) && i < samples->_length; i++) {
                if (info->_buffer[i].valid_data) {
                    ph = info->_buffer[i].publication_handle;

                    /** Check that the sample is the next one expected */
                    pubCount = retrieve_handle(count, ph);
                    pubStartCount = retrieve_handle(startCount, ph);
                    if (!pubCount && !pubStartCount) {
                        pubCount = store_handle(count, ph);
                        pubStartCount = store_handle(startCount, ph);
                        pubCount->count = samples->_buffer[i].count;
                        pubStartCount->count = samples->_buffer[i].count;
                    }
                    if (samples->_buffer[i].count != pubCount->count) {
                        outOfOrder++;
                    }
                    pubCount->count = samples->_buffer[i].count + 1;

                    /** Add the sample payload size to the total received */
                    payloadSize = samples->_buffer[i].payload._length;
                    received += payloadSize + 8;
                }
            }

            /** Check that at lease on second has passed since the last output */
            time = exampleGetTime();
            if (exampleTimevalToMicroseconds(&time) > (exampleTimevalToMicroseconds(&prevTime) + US_IN_ONE_SEC)) {
                /** If not the first iteration */
                if (exampleTimevalToMicroseconds(&prevTime)) {
                    /**
                     * Calculate the samples and bytes received and the time passed since the
                     * last iteration and output
                     */
                    deltaReceived = received - prevReceived;
                    deltaTv = exampleSubtractTimevalFromTimeval(&time, &prevTime);
                    deltaTime = (double)exampleTimevalToMicroseconds(&deltaTv) / US_IN_ONE_SEC;

                    printf("Payload size: %lu | Total received: %llu samples, %llu bytes | Out of order: %llu samples | "
                        "Transfer rate: %.2lf samples/s, %.2lf Mbit/s\n",
                        payloadSize, samplesReceived(count, startCount, FALSE), received, outOfOrder,
                        samplesReceived(count, prevCount, TRUE) / deltaTime,
                        (deltaReceived / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime);
                    fflush (stdout);
                    cycles++;
                } else {
                    /* Copy entries from startCount map to prevCount map */
                    copy_handles(startCount, prevCount);

                    /** Set the start time if it is the first iteration */
                    startTime = time;
                }
                /** Update the previous values for next iteration */
                copy_handles(count, prevCount);
                prevReceived = received;
                prevTime = time;
            }

            status = ThroughputModule_DataTypeDataReader_return_loan(e->reader, samples, info);
            CHECK_STATUS_MACRO(status);
        }

        /** Output totals and averages */
        deltaTv = exampleSubtractTimevalFromTimeval(&time, &startTime);
        deltaTime = (double)exampleTimevalToMicroseconds(&deltaTv) / US_IN_ONE_SEC;
        printf("\nTotal received: %llu samples, %llu bytes\n",
            samplesReceived(count, startCount, FALSE), received);
        printf("Out of order: %llu samples\n",
            outOfOrder);
        printf("Average transfer rate: %.2lf samples/s, %.2lf Mbit/s\n",
            samplesReceived(count, startCount, FALSE) / deltaTime,
            ((double)received / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime);

        DDS_free(conditions);
        DDS_free(samples);
        DDS_free(info);
        HandleMap__free(count);
        HandleMap__free(startCount);
        HandleMap__free(prevCount);
    }

    /** Cleanup entities */
    if (e->participant) {
        status = DDS_DomainParticipant_delete_contained_entities(e->participant);
        CHECK_STATUS_MACRO(status);
        status = DDS_DomainParticipantFactory_delete_participant(e->domainParticipantFactory, e->participant);
        CHECK_STATUS_MACRO(status);
        DDS_free(e->waitSet);
        DDS_free(e->typeSupport);
    }
    DDS_free(terminated);
    free(e);

#ifdef _WIN32
    SetConsoleCtrlHandler(0, FALSE);
#else
    sigaction(SIGINT, &oldAction, 0);
#endif
    return result;
}
