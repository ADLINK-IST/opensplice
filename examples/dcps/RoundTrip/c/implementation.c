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

#include "implementation.h"
#include "common/example_utilities.h"
#include "common/example_error_sac.h"

#include <stdio.h>
#include <stdlib.h>

#include "RoundTrip.h"

#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif

/**
 * @addtogroup examplesdcpsRoundTripsac The Standalone C DCPS API RoundTrip example
 *
 * The RoundTrip example consists of a Ping and a Pong application. Ping sends sample
 * to Pong by writing to the Ping partition which Pong subscribes to. Pong them sends them
 * back to Ping by writing on the Pong partition which Ping subscribes to. Ping measure
 * the amount of time taken to write and read each sample as well as the total round trip
 * time to send a sample to Pong and receive it back.
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
static DDS_boolean CtrlHandler(DWORD fdwCtrlType)
{
    status = DDS_GuardCondition_set_trigger_value(terminated, TRUE);
    CHECK_STATUS_MACRO(status);
    return TRUE; //Don't let other handlers handle this key
}
#else
static void CtrlHandler(int fdwCtrlType)
{
    status = DDS_GuardCondition_set_trigger_value(terminated, TRUE);
    CHECK_STATUS_MACRO(status);
}
#endif

/**
 * This struct serves as a container holding initialised entities used by ping and pong.
 */
typedef struct
{
    /** The DomainParticipantFactory used by ping and pong */
    DDS_DomainParticipantFactory factory;
    /** The DomainParticipant used by ping and pong */
    DDS_DomainParticipant participant;
    /** The TypeSupport for the sample */
    RoundTripModule_SampleTypeSupport typeSupport;
    /** The Topic used by ping and pong */
    DDS_Topic topic;
    /** The Publisher used by ping and pong */
    DDS_Publisher publisher;
    /** The DataWriter used by ping and pong */
    RoundTripModule_SampleDataWriter writer;
    /** The Subscriber used by ping and pong */
    DDS_Subscriber subscriber;
    /** The DataReader used by ping and pong */
    RoundTripModule_SampleDataReader reader;
    /** The WaitSet used by ping and pong */
    DDS_WaitSet waitSet;
    /** The StatusCondition used by ping and pong,
     * triggered when data is available to read */
    DDS_StatusCondition dataAvailable;

    /** The sample used to send data */
    RoundTripModule_Sample *data;
    /** The condition sequence used to store conditions returned by the WaitSet */
    DDS_ConditionSeq *conditions;
    /** The sequence used to hold samples received by the DataReader */
    DDS_sequence_RoundTripModule_Sample *samples;
    /** The sequence used to hold information about the samples received by the DataReader */
    DDS_SampleInfoSeq *info;
    /** A sequence used to store instance handles */
    DDS_InstanceHandleSeq *ihs;

    ExampleTimeStats roundTrip;
    ExampleTimeStats writeAccess;
    ExampleTimeStats readAccess;
    ExampleTimeStats roundTripOverall;
    ExampleTimeStats writeAccessOverall;
    ExampleTimeStats readAccessOverall;
} Entities;

