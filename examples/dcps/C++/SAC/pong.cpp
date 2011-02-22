/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "dds_dcps.h"
#include "pingpong.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Configurable parameters (through cmdline)
 * These are the default settings
 */
static const char * read_partition = "PING";
static const char * write_partition = "PONG";

int
main (
    int argc,
    char *argv[])
{
    DDS_DomainId_t                             myDomain           = DDS_OBJECT_NIL;
    DDS_DomainParticipantFactory               dpf                = DDS_OBJECT_NIL;
    DDS_DomainParticipant                      dp                 = DDS_OBJECT_NIL;
    DDS_Publisher                              p                  = DDS_OBJECT_NIL;
    DDS_Subscriber                             s                  = DDS_OBJECT_NIL;

    pingpong_PP_min_msgDataWriter              PP_min_writer      = DDS_OBJECT_NIL;
    pingpong_PP_seq_msgDataWriter              PP_seq_writer      = DDS_OBJECT_NIL;
    pingpong_PP_string_msgDataWriter           PP_string_writer   = DDS_OBJECT_NIL;
    pingpong_PP_fixed_msgDataWriter            PP_fixed_writer    = DDS_OBJECT_NIL;
    pingpong_PP_array_msgDataWriter            PP_array_writer    = DDS_OBJECT_NIL;
    pingpong_PP_bseq_msgDataWriter      PP_bseq_writer    = DDS_OBJECT_NIL;

    pingpong_PP_min_msgDataReader              PP_min_reader      = DDS_OBJECT_NIL;
    pingpong_PP_seq_msgDataReader              PP_seq_reader      = DDS_OBJECT_NIL;
    pingpong_PP_string_msgDataReader           PP_string_reader   = DDS_OBJECT_NIL;
    pingpong_PP_fixed_msgDataReader            PP_fixed_reader    = DDS_OBJECT_NIL;
    pingpong_PP_array_msgDataReader            PP_array_reader    = DDS_OBJECT_NIL;
    pingpong_PP_bseq_msgDataReader      PP_bseq_reader    = DDS_OBJECT_NIL;
    pingpong_PP_quit_msgDataReader             PP_quit_reader     = DDS_OBJECT_NIL;

    pingpong_PP_min_msgTypeSupport             PP_min_dt;
    pingpong_PP_seq_msgTypeSupport             PP_seq_dt;
    pingpong_PP_string_msgTypeSupport          PP_string_dt;
    pingpong_PP_fixed_msgTypeSupport           PP_fixed_dt;
    pingpong_PP_array_msgTypeSupport           PP_array_dt;
    pingpong_PP_bseq_msgTypeSupport     PP_bseq_dt;
    pingpong_PP_quit_msgTypeSupport            PP_quit_dt;

    DDS_sequence_pingpong_PP_min_msg           PP_min_dataList    = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_sequence_pingpong_PP_seq_msg           PP_seq_dataList    = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_sequence_pingpong_PP_string_msg        PP_string_dataList = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_sequence_pingpong_PP_fixed_msg         PP_fixed_dataList  = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_sequence_pingpong_PP_array_msg         PP_array_dataList  = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_sequence_pingpong_PP_bseq_msg   PP_bseq_dataList  = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_sequence_pingpong_PP_quit_msg          PP_quit_dataList   = { 0, 0, DDS_OBJECT_NIL, FALSE };

    DDS_StatusCondition                        PP_min_sc;
    DDS_StatusCondition                        PP_seq_sc;
    DDS_StatusCondition                        PP_string_sc;
    DDS_StatusCondition                        PP_fixed_sc;
    DDS_StatusCondition                        PP_array_sc;
    DDS_StatusCondition                        PP_bseq_sc;
    DDS_StatusCondition                        PP_quit_sc;

    DDS_Topic                                  PP_min_topic       = DDS_OBJECT_NIL;
    DDS_Topic                                  PP_seq_topic       = DDS_OBJECT_NIL;
    DDS_Topic                                  PP_string_topic    = DDS_OBJECT_NIL;
    DDS_Topic                                  PP_fixed_topic     = DDS_OBJECT_NIL;
    DDS_Topic                                  PP_array_topic     = DDS_OBJECT_NIL;
    DDS_Topic                                  PP_bseq_topic     = DDS_OBJECT_NIL;
    DDS_Topic                                  PP_quit_topic      = DDS_OBJECT_NIL;

    DDS_ConditionSeq                          *conditionList;
    DDS_SampleInfoSeq                          infoList           = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_WaitSet                                w;

    DDS_DomainParticipantQos                  *dpQos;
    DDS_TopicQos                              *tQos;
    DDS_PublisherQos                          *pQos;
    DDS_DataWriterQos                         *dwQos;
    DDS_SubscriberQos                         *sQos;
    DDS_DataReaderQos                         *drQos;

    DDS_boolean                                terminate = FALSE;

    DDS_ReturnCode_t                           result;
    int                                        i;
    int                                        imax;
    int                                        j;
    int                                        jmax;

    printf ("Starting pong example\n");
    fflush(stdout);
    /*
     * Evaluate cmdline arguments
     */

#ifdef INTEGRITY
    read_partition = "PongRead";
    write_partition = "PongWrite";
#else
    if (argc != 1) {
        if (argc != 3) {
            printf ("Invalid.....\n Usage: %s [READ_PARTITION WRITE_PARTITION]\n", argv[0]);
            exit (1);
        }
        read_partition  = argv[1];
        write_partition = argv[2];
    }
#endif

    /*
     * Create WaitSet
     */
    w = DDS_WaitSet__alloc ();
    /*
     * Initialize Qos variables
     */ 
    dpQos = DDS_DomainParticipantQos__alloc();
    tQos  = DDS_TopicQos__alloc();
    pQos  = DDS_PublisherQos__alloc();
    dwQos = DDS_DataWriterQos__alloc();
    sQos  = DDS_SubscriberQos__alloc();
    drQos = DDS_DataReaderQos__alloc();
    /*
     * Initialize condition list
     */
    conditionList = NULL;

    /*
     * Create participant
     */
    dpf = DDS_DomainParticipantFactory_get_instance ();
    dp = DDS_DomainParticipantFactory_create_participant (dpf, myDomain, DDS_PARTICIPANT_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
    if (dp == DDS_OBJECT_NIL) {
        printf ("%s PONG: ERROR - Splice Daemon not running\n", argv[0]);
        exit(1);
    }

    /* 
     * Create PONG publisher
     */
    DDS_DomainParticipant_get_default_publisher_qos (dp, pQos);
    pQos->partition.name._length = 1;
    pQos->partition.name._maximum = 1;
    pQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    pQos->partition.name._buffer[0] = DDS_string_alloc (strlen (write_partition) + 1);
    strcpy (pQos->partition.name._buffer[0], write_partition);
    p = DDS_DomainParticipant_create_publisher (dp, pQos, NULL, DDS_STATUS_MASK_NONE);
    DDS_free (pQos);

    /*
     * Create PING subscriber
     */
    DDS_DomainParticipant_get_default_subscriber_qos (dp, sQos);
    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_alloc (strlen (read_partition) + 1);
    strcpy (sQos->partition.name._buffer[0], read_partition);
    s = DDS_DomainParticipant_create_subscriber (dp, sQos, NULL, DDS_STATUS_MASK_NONE);
    DDS_free (sQos);

    /*
     * PP_min_msg
     */
    
    /* Create Topic */
    PP_min_dt = pingpong_PP_min_msgTypeSupport__alloc ();
    pingpong_PP_min_msgTypeSupport_register_type (PP_min_dt, dp, "pingpong::PP_min_msg");
    PP_min_topic = DDS_DomainParticipant_create_topic (dp, "PP_min_topic", "pingpong::PP_min_msg", DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datawriter */
    PP_min_writer = DDS_Publisher_create_datawriter (p, PP_min_topic, DDS_DATAWRITER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datareader */
    PP_min_reader = DDS_Subscriber_create_datareader (s, PP_min_topic, DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Add datareader statuscondition to waitset */
    PP_min_sc = DDS_DataReader_get_statuscondition (PP_min_reader);
    DDS_StatusCondition_set_enabled_statuses (PP_min_sc, DDS_DATA_AVAILABLE_STATUS);
    result = DDS_WaitSet_attach_condition (w, PP_min_sc);

    /*
     * PP_seq_msg
     */

    /*  Create Topic */
    PP_seq_dt = pingpong_PP_seq_msgTypeSupport__alloc ();
    pingpong_PP_seq_msgTypeSupport_register_type (PP_seq_dt, dp, "pingpong::PP_seq_msg");
    PP_seq_topic = DDS_DomainParticipant_create_topic (dp, "PP_seq_topic", "pingpong::PP_seq_msg", DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datawriter */
    PP_seq_writer = DDS_Publisher_create_datawriter (p, PP_seq_topic, DDS_DATAWRITER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datareader */
    PP_seq_reader = DDS_Subscriber_create_datareader (s, PP_seq_topic, DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Add datareader statuscondition to waitset */
    PP_seq_sc = DDS_DataReader_get_statuscondition (PP_seq_reader);
    DDS_StatusCondition_set_enabled_statuses (PP_seq_sc, DDS_DATA_AVAILABLE_STATUS);
    result = DDS_WaitSet_attach_condition (w, PP_seq_sc);

    /*
     * PP_string_msg
     */

    /*  Create Topic */
    PP_string_dt = pingpong_PP_string_msgTypeSupport__alloc ();
    pingpong_PP_string_msgTypeSupport_register_type (PP_string_dt, dp, "pingpong::PP_string_msg");
    PP_string_topic = DDS_DomainParticipant_create_topic (dp, "PP_string_topic", "pingpong::PP_string_msg", DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datawriter */
    PP_string_writer = DDS_Publisher_create_datawriter (p, PP_string_topic, DDS_DATAWRITER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datareader */
    PP_string_reader = DDS_Subscriber_create_datareader (s, PP_string_topic, DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Add datareader statuscondition to waitset */
    PP_string_sc = DDS_DataReader_get_statuscondition (PP_string_reader);
    DDS_StatusCondition_set_enabled_statuses (PP_string_sc, DDS_DATA_AVAILABLE_STATUS);
    result = DDS_WaitSet_attach_condition (w, PP_string_sc);

    /*
     * PP_fixed_msg
     */
    
    /*  Create Topic */
    PP_fixed_dt = pingpong_PP_fixed_msgTypeSupport__alloc ();
    pingpong_PP_fixed_msgTypeSupport_register_type (PP_fixed_dt, dp, "pingpong::PP_fixed_msg");
    PP_fixed_topic = DDS_DomainParticipant_create_topic (dp, "PP_fixed_topic", "pingpong::PP_fixed_msg", DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datawriter */
    PP_fixed_writer = DDS_Publisher_create_datawriter (p, PP_fixed_topic, DDS_DATAWRITER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datareader */
    PP_fixed_reader = DDS_Subscriber_create_datareader (s, PP_fixed_topic, DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Add datareader statuscondition to waitset */
    PP_fixed_sc = DDS_DataReader_get_statuscondition (PP_fixed_reader);
    DDS_StatusCondition_set_enabled_statuses (PP_fixed_sc, DDS_DATA_AVAILABLE_STATUS);
    result = DDS_WaitSet_attach_condition (w, PP_fixed_sc);

    /*
     * PP_array_msg
     */

    /*  Create Topic */
    PP_array_dt = pingpong_PP_array_msgTypeSupport__alloc ();
    pingpong_PP_array_msgTypeSupport_register_type (PP_array_dt, dp, "pingpong::PP_array_msg");
    PP_array_topic = DDS_DomainParticipant_create_topic (dp, "PP_array_topic", "pingpong::PP_array_msg", DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datawriter */
    PP_array_writer = DDS_Publisher_create_datawriter (p, PP_array_topic, DDS_DATAWRITER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datareader */
    PP_array_reader = DDS_Subscriber_create_datareader (s, PP_array_topic, DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Add datareader statuscondition to waitset */
    PP_array_sc = DDS_DataReader_get_statuscondition (PP_array_reader);
    DDS_StatusCondition_set_enabled_statuses (PP_array_sc, DDS_DATA_AVAILABLE_STATUS);
    result = DDS_WaitSet_attach_condition (w, PP_array_sc);

    /*
     * PP_bseq_msg
     */

    /*  Create Topic */
    PP_bseq_dt = pingpong_PP_bseq_msgTypeSupport__alloc ();
    pingpong_PP_bseq_msgTypeSupport_register_type (PP_bseq_dt, dp, "pingpong::PP_bseq_msg");
    PP_bseq_topic = DDS_DomainParticipant_create_topic (dp, "PP_bseq_topic", "pingpong::PP_bseq_msg", DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datawriter */
    PP_bseq_writer = DDS_Publisher_create_datawriter (p, PP_bseq_topic, DDS_DATAWRITER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datareader */
    PP_bseq_reader = DDS_Subscriber_create_datareader (s, PP_bseq_topic, DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Add datareader statuscondition to waitset */
    PP_bseq_sc = DDS_DataReader_get_statuscondition (PP_bseq_reader);
    DDS_StatusCondition_set_enabled_statuses (PP_bseq_sc, DDS_DATA_AVAILABLE_STATUS);
    result = DDS_WaitSet_attach_condition (w, PP_bseq_sc);

    /*
     * PP_quit_msg
     */
    
    /*  Create Topic */
    PP_quit_dt = pingpong_PP_quit_msgTypeSupport__alloc ();
    pingpong_PP_quit_msgTypeSupport_register_type (PP_quit_dt, dp, "pingpong::PP_quit_msg");
    PP_quit_topic = DDS_DomainParticipant_create_topic (dp, "PP_quit_topic", "pingpong::PP_quit_msg", DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datareader */
    PP_quit_reader = DDS_Subscriber_create_datareader (s, PP_quit_topic, DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Add datareader statuscondition to waitset */
    PP_quit_sc = DDS_DataReader_get_statuscondition (PP_quit_reader);
    DDS_StatusCondition_set_enabled_statuses (PP_quit_sc, DDS_DATA_AVAILABLE_STATUS);
    result = DDS_WaitSet_attach_condition (w, PP_quit_sc);

    while (!terminate) {
	DDS_Duration_t wait_timeout = DDS_DURATION_INFINITE;

        /* printf ("PONG: waiting for PING\n"); */
        conditionList = DDS_ConditionSeq__alloc();
        result = DDS_WaitSet_wait (w, conditionList, &wait_timeout);

        if (result == DDS_RETCODE_OK) {
            imax = conditionList->_length;
            for (i = 0; i < imax; i++) {
                if (conditionList->_buffer[i] == PP_min_sc) {
    		    /* printf ("PONG: PING_min arrived\n"); */
                    result = pingpong_PP_min_msgDataReader_take (PP_min_reader, &PP_min_dataList, &infoList, DDS_LENGTH_UNLIMITED,
                                 DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
                    jmax = PP_min_dataList._length;
                    if (jmax != 0) {
                        for (j = 0; j < jmax; j++) {
                            if (infoList._buffer[j].valid_data) {
                                result = pingpong_PP_min_msgDataWriter_write (PP_min_writer, &PP_min_dataList._buffer[j], DDS_HANDLE_NIL);
                            }
                        }
                        result = pingpong_PP_min_msgDataReader_return_loan (PP_min_reader, &PP_min_dataList, &infoList);
                    } else {
                        printf ("PONG: PING_min triggered, but no data available\n");
                    }
                } else if (conditionList->_buffer[i] == PP_seq_sc) {
    		    /* printf ("PONG: PING_seq arrived\n"); */
                    result = pingpong_PP_seq_msgDataReader_take (PP_seq_reader, &PP_seq_dataList, &infoList, DDS_LENGTH_UNLIMITED,
                                 DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
                    jmax = PP_seq_dataList._length;
                    if (jmax != 0) {
                        for (j = 0; j < jmax; j++) {
                            if (infoList._buffer[j].valid_data) {
                                result = pingpong_PP_seq_msgDataWriter_write (PP_seq_writer, &PP_seq_dataList._buffer[j], DDS_HANDLE_NIL);
                            }
                        }
                        result = pingpong_PP_seq_msgDataReader_return_loan (PP_seq_reader, &PP_seq_dataList, &infoList);
                    } else {
                        printf ("PONG: PING_seq triggered, but no data available\n");
                    }
                } else if (conditionList->_buffer[i] == PP_string_sc) {
    		    /* printf ("PONG: PING_string arrived\n"); */
                    result = pingpong_PP_string_msgDataReader_take (PP_string_reader, &PP_string_dataList, &infoList, DDS_LENGTH_UNLIMITED,
                                 DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
                    jmax = PP_string_dataList._length;
                    if (jmax != 0) {
                        for (j = 0; j < jmax; j++) {
                            if (infoList._buffer[j].valid_data) {
                                result = pingpong_PP_string_msgDataWriter_write (PP_string_writer, &PP_string_dataList._buffer[j], DDS_HANDLE_NIL);
                            }
                        }
                        result = pingpong_PP_string_msgDataReader_return_loan (PP_string_reader, &PP_string_dataList, &infoList);
                    } else {
                        printf ("PONG: PING_string triggered, but no data available\n");
                        exit(1);
                    }
                } else if (conditionList->_buffer[i] == PP_fixed_sc) {
    		    /* printf ("PONG: PING_fixed arrived\n"); */
                    result = pingpong_PP_fixed_msgDataReader_take (PP_fixed_reader, &PP_fixed_dataList, &infoList, DDS_LENGTH_UNLIMITED,
                                 DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
                    jmax = PP_fixed_dataList._length;
                    if (jmax != 0) {
                        for (j = 0; j < jmax; j++) {
                            if (infoList._buffer[j].valid_data) {
                                result = pingpong_PP_fixed_msgDataWriter_write (PP_fixed_writer, &PP_fixed_dataList._buffer[j], DDS_HANDLE_NIL);
                            }
                        }
                        result = pingpong_PP_fixed_msgDataReader_return_loan (PP_fixed_reader, &PP_fixed_dataList, &infoList);
                    } else {
                        printf ("PONG: PING_fixed triggered, but no data available\n");
                    }
                } else if (conditionList->_buffer[i] == PP_array_sc) {
    		    /* printf ("PONG: PING_array arrived\n"); */
                    result = pingpong_PP_array_msgDataReader_take (PP_array_reader, &PP_array_dataList, &infoList, DDS_LENGTH_UNLIMITED,
                                 DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
                    jmax = PP_array_dataList._length;
                    if (jmax != 0) {
                        for (j = 0; j < jmax; j++) {
                            if (infoList._buffer[j].valid_data) {
                                result = pingpong_PP_array_msgDataWriter_write (PP_array_writer, &PP_array_dataList._buffer[j], DDS_HANDLE_NIL);
                            }
                        }
                        result = pingpong_PP_array_msgDataReader_return_loan (PP_array_reader, &PP_array_dataList, &infoList);
                    } else {
                        printf ("PONG: PING_array triggered, but no data available\n");
                    }
                } else if (conditionList->_buffer[i] == PP_bseq_sc) {
    		    /* printf ("PONG: PING_bseq arrived\n"); */
                    result = pingpong_PP_bseq_msgDataReader_take (PP_bseq_reader, &PP_bseq_dataList, &infoList, DDS_LENGTH_UNLIMITED,
                                 DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
                    jmax = PP_bseq_dataList._length;
                    if (jmax != 0) {
                        for (j = 0; j < jmax; j++) {
                            if (infoList._buffer[j].valid_data) {
                                result = pingpong_PP_bseq_msgDataWriter_write (PP_bseq_writer, &PP_bseq_dataList._buffer[j], DDS_HANDLE_NIL);
                            }
                        }
                        result = pingpong_PP_bseq_msgDataReader_return_loan (PP_bseq_reader, &PP_bseq_dataList, &infoList);
                    } else {
                        printf ("PONG: PING_bseq triggered, but no data available\n");
                    }
                } else if (conditionList->_buffer[i] == PP_quit_sc) {
    		    /* printf ("PONG: PING_quit arrived\n"); */
                    result = pingpong_PP_quit_msgDataReader_take (PP_quit_reader, &PP_quit_dataList, &infoList, DDS_LENGTH_UNLIMITED,
                                 DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
                    jmax = PP_quit_dataList._length;
                    if (jmax != 0) {
                        if (PP_quit_dataList._buffer[0].quit) {
                            terminate = TRUE;
                        }
                        result = pingpong_PP_quit_msgDataReader_return_loan (PP_quit_reader, &PP_quit_dataList, &infoList);
                    } else {
                        printf ("PONG: PING_quit triggered, but no data available\n");
                    }
                } else {
                    printf ("PONG: unknown condition triggered: %lx\n", (unsigned long)conditionList->_buffer[i]);
                }
            }
        } else {
            if (result == DDS_RETCODE_ALREADY_DELETED) {
                terminate = TRUE;
            }
            printf ("PONG: wait returned not ok: %d\n", result);
        }
        DDS_free(conditionList);
    }

    result = DDS_Subscriber_delete_datareader (s, PP_min_reader);
    result = DDS_Publisher_delete_datawriter (p, PP_min_writer);
    result = DDS_Subscriber_delete_datareader (s, PP_seq_reader);
    result = DDS_Publisher_delete_datawriter (p, PP_seq_writer);
    result = DDS_Subscriber_delete_datareader (s, PP_string_reader);
    result = DDS_Publisher_delete_datawriter (p, PP_string_writer);
    result = DDS_Subscriber_delete_datareader (s, PP_fixed_reader);
    result = DDS_Publisher_delete_datawriter (p, PP_fixed_writer);
    result = DDS_Subscriber_delete_datareader (s, PP_array_reader);
    result = DDS_Publisher_delete_datawriter (p, PP_array_writer);
    result = DDS_Subscriber_delete_datareader (s, PP_bseq_reader);
    result = DDS_Publisher_delete_datawriter (p, PP_bseq_writer);
    result = DDS_Subscriber_delete_datareader (s, PP_quit_reader);
    result = DDS_DomainParticipant_delete_subscriber (dp, s);
    result = DDS_DomainParticipant_delete_publisher (dp, p);
    result = DDS_DomainParticipant_delete_topic (dp, PP_min_topic);
    result = DDS_DomainParticipant_delete_topic (dp, PP_seq_topic);
    result = DDS_DomainParticipant_delete_topic (dp, PP_string_topic);
    result = DDS_DomainParticipant_delete_topic (dp, PP_fixed_topic);
    result = DDS_DomainParticipant_delete_topic (dp, PP_array_topic);
    result = DDS_DomainParticipant_delete_topic (dp, PP_bseq_topic);
    result = DDS_DomainParticipant_delete_topic (dp, PP_quit_topic);
    result = DDS_DomainParticipantFactory_delete_participant (dpf, dp);
    DDS_free (w);
    DDS_free (PP_min_dt);
    DDS_free (PP_seq_dt);
    DDS_free (PP_string_dt);
    DDS_free (PP_fixed_dt);
    DDS_free (PP_array_dt);
    DDS_free (PP_bseq_dt);
    DDS_free (PP_quit_dt);
    DDS_free (dpQos);
    DDS_free (tQos);
    DDS_free (dwQos);
    DDS_free (drQos);

    printf ("Completed pong example\n");
    fflush(stdout);
    return 0;
}
