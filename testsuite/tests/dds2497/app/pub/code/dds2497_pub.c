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

#include "os_stdlib.h"

#include "cmx_participant.h"
#include "cmx_factory.h"
#include "cmx_entity.h"

#include "dds_dcps.h"
#include "net2497.h"

char *RetCodeName[13] = {
    "DDS_RETCODE_OK",
    "DDS_RETCODE_ERROR",
    "DDS_RETCODE_UNSUPPORTED",
    "DDS_RETCODE_BAD_PARAMETER",
    "DDS_RETCODE_PRECONDITION_NOT_MET",
    "DDS_RETCODE_OUT_OF_RESOURCES",
    "DDS_RETCODE_NOT_ENABLED",
    "DDS_RETCODE_IMMUTABLE_POLICY",
    "DDS_RETCODE_INCONSISTENT_POLICY",
    "DDS_RETCODE_ALREADY_DELETED",
    "DDS_RETCODE_TIMEOUT",
    "DDS_RETCODE_NO_DATA",
    "DDS_RETCODE_ILLEGAL_OPERATION" };


void createMessage(net2497_topic2497 *sample, DDS_unsigned_long s) {

    DDS_unsigned_long i;

    sample->index = s;
    sample->seq_payload._length = s;
    sample->seq_payload._maximum = s;
    sample->seq_payload._release = TRUE;
    sample->seq_payload._buffer = DDS_sequence_net2497_payload_allocbuf(s);

    sample->seq_payloadfloat._length = s;
    sample->seq_payloadfloat._maximum = s;
    sample->seq_payloadfloat._release = TRUE;
    sample->seq_payloadfloat._buffer = DDS_sequence_net2497_payloadfloat_allocbuf(s);

    sample->seq_payloaddouble._length = s;
   sample->seq_payloaddouble._maximum = s;
   sample->seq_payloaddouble._release = TRUE;
   sample->seq_payloaddouble._buffer = DDS_sequence_net2497_payloaddouble_allocbuf(s);

    for (i=0;i<s;i++) {
        sample->seq_payload._buffer[i].long1 = 1;
        sample->seq_payload._buffer[i].long2 = 2;
        sample->seq_payload._buffer[i].long3 = 3;
        sample->seq_payload._buffer[i].long4 = 4;
        sample->seq_payload._buffer[i].long5 = 5;
        sample->seq_payload._buffer[i].long6 = 6;
        sample->seq_payload._buffer[i].long7 = 7;
        sample->seq_payload._buffer[i].long8 = 8;
        sample->seq_payload._buffer[i].long9 = 9;
        sample->seq_payload._buffer[i].long10 = 10;

        sample->seq_payloadfloat._buffer[i].float1 = 1.1;
        sample->seq_payloadfloat._buffer[i].float2 = 2.2;
        sample->seq_payloadfloat._buffer[i].float3 = 3.3;
        sample->seq_payloadfloat._buffer[i].float4 = 4.4;
        sample->seq_payloadfloat._buffer[i].float5 = 5.5;
        sample->seq_payloadfloat._buffer[i].float6 = 6.6;
        sample->seq_payloadfloat._buffer[i].float7 = 7.7;
        sample->seq_payloadfloat._buffer[i].float8 = 8.8;
        sample->seq_payloadfloat._buffer[i].float9 = 9.9;
        sample->seq_payloadfloat._buffer[i].float10 = 10.0;

        sample->seq_payloaddouble._buffer[i].double1 = 1234.1234567890;
       sample->seq_payloaddouble._buffer[i].double2 = 2234.2234567890;
       sample->seq_payloaddouble._buffer[i].double3 = 3234.3234567890;
       sample->seq_payloaddouble._buffer[i].double4 = 4234.4234567890;
       sample->seq_payloaddouble._buffer[i].double5 = 5234.5234567890;
       sample->seq_payloaddouble._buffer[i].double6 = 6234.6234567890;
       sample->seq_payloaddouble._buffer[i].double7 = 7234.7234567890;
       sample->seq_payloaddouble._buffer[i].double8 = 8234.8234567890;
       sample->seq_payloaddouble._buffer[i].double9 = 9234.9234567890;
       sample->seq_payloaddouble._buffer[i].double10 = 10234.0234567890;
    }

}


char *getErrorName(DDS_ReturnCode_t status)
{
    return RetCodeName[status];
}


void checkStatus(
    DDS_ReturnCode_t status,
    const char *info ) {

    if (status != DDS_RETCODE_OK && status != DDS_RETCODE_NO_DATA) {
        fprintf(stderr, "Error in %s: %s\n", info, getErrorName(status));
        exit (EXIT_FAILURE);
    }
}