/**
* This function initialises the entities used by ping and pong, or exit
* on any failure.
*/
void initialise(Entities *e, const char *pubPartition, const char *subPartition)
{
    DDS_PublisherQos *pubQos;
    DDS_DataWriterQos *dwQos;
    DDS_DataReaderQos *drQos;
    DDS_SubscriberQos *subQos;

    /* Register handler for Ctrl-C */
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
#else
    struct sigaction sat;
    sat.sa_handler = CtrlHandler;
    sigemptyset(&sat.sa_mask);
    sat.sa_flags = 0;
    sigaction(SIGINT,&sat,&oldAction);
#endif
    terminated = DDS_GuardCondition__alloc();

    /** A DDS_DomainParticipant is created for the default domain. */
    e->factory = DDS_DomainParticipantFactory_get_instance();
    CHECK_HANDLE_MACRO(e->factory);
    e->participant = DDS_DomainParticipantFactory_create_participant(
        e->factory, DDS_DOMAIN_ID_DEFAULT, DDS_PARTICIPANT_QOS_DEFAULT, 0, DDS_STATUS_MASK_NONE);
    CHECK_HANDLE_MACRO(e->participant);

    /** The sample type is created and registered */
    e->typeSupport = RoundTripModule_SampleTypeSupport__alloc();
    CHECK_HANDLE_MACRO(e->typeSupport);
    status = RoundTripModule_SampleTypeSupport_register_type(
        e->typeSupport, e->participant, RoundTripModule_SampleTypeSupport_get_type_name(e->typeSupport));
    CHECK_STATUS_MACRO(status);

    /** A DDS_Topic is created for our sample type on the domain participant. */
    e->topic = DDS_DomainParticipant_create_topic(
        e->participant, "RoundTrip", RoundTripModule_SampleTypeSupport_get_type_name(e->typeSupport),
                                                  DDS_TOPIC_QOS_DEFAULT, 0, DDS_STATUS_MASK_NONE);
    CHECK_HANDLE_MACRO(e->topic);

    /** A DDS_Publisher is created on the domain participant. */
    pubQos = DDS_PublisherQos__alloc();
    CHECK_HANDLE_MACRO(pubQos);
    status = DDS_DomainParticipant_get_default_publisher_qos(e->participant, pubQos);
    CHECK_STATUS_MACRO(status);
    pubQos->partition.name._length = 1;
    pubQos->partition.name._maximum = 1;
    pubQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    pubQos->partition.name._buffer[0] = DDS_string_alloc((DDS_unsigned_long)strlen(pubPartition) + 1);
    strcpy(pubQos->partition.name._buffer[0], pubPartition);
    e->publisher = DDS_DomainParticipant_create_publisher(e->participant, pubQos, 0, DDS_STATUS_MASK_NONE);
    CHECK_HANDLE_MACRO(e->publisher);
    DDS_free(pubQos);

    /** A DDS_DataWriter is created on the Publisher & Topic with a modififed Qos. */
    dwQos = DDS_DataWriterQos__alloc();
    CHECK_HANDLE_MACRO(dwQos);
    status = DDS_Publisher_get_default_datawriter_qos(e->publisher, dwQos);
    CHECK_STATUS_MACRO(status);
    dwQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    dwQos->reliability.max_blocking_time.sec = 10;
    dwQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
    dwQos->writer_data_lifecycle.autodispose_unregistered_instances = FALSE;
    e->writer = DDS_Publisher_create_datawriter(e->publisher, e->topic, dwQos, 0, DDS_STATUS_MASK_NONE);
    CHECK_HANDLE_MACRO(e->writer);
    DDS_free(dwQos);

    /** A DDS_Subscriber is created on the domain participant. */
    subQos = DDS_SubscriberQos__alloc();
    CHECK_HANDLE_MACRO(subQos);
    status = DDS_DomainParticipant_get_default_subscriber_qos(e->participant, subQos);
    CHECK_STATUS_MACRO(status);
    subQos->partition.name._length = 1;
    subQos->partition.name._maximum = 1;
    subQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    subQos->partition.name._buffer[0] = DDS_string_alloc((DDS_unsigned_long)strlen(subPartition) + 1);
    strcpy(subQos->partition.name._buffer[0], subPartition);
    e->subscriber = DDS_DomainParticipant_create_subscriber(e->participant, subQos, 0, DDS_STATUS_MASK_NONE);
    CHECK_HANDLE_MACRO(e->subscriber);

    /** A DDS_DataReader is created on the Subscriber & Topic with a modified QoS. */
    drQos = DDS_DataReaderQos__alloc();
    CHECK_HANDLE_MACRO(drQos);
    status = DDS_Subscriber_get_default_datareader_qos(e->subscriber, drQos);
    CHECK_STATUS_MACRO(status);
    drQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    drQos->reliability.max_blocking_time.sec = 10;
    drQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
    e->reader = DDS_Subscriber_create_datareader(
        e->subscriber, e->topic, drQos, 0, DDS_STATUS_MASK_NONE);
    CHECK_HANDLE_MACRO(e->reader);

    /** A DDS_StatusCondition is created which is triggered when data is available to read */
    e->dataAvailable = DDS_DataReader_get_statuscondition(e->reader);
    CHECK_HANDLE_MACRO(e->dataAvailable);
    status = DDS_StatusCondition_set_enabled_statuses(e->dataAvailable, DDS_DATA_AVAILABLE_STATUS);
    CHECK_STATUS_MACRO(status);

    /** A DDS_WaitSet is created and the data available status condition is attached */
    e->waitSet = DDS_WaitSet__alloc();
    CHECK_HANDLE_MACRO(e->waitSet);
    status = DDS_WaitSet_attach_condition(e->waitSet, e->dataAvailable);
    CHECK_STATUS_MACRO(status);

    status = DDS_WaitSet_attach_condition(e->waitSet, terminated);
    CHECK_STATUS_MACRO(status);

    /** Initialise data structures */
    e->data = RoundTripModule_Sample__alloc();
    CHECK_HANDLE_MACRO(e->data);

    e->conditions = DDS_ConditionSeq__alloc();
    CHECK_HANDLE_MACRO(e->conditions);

    e->samples = DDS_sequence_RoundTripModule_Sample__alloc();
    CHECK_HANDLE_MACRO(e->samples);

    e->info = DDS_SampleInfoSeq__alloc();
    CHECK_HANDLE_MACRO(e->info);

    e->ihs = DDS_InstanceHandleSeq__alloc();
    CHECK_HANDLE_MACRO(e->ihs);

    /** Initialise ExampleTimeStats used to track timing */
    e->roundTrip = exampleInitTimeStats();
    e->writeAccess = exampleInitTimeStats();
    e->readAccess = exampleInitTimeStats();
    e->roundTripOverall = exampleInitTimeStats();
    e->writeAccessOverall = exampleInitTimeStats();
    e->readAccessOverall = exampleInitTimeStats();
}

