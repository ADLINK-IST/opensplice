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

int compare_float(float f1, float f2)
{
    float precision = 0.000001;
    if (((f1 - precision) < f2) && ((f1 + precision) > f2)) {
        return 1;
    } else {
        return 0;
    }
}

int compare_double(double f1, double f2)
{
    double precision = 0.00000000001;
    if (((f1 - precision) < f2) && ((f1 + precision) > f2)) {
      return 1;
    } else {
      return 0;
    }
}



int main ()
{
    /* Generic DDS entities */
    DDS_DomainParticipantFactory    dpf;
    DDS_DomainParticipant           participant;
    DDS_Topic                       thisTopicA;
    DDS_Subscriber                  sub;

    /* Type-specific DDS entities */
    net2497_topic2497TypeSupport    TS;
    net2497_topic2497DataReader     readerA;
    DDS_sequence_net2497_topic2497  *msgSeqA;
    DDS_SampleInfoSeq               *infoSeqA;

    /* QosPolicy holders */
    DDS_TopicQos                    *thisTopicQos;

    /* DDS Identifiers */
    DDS_DomainId_t                  domain = NULL;
    DDS_ReturnCode_t                status;

    /* Others */
    DDS_unsigned_long               i;
    DDS_unsigned_long               j;
	DDS_unsigned_long				msgindex =0;
    char *                          thisTypeName = NULL;
    os_time                         delay;

    /* Create a DomainParticipantFactory and a DomainParticipant (using Default QoS settings. */
    dpf = DDS_DomainParticipantFactory_get_instance ();
    checkHandle(dpf, "DDS_DomainParticipantFactory_get_instance");
    participant = DDS_DomainParticipantFactory_create_participant (
        dpf, 
        domain, 
        DDS_PARTICIPANT_QOS_DEFAULT, 
        NULL, 
        DDS_ANY_STATUS);
    checkHandle(participant, "DDS_DomainParticipantFactory_create_participant");  

    /* Register the required datatype for messages. */
    TS = net2497_topic2497TypeSupport__alloc();
    checkHandle(TS, "net2497_topic2497TypeSupport__alloc");
    thisTypeName = net2497_topic2497TypeSupport_get_type_name(TS);
    status = net2497_topic2497TypeSupport_register_type(
        TS, 
        participant, 
        thisTypeName);
    checkStatus(status, "net2497_topic2497TypeSupport_register_type");

    /* Set the CompressionQosPolicy  */
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
	

    /* Create a Subscriber */
    sub = DDS_DomainParticipant_create_subscriber(participant, DDS_SUBSCRIBER_QOS_DEFAULT, NULL, DDS_ANY_STATUS);
    checkHandle(sub, "DDS_DomainParticipant_create_subscriber");

    /* Create a DataReader */
    readerA = DDS_Subscriber_create_datareader( 
        sub, 
        thisTopicA, 
        DDS_DATAREADER_QOS_USE_TOPIC_QOS, 
        NULL,
        DDS_ANY_STATUS);
    checkHandle(readerA, "DDS_Subscriber_create_datareader");
	

    /* Allocate the sequence holders for the DataReader */
    msgSeqA = DDS_sequence_net2497_topic2497__alloc();
    checkHandle(msgSeqA, "DDS_sequence_net2497_topic2497__alloc");
    infoSeqA = DDS_SampleInfoSeq__alloc();
    checkHandle(infoSeqA, "DDS_SampleInfoSeq__alloc");
	
    delay.tv_sec = 0;
    delay.tv_nsec = 500000000;
    fprintf (stdout, "dds2497 Subscriber waiting for samples...\n");
    fflush(stdout);
    while (msgindex <= 750) {
        status = net2497_topic2497DataReader_take(
            readerA, 
            msgSeqA, 
            infoSeqA, 
            DDS_LENGTH_UNLIMITED, 
            DDS_NOT_READ_SAMPLE_STATE,
            DDS_ANY_VIEW_STATE, 
            DDS_ALIVE_INSTANCE_STATE );
        checkStatus(status, "net2497_topic2497DataReader_take");
        for (i = 0; i < msgSeqA->_length; i++) {
            net2497_topic2497 *msg = &(msgSeqA->_buffer[i]);
            DDS_SampleInfo *info = &(infoSeqA->_buffer[i]);	
            if (info->valid_data) {	
              fprintf (stdout, "taken sample from A # %d\n", msg->index);
              fflush(stdout);
              msgindex = msg->index;
              for (j =0; j<msg->index;j++) {
                    if (msg->seq_payload._buffer[j].long1 != 1) { fprintf (stdout, "error in sequence long1 \n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long2 != 2) { fprintf (stdout, "error in sequence long2 \n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long3 != 3) { fprintf (stdout, "error in sequence long3\n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long4 != 4) { fprintf (stdout, "error in sequence long4\n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long5 != 5) { fprintf (stdout, "error in sequence long4\n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long6 != 6) { fprintf (stdout, "error in sequence long6\n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long7 != 7) { fprintf (stdout, "error in sequence long7\n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long8 != 8) { fprintf (stdout, "error in sequence long7\n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long9 != 9) { fprintf (stdout, "error in sequence long9\n"); msgindex = 99999; }
                     if (msg->seq_payload._buffer[j].long10 != 10) { fprintf (stdout, "error in sequence long10\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float1, 1.1)) { fprintf (stdout, "error in sequence float1\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float2 , 2.2)) { fprintf (stdout, "error in sequence float2\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float3 , 3.3)) { fprintf (stdout, "error in sequence float3\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float4 , 4.4)) { fprintf (stdout, "error in sequence float4\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float5 , 5.5)) { fprintf (stdout, "error in sequence float5\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float6 , 6.6)) { fprintf (stdout, "error in sequence float6\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float7 , 7.7)) { fprintf (stdout, "error in sequence float7\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float8 , 8.8)) { fprintf (stdout, "error in sequence float8\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float9 , 9.9)) { fprintf (stdout, "error in sequence float9\n"); msgindex = 99999; }
                     if (!compare_float(msg->seq_payloadfloat._buffer[j].float10 , 10.0)) { fprintf (stdout, "error in sequence float10\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double1 , 1234.1234567890)) { fprintf (stdout, "error in sequence double1\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double2 , 2234.2234567890)) { fprintf (stdout, "error in sequence double2\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double3 , 3234.3234567890)) { fprintf (stdout, "error in sequence double3\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double4 , 4234.4234567890)) { fprintf (stdout, "error in sequence double4\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double5 , 5234.5234567890)) { fprintf (stdout, "error in sequence double5\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double6 , 6234.6234567890)) { fprintf (stdout, "error in sequence double6\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double7 , 7234.7234567890)) { fprintf (stdout, "error in sequence double7\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double8 , 8234.8234567890)) { fprintf (stdout, "error in sequence double8\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double9 , 9234.9234567890)) { fprintf (stdout, "error in sequence double9\n"); msgindex = 99999; }
                     if (!compare_double(msg->seq_payloaddouble._buffer[j].double10 , 10234.0234567890)) { fprintf (stdout, "error in sequence double10\n"); msgindex = 99999; }
              }

            }
        }

        status = net2497_topic2497DataReader_return_loan(readerA, msgSeqA, infoSeqA);
        checkStatus(status, "net2497_topic2497DataReader_return_loan");

        /* Sleep for some amount of time, as not to consume too much CPU cycles. */
        os_nanoSleep(delay);
    }

    return EXIT_SUCCESS;
}