void checkHandle(
    void *handle,
    char *info ) {

     if (!handle) {
        fprintf(stderr, "Error in %s: Creation failed: invalid handle\n", info);
        exit (EXIT_FAILURE);
     }
}

int main (int argc,
    char **argv) 
{
    /* Generic DDS entities */
    DDS_DomainParticipantFactory    dpf;
    DDS_DomainParticipant           participant;
    DDS_Topic                       thisTopicA;
    DDS_Publisher                   pub;

    /* QosPolicy holders */
    DDS_TopicQos                    *thisTopicQos;

    /* DDS Identifiers */
    DDS_DomainId_t                  domain = NULL;
    DDS_ReturnCode_t                status;

    /* Type-specific DDS entities */
    net2497_topic2497TypeSupport    TS;
    net2497_topic2497DataWriter     writerA;

    /* Sample definitions */
    net2497_topic2497               *sampleA;
    
    /* Others */
    char                            *thisTypeName = NULL;
    os_time                         delay;
    DDS_unsigned_long i;
    DDS_unsigned_long j;
    DDS_unsigned_long size;
    DDS_unsigned_long burst;
    DDS_DataWriterQos  writerQos;
    memset(&writerQos,       0, sizeof(writerQos));


    /* Create a DomainParticipantFactory and a DomainParticipant (using Default 
QoS settings). */
    dpf = DDS_DomainParticipantFactory_get_instance ();
    checkHandle(dpf, "DDS_DomainParticipantFactory_get_instance");
    participant = DDS_DomainParticipantFactory_create_participant (
        dpf, 
        domain, 
        DDS_PARTICIPANT_QOS_DEFAULT, 
        NULL,
        DDS_ANY_STATUS);
    checkHandle(participant, "DDS_DomainParticipantFactory_create_participant");
  
    /* Register the required datatype */
    TS = net2497_topic2497TypeSupport__alloc();
    checkHandle(TS, "net2497_topic2497TypeSupport__alloc");
    thisTypeName = net2497_topic2497TypeSupport_get_type_name(TS);
    status = net2497_topic2497TypeSupport_register_type(
        TS, 
        participant, 
        thisTypeName);
    checkStatus(status, "net2497_topic2497TypeSupport_register_type");
    
    /* Set the QosPolicy  */
    thisTopicQos = DDS_TopicQos__alloc();
    checkHandle(thisTopicQos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(participant, thisTopicQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");
    thisTopicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    thisTopicQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;

    /* Make the tailored QoS the new default. */
    status = DDS_DomainParticipant_set_default_topic_qos(participant, thisTopicQos);
    checkStatus(status, "DDS_DomainParticipant_set_default_topic_qos");

    /* Use the changed policy when defining the topic */
    thisTopicA = DDS_DomainParticipant_create_topic(
        participant, 
        "net2497_topic2497A",
        thisTypeName, 
        thisTopicQos, 
        NULL,
        DDS_ANY_STATUS);
    checkHandle(thisTopicA, "DDS_DomainParticipant_create_topic (paragraph)");
	

    /* Create a Publisher */
    pub = DDS_DomainParticipant_create_publisher(participant, DDS_PUBLISHER_QOS_DEFAULT, NULL, DDS_ANY_STATUS);
    checkHandle(pub, "DDS_DomainParticipant_create_publisher");
    
    DDS_Publisher_get_default_datawriter_qos(pub, &writerQos);
    writerQos.latency_budget.duration.sec     = 1;
    writerQos.latency_budget.duration.nanosec = 0;
    DDS_Publisher_set_default_datawriter_qos(pub, &writerQos);

    /* Create a DataWriter */
    writerA = DDS_Publisher_create_datawriter( 
        pub, 
        thisTopicA, 
        DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
        NULL,
        DDS_ANY_STATUS);
    checkHandle(writerA, "DDS_Publisher_create_datawriter (paragraph)");



	

    /* Initialize the messages */
    sampleA = net2497_topic2497__alloc();
    checkHandle(sampleA, "net2497_topic2497__alloc");
    sampleA->index = 0;
    size = 1;
    burst =1;

    delay.tv_sec = 0;
    delay.tv_nsec = 500000000;
    while (size <= 750) // limit of the 100MB shared memory
    {

        for (j=0;j<burst;j++) {
            createMessage(sampleA,size);
            status = net2497_topic2497DataWriter_write (writerA, sampleA, DDS_HANDLE_NIL);
            checkStatus(status, "net2497_topic2497DataWriter_write");
            size++;
        }

        burst++;
        fflush(stdout);
        printf ("Written sample to A # %d\n", sampleA->index);
		         fflush(stdout);
        os_nanoSleep(delay);
    }

    return EXIT_SUCCESS;
}