/**
* This function cleans up after the application has finished running
*/
void cleanup(Entities *e)
{
    DDS_free(e->data->payload._buffer);
    DDS_free(e->data);
    DDS_free(e->conditions);
    status = RoundTripModule_SampleDataReader_return_loan(e->reader, e->samples, e->info);
    CHECK_STATUS_MACRO(status);
    DDS_free(e->samples);
    DDS_free(e->info);
    DDS_free(e->ihs);
    status = DDS_WaitSet_detach_condition(e->waitSet, e->dataAvailable);
    CHECK_STATUS_MACRO(status);
    DDS_free(e->waitSet);
    status = DDS_DomainParticipant_delete_contained_entities(e->participant);
    CHECK_STATUS_MACRO(status);
    status = DDS_DomainParticipantFactory_delete_participant(e->factory, e->participant);
    CHECK_STATUS_MACRO(status);

    exampleDeleteTimeStats(&e->roundTrip);
    exampleDeleteTimeStats(&e->writeAccess);
    exampleDeleteTimeStats(&e->readAccess);
    exampleDeleteTimeStats(&e->roundTripOverall);
    exampleDeleteTimeStats(&e->writeAccessOverall);
    exampleDeleteTimeStats(&e->readAccessOverall);

#ifdef _WIN32
    SetConsoleCtrlHandler(0, FALSE);
#else
    sigaction(SIGINT,&oldAction, 0);
#endif

}

/**
 * This function performs the Ping role in this example.
 * @return 0 if a sample is successfully written, 1 otherwise.
 */
int ping(int argc, char *argv[])
{
    unsigned long payloadSize = 0;
    unsigned long long numSamples = 0;
    unsigned long timeOut = 0;
    DDS_boolean pongRunning;
    struct timeval startTime;
    struct timeval time;
    struct timeval preWriteTime;
    struct timeval postWriteTime;
    struct timeval preTakeTime;
    struct timeval postTakeTime;
    struct timeval difference = {0, 0};
    DDS_Duration_t waitTimeout = {1, 0};
    unsigned long long i;
    unsigned long elapsed = 0;
    DDS_boolean invalid = FALSE;
    DDS_boolean warmUp = TRUE;

    /** Initialise entities */
    Entities e;
    initialise(&e, "ping", "pong");

    setbuf(stdout, NULL);

    /** Interpret arguments */
    if(argc == 2 && strcmp(argv[1], "quit") == 0)
    {
        /** Wait for pong to run */
        printf("Waiting for pong to run...\n");
        pongRunning = FALSE;
        while(!DDS_GuardCondition_get_trigger_value(terminated) && !pongRunning)
        {
            RoundTripModule_SampleDataWriter_get_matched_subscriptions(e.writer, e.ihs);
            if(e.ihs->_length != 0)
            {
                pongRunning = TRUE;
            }
        };
        printf("Sending termination request.\n");
        /** Send quit signal to pong if "quit" is supplied as an argument to ping */
        RoundTripModule_SampleDataWriter_dispose(e.writer, e.data, DDS_HANDLE_NIL);
        exampleSleepMilliseconds(1000);

        cleanup(&e);
        return 0;
    }
    if(argc >= 2)
    {
        payloadSize = atoi(argv[1]);

        if(payloadSize > 655536)
        {
            invalid = TRUE;
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
        printf("Usage (parameters must be supplied in order):\n"
                "./ping [payloadSize (bytes, 0 - 655536)] [numSamples (0 = infinite)] [timeOut (seconds, 0 = infinite)]\n"
                "./ping quit - ping sends a quit signal to pong.\n"
                "Defaults:\n"
                "./ping 0 0 0\n");
        cleanup(&e);
        exit(1);
    }

    printf("# payloadSize: %lu | numSamples: %llu | timeOut: %lu\n\n", payloadSize, numSamples, timeOut);

    e.data->payload._length = payloadSize;
    e.data->payload._maximum = payloadSize;
    e.data->payload._buffer = DDS_sequence_char_allocbuf(payloadSize);
    for(i = 0; i < payloadSize; i++)
    {
        e.data->payload._buffer[i] = 'a';
    }

    startTime = exampleGetTime();
    printf("# Warming up...\n");
    while(!DDS_GuardCondition_get_trigger_value(terminated) && exampleTimevalToMicroseconds(&difference) / US_IN_ONE_SEC < 5)
    {
        status = RoundTripModule_SampleDataWriter_write(e.writer, e.data, DDS_HANDLE_NIL);
        CHECK_STATUS_MACRO(status);
        status = DDS_WaitSet_wait(e.waitSet, e.conditions, &waitTimeout);
        if(status != DDS_RETCODE_TIMEOUT)
        {
            CHECK_STATUS_MACRO(status);
            status = RoundTripModule_SampleDataReader_take(e.reader, e.samples, e.info, DDS_LENGTH_UNLIMITED,
                                        DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
            CHECK_STATUS_MACRO(status);
            status = RoundTripModule_SampleDataReader_return_loan(e.reader, e.samples, e.info);
            CHECK_STATUS_MACRO(status);
        }

        time = exampleGetTime();
        difference = exampleSubtractTimevalFromTimeval(&time, &startTime);
    }
    if(!DDS_GuardCondition_get_trigger_value(terminated))
    {
        warmUp = FALSE;
        printf("# Warm up complete.\n\n");

        printf("# Round trip measurements (in us)\n");
        printf("#             Round trip time [us]         Write-access time [us]       Read-access time [us]\n");
        printf("# Seconds     Count   median      min      Count   median      min      Count   median      min\n");
    }

    startTime = exampleGetTime();
    for(i = 0; !DDS_GuardCondition_get_trigger_value(terminated) && (!numSamples || i < numSamples); i++)
    {
        /** Write a sample that pong can send back */
        preWriteTime = exampleGetTime();
        status = RoundTripModule_SampleDataWriter_write(e.writer, e.data, DDS_HANDLE_NIL);
        postWriteTime = exampleGetTime();
        CHECK_STATUS_MACRO(status);

        /** Wait for response from pong */
        status = DDS_WaitSet_wait(e.waitSet, e.conditions, &waitTimeout);
        if(status != DDS_RETCODE_TIMEOUT)
        {
            CHECK_STATUS_MACRO(status);

            /** Take sample and check that it is valid */
            preTakeTime = exampleGetTime();
            status = RoundTripModule_SampleDataReader_take(e.reader, e.samples, e.info, DDS_LENGTH_UNLIMITED,
                                        DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
            postTakeTime = exampleGetTime();
            CHECK_STATUS_MACRO(status);

            if(!DDS_GuardCondition_get_trigger_value(terminated))
            {
                if(e.samples->_length != 1)
                {
                    fprintf(stdout, "%s%d%s", "ERROR: Ping received ", e.samples->_length,
                                " samples but was expecting 1. Are multiple pong applications running?\n");

                    cleanup(&e);
                    exit(0);
                }
                else if(!e.info->_buffer[0].valid_data)
                {
                    printf("ERROR: Ping received an invalid sample. Has pong terminated already?\n");

                    cleanup(&e);
                    exit(0);
                }
            }
            status = RoundTripModule_SampleDataReader_return_loan(e.reader, e.samples, e.info);
            CHECK_STATUS_MACRO(status);

            /** Update stats */
            difference = exampleSubtractTimevalFromTimeval(&postWriteTime, &preWriteTime);
            e.writeAccess = *exampleAddMicrosecondsToTimeStats(
                &e.writeAccess, exampleTimevalToMicroseconds(&difference));

            difference = exampleSubtractTimevalFromTimeval(&postTakeTime, &preTakeTime);
            e.readAccess = *exampleAddMicrosecondsToTimeStats(
                &e.readAccess, exampleTimevalToMicroseconds(&difference));

            difference = exampleSubtractTimevalFromTimeval(&postTakeTime, &preWriteTime);
            e.roundTrip = *exampleAddMicrosecondsToTimeStats(
                &e.roundTrip, exampleTimevalToMicroseconds(&difference));

            difference = exampleSubtractTimevalFromTimeval(&postWriteTime, &preWriteTime);
            e.writeAccessOverall = *exampleAddMicrosecondsToTimeStats(
                &e.writeAccessOverall, exampleTimevalToMicroseconds(&difference));

            difference = exampleSubtractTimevalFromTimeval(&postTakeTime, &preTakeTime);
            e.readAccessOverall = *exampleAddMicrosecondsToTimeStats(
                &e.readAccessOverall, exampleTimevalToMicroseconds(&difference));

            difference = exampleSubtractTimevalFromTimeval(&postTakeTime, &preWriteTime);
            e.roundTripOverall = *exampleAddMicrosecondsToTimeStats(
                &e.roundTripOverall, exampleTimevalToMicroseconds(&difference));

            /** Print stats each second */
            difference = exampleSubtractTimevalFromTimeval(&postTakeTime, &startTime);
            if(exampleTimevalToMicroseconds(&difference) > US_IN_ONE_SEC || (i && i == numSamples))
            {
                /* Print stats */
                printf ("%9lu %9lu %8.0f %8lu %10lu %8.0f %8lu %10lu %8.0f %8lu\n",
                    elapsed + 1,
                    e.roundTrip.count,
                    exampleGetMedianFromTimeStats(&e.roundTrip),
                    e.roundTrip.min,
                    e.writeAccess.count,
                    exampleGetMedianFromTimeStats(&e.writeAccess),
                    e.writeAccess.min,
                    e.readAccess.count,
                    exampleGetMedianFromTimeStats(&e.readAccess),
                    e.readAccess.min);

                /* Reset stats for next run */
                exampleResetTimeStats(&e.roundTrip);
                exampleResetTimeStats(&e.writeAccess);
                exampleResetTimeStats(&e.readAccess);

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
            status = DDS_GuardCondition_set_trigger_value(terminated, TRUE);
            CHECK_STATUS_MACRO(status);
        }
    }

    if(!warmUp)
    {
        /** Print overall stats */
        printf ("\n%9s %9lu %8.0f %8lu %10lu %8.0f %8lu %10lu %8.0f %8lu\n",
                    "# Overall",
                    e.roundTripOverall.count,
                    exampleGetMedianFromTimeStats(&e.roundTripOverall),
                    e.roundTripOverall.min,
                    e.writeAccessOverall.count,
                    exampleGetMedianFromTimeStats(&e.writeAccessOverall),
                    e.writeAccessOverall.min,
                    e.readAccessOverall.count,
                    exampleGetMedianFromTimeStats(&e.readAccessOverall),
                    e.readAccessOverall.min);
    }

    cleanup(&e);
    return 0;
}

/**
 * Runs the Pong role in this example.
 * @return 0 if a sample is successfully read, 1 otherwise.
 */
int pong(int argc, char *argv[])
{
    DDS_Duration_t waitTimeout = DDS_DURATION_INFINITE;
    unsigned int i;

    /** Initialise entities */
    Entities e;
    initialise(&e, "pong", "ping");

    printf("Waiting for samples from ping to send back...\n");
    fflush(stdout);

    while(!DDS_GuardCondition_get_trigger_value(terminated))
    {
        /** Wait for a sample from ping */
        status = DDS_WaitSet_wait(e.waitSet, e.conditions, &waitTimeout);
        if (status != DDS_RETCODE_TIMEOUT)
        {
            CHECK_STATUS_MACRO(status);

            /** Take samples */
            status = RoundTripModule_SampleDataReader_take(
                e.reader, e.samples, e.info, DDS_LENGTH_UNLIMITED, DDS_ANY_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
            CHECK_STATUS_MACRO(status);
            for (i = 0; !DDS_GuardCondition_get_trigger_value(terminated) && i < e.samples->_length; i++)
            {
                /** If writer has been disposed terminate pong */
                if(e.info->_buffer[i].instance_state == DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE)
                {
                    printf("Received termination request. Terminating.\n");
                    status = DDS_GuardCondition_set_trigger_value(terminated, TRUE);
                    CHECK_STATUS_MACRO(status);
                    break;
                }
                /** If sample is valid, send it back to ping */
                else if(e.info->_buffer[i].valid_data)
                {
                    status = RoundTripModule_SampleDataWriter_write(e.writer, &e.samples->_buffer[i],
                                                                    DDS_HANDLE_NIL);
                    CHECK_STATUS_MACRO(status);
                }
            }
            status = RoundTripModule_SampleDataReader_return_loan(e.reader, e.samples, e.info);
            CHECK_STATUS_MACRO(status);
        }
    }

    cleanup(&e);
    return 0;
}

C_EXAMPLE_ENTRYPOINT(DCPS_SAC_RoundTrip_ping, ping)
C_EXAMPLE_ENTRYPOINT(DCPS_SAC_RoundTrip_pong, pong)
