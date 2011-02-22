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

#include "ccpp_dds_dcps.h"
#include "ccpp_pingpong.h"

#include <iostream>

using namespace std;
using namespace DDS;
using namespace CORBA;
using namespace pingpong;

//
// Configurable parameters (through cmdline)
// These are the default settings
//
static const char * read_partition = "PING";
static const char * write_partition = "PONG";

int
main (
    int argc,
    char *argv[])
{
    DomainId_t                        myDomain           = NULL;
    DomainParticipantFactory_ptr      dpf                = NULL;
    DomainParticipant_ptr             dp                 = NULL;
    Topic_ptr                         t                  = NULL;
    Publisher_ptr                     p                  = NULL;
    Subscriber_ptr                    s                  = NULL;
    DataWriter_ptr                    dw                 = NULL;
    DataReader_ptr                    dr                 = NULL;

    PP_min_msgDataWriter_ptr          PP_min_writer      = NULL;
    PP_seq_msgDataWriter_ptr          PP_seq_writer      = NULL;
    PP_string_msgDataWriter_ptr       PP_string_writer   = NULL;
    PP_fixed_msgDataWriter_ptr        PP_fixed_writer    = NULL;
    PP_array_msgDataWriter_ptr        PP_array_writer    = NULL;

    PP_min_msgDataReader_ptr          PP_min_reader      = NULL;
    PP_seq_msgDataReader_ptr          PP_seq_reader      = NULL;
    PP_string_msgDataReader_ptr       PP_string_reader   = NULL;
    PP_fixed_msgDataReader_ptr        PP_fixed_reader    = NULL;
    PP_array_msgDataReader_ptr        PP_array_reader    = NULL;
    PP_quit_msgDataReader_ptr         PP_quit_reader     = NULL;

    PP_min_msgTypeSupport             PP_min_dt;
    PP_seq_msgTypeSupport             PP_seq_dt;
    PP_string_msgTypeSupport          PP_string_dt;
    PP_fixed_msgTypeSupport           PP_fixed_dt;
    PP_array_msgTypeSupport           PP_array_dt;
    PP_quit_msgTypeSupport            PP_quit_dt;

    PP_min_msgSeq_var                 PP_min_dataList    = new PP_min_msgSeq;
    PP_seq_msgSeq_var                 PP_seq_dataList    = new PP_seq_msgSeq;
    PP_string_msgSeq_var              PP_string_dataList = new PP_string_msgSeq;
    PP_fixed_msgSeq_var               PP_fixed_dataList  = new PP_fixed_msgSeq;
    PP_array_msgSeq_var               PP_array_dataList  = new PP_array_msgSeq;
    PP_quit_msgSeq_var                PP_quit_dataList   = new PP_quit_msgSeq;

    StatusCondition_ptr               PP_min_sc;
    StatusCondition_ptr               PP_seq_sc;
    StatusCondition_ptr               PP_string_sc;
    StatusCondition_ptr               PP_fixed_sc;
    StatusCondition_ptr               PP_array_sc;
    StatusCondition_ptr               PP_quit_sc;

    Topic_ptr                         PP_min_topic       = NULL;
    Topic_ptr                         PP_seq_topic       = NULL;
    Topic_ptr                         PP_string_topic    = NULL;
    Topic_ptr                         PP_fixed_topic     = NULL;
    Topic_ptr                         PP_array_topic     = NULL;
    Topic_ptr                         PP_quit_topic      = NULL;

    ConditionSeq_var                  conditionList      = new ConditionSeq;
    SampleInfoSeq_var                 infoList           = new SampleInfoSeq;
    WaitSet                           w;

    DomainParticipantQos              dpQos;
    TopicQos                          tQos;
    PublisherQos                      pQos;
    DataWriterQos                     dwQos;
    SubscriberQos                     sQos;
    DataReaderQos                     drQos;

    Boolean                           terminate = FALSE;

    ReturnCode_t                      result;
    CORBA::ULong                      i;
    int                               imax;
    CORBA::ULong                      j;
    int                               jmax;

    //
    // Evaluate cmdline arguments
    //

    if (argc != 1) {
        if (argc != 3) {
            printf ("Invalid.....\n Usage: %s [READ_PARTITION WRITE_PARTITION]\n", argv[0]);
            exit (1);
        }
        read_partition  = argv[1];
        write_partition = argv[2];
    }

    //
    // Create participant
    //

    dpf = DomainParticipantFactory::get_instance ();
    dp = dpf->create_participant (myDomain, PARTICIPANT_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    if (dp == NULL) {
        cout << argv[0] << "PONG: ERROR - Splice Daemon not running";
        exit(1);
    }

    // 
    // Create PONG publisher
    //

    dp->get_default_publisher_qos (pQos);
    pQos.partition.name.length (1);
    pQos.partition.name[0] = string_dup (write_partition);
    p = dp->create_publisher (pQos, NULL, DDS::STATUS_MASK_NONE);
    
    //
    // Create PING subscriber
    //

    dp->get_default_subscriber_qos (sQos);
    sQos.partition.name.length (1);
    sQos.partition.name[0] = string_dup (read_partition);
    s = dp->create_subscriber (sQos, NULL, DDS::STATUS_MASK_NONE);

    //
    // PP_min_msg
    //
    
    //  Create Topic
    PP_min_dt.register_type (dp, "pingpong::PP_min_msg");
    PP_min_topic = dp->create_topic ("PP_min_topic", "pingpong::PP_min_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

    // Create datawriter
    dw = p->create_datawriter (PP_min_topic, DATAWRITER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_min_writer = dynamic_cast<PP_min_msgDataWriter_ptr> (dw);

    // Create datareader
    dr = s->create_datareader (PP_min_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_min_reader = dynamic_cast<PP_min_msgDataReader_ptr> (dr);

    // Add datareader statuscondition to waitset
    PP_min_sc = PP_min_reader->get_statuscondition ();
    PP_min_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
    result = w.attach_condition (PP_min_sc);

    //
    // PP_seq_msg
    //

    //  Create Topic
    PP_seq_dt.register_type (dp, "pingpong::PP_seq_msg");
    PP_seq_topic = dp->create_topic ("PP_seq_topic", "pingpong::PP_seq_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

    // Create datawriter
    dw = p->create_datawriter (PP_seq_topic, DATAWRITER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_seq_writer = dynamic_cast<PP_seq_msgDataWriter_ptr> (dw);

    // Create datareader
    dr = s->create_datareader (PP_seq_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_seq_reader = dynamic_cast<PP_seq_msgDataReader_ptr> (dr);

    // Add datareader statuscondition to waitset
    PP_seq_sc = PP_seq_reader->get_statuscondition ();
    PP_seq_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
    result = w.attach_condition (PP_seq_sc);
    
    //
    // PP_string_msg
    //
    
    //  Create Topic
    PP_string_dt.register_type (dp, "pingpong::PP_string_msg");
    PP_string_topic = dp->create_topic ("PP_string_topic", "pingpong::PP_string_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

    // Create datawriter
    dw = p->create_datawriter (PP_string_topic, DATAWRITER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_string_writer = dynamic_cast<PP_string_msgDataWriter_ptr> (dw);

    // Create datareader
    dr = s->create_datareader (PP_string_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_string_reader = dynamic_cast<PP_string_msgDataReader_ptr> (dr);

    // Add datareader statuscondition to waitset
    PP_string_sc = PP_string_reader->get_statuscondition ();
    PP_string_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
    result = w.attach_condition (PP_string_sc);
    
    //
    // PP_fixed_msg
    //
    
    //  Create Topic
    PP_fixed_dt.register_type (dp, "pingpong::PP_fixed_msg");
    PP_fixed_topic = dp->create_topic ("PP_fixed_topic", "pingpong::PP_fixed_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

    // Create datawriter
    dw = p->create_datawriter (PP_fixed_topic, DATAWRITER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_fixed_writer = dynamic_cast<PP_fixed_msgDataWriter_ptr> (dw);

    // Create datareader
    dr = s->create_datareader (PP_fixed_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_fixed_reader = dynamic_cast<PP_fixed_msgDataReader_ptr> (dr);

    // Add datareader statuscondition to waitset
    PP_fixed_sc = PP_fixed_reader->get_statuscondition ();
    PP_fixed_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
    result = w.attach_condition (PP_fixed_sc);
    
    //
    // PP_array_msg
    //
    
    //  Create Topic
    PP_array_dt.register_type (dp, "pingpong::PP_array_msg");
    PP_array_topic = dp->create_topic ("PP_array_topic", "pingpong::PP_array_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

    // Create datawriter
    dw = p->create_datawriter (PP_array_topic, DATAWRITER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_array_writer = dynamic_cast<PP_array_msgDataWriter_ptr> (dw);

    // Create datareader
    dr = s->create_datareader (PP_array_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_array_reader = dynamic_cast<PP_array_msgDataReader_ptr> (dr);

    // Add datareader statuscondition to waitset
    PP_array_sc = PP_array_reader->get_statuscondition ();
    PP_array_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
    result = w.attach_condition (PP_array_sc);

    //
    // PP_quit_msg
    //
    
    //  Create Topic
    PP_quit_dt.register_type (dp, "pingpong::PP_quit_msg");
    PP_quit_topic = dp->create_topic ("PP_quit_topic", "pingpong::PP_quit_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

    // Create datareader
    dr = s->create_datareader (PP_quit_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_quit_reader = dynamic_cast<PP_quit_msgDataReader_ptr> (dr);

    // Add datareader statuscondition to waitset
    PP_quit_sc = PP_quit_reader->get_statuscondition ();
    PP_quit_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
    result = w.attach_condition (PP_quit_sc);

    while (!terminate) {
        // cout << "PONG: waiting for PING" << endl;
        result = w.wait (conditionList.inout (), DURATION_INFINITE);
        if (result == DDS::RETCODE_ALREADY_DELETED) {
            terminate = TRUE;
            continue;
        }
        imax = conditionList->length ();
        for (i = 0; i < imax; i++) {
            if (conditionList[i].in() == PP_min_sc) {
                // cout << "PONG: PING_min arrived" << endl;
                result = PP_min_reader->take (PP_min_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
                jmax = PP_min_dataList->length ();
                if (jmax != 0) {
                    for (j = 0; j < jmax; j++) {
                        if (infoList[j].valid_data) {
                            result = PP_min_writer->write (PP_min_dataList[j], HANDLE_NIL);
                        }
                    }
                    result = PP_min_reader->return_loan (PP_min_dataList, infoList);
                } else {
                    cout << "PONG: PING_min triggered, but no data available" << endl;
                }
            } else if (conditionList[i].in() == PP_seq_sc) {
                // cout << "PONG: PING_seq arrived" << endl;
                result = PP_seq_reader->take (PP_seq_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
                jmax = PP_seq_dataList->length ();
                if (jmax != 0) {
                    for (j = 0; j < jmax; j++) {
                        if (infoList[j].valid_data) {
                            result = PP_seq_writer->write (PP_seq_dataList[j], HANDLE_NIL);
                        }
                    }
                    result = PP_seq_reader->return_loan (PP_seq_dataList, infoList);
                } else {
                    cout << "PONG: PING_seq triggered, but no data available" << endl;
                }
            } else if (conditionList[i].in() == PP_string_sc) {
                // cout << "PONG: PING_string arrived" << endl;
                result = PP_string_reader->take (PP_string_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
                jmax = PP_string_dataList->length ();
                if (jmax != 0) {
                    for (j = 0; j < jmax; j++) {
                        if (infoList[j].valid_data) {
                            result = PP_string_writer->write (PP_string_dataList[j], HANDLE_NIL);
                        }
                    }
                    result = PP_string_reader->return_loan (PP_string_dataList, infoList);
                } else {
                    cout << "PONG: PING_string triggered, but no data available" << endl;
                    exit(1);
                }
            } else if (conditionList[i].in() == PP_fixed_sc) {
                // cout << "PONG: PING_fixed arrived" << endl;
                result = PP_fixed_reader->take (PP_fixed_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
                jmax = PP_fixed_dataList->length ();
                if (jmax != 0) {
                    for (j = 0; j < jmax; j++) {
                        if (infoList[j].valid_data) {
                            result = PP_fixed_writer->write (PP_fixed_dataList[j], HANDLE_NIL);
                        }
                    }
                    result = PP_fixed_reader->return_loan (PP_fixed_dataList, infoList);
                } else {
                    cout << "PONG: PING_fixed triggered, but no data available" << endl;
                }
            } else if (conditionList[i].in() == PP_array_sc) {
                // cout << "PONG: PING_array arrived" << endl;
                result = PP_array_reader->take (PP_array_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
                jmax = PP_array_dataList->length ();
                if (jmax != 0) {
                    for (j = 0; j < jmax; j++) {
                        if (infoList[j].valid_data) {
                            result = PP_array_writer->write (PP_array_dataList[j], HANDLE_NIL);
                        }
                    }
                    result = PP_array_reader->return_loan (PP_array_dataList, infoList);
                } else {
                    cout << "PONG: PING_array triggered, but no data available" << endl;
                }
            } else if (conditionList[i].in() == PP_quit_sc) {
                // cout << "PONG: PING_quit arrived" << endl;
                result = PP_quit_reader->take (PP_quit_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
                jmax = PP_quit_dataList->length ();
                if (jmax != 0) {
                    if (PP_quit_dataList[0].quit) {
                        terminate = TRUE;
                    }
                    result = PP_quit_reader->return_loan (PP_quit_dataList, infoList);
                } else {
                    cout << "PONG: PING_quit triggered, but no data available" << endl;
                }
            } else {
                cout << "PONG: unknown condition triggered: " <<  conditionList[i].in() << endl;
            }
        }
    }

    result = s->delete_datareader (PP_min_reader);
    result = p->delete_datawriter (PP_min_writer);
    result = s->delete_datareader (PP_seq_reader);
    result = p->delete_datawriter (PP_seq_writer);
    result = s->delete_datareader (PP_string_reader);
    result = p->delete_datawriter (PP_string_writer);
    result = s->delete_datareader (PP_fixed_reader);
    result = p->delete_datawriter (PP_fixed_writer);
    result = s->delete_datareader (PP_array_reader);
    result = p->delete_datawriter (PP_array_writer);
    result = s->delete_datareader (PP_quit_reader);
    result = dp->delete_subscriber (s);
    result = dp->delete_publisher (p);
    result = dp->delete_topic (PP_min_topic);
    result = dp->delete_topic (PP_seq_topic);
    result = dp->delete_topic (PP_string_topic);
    result = dp->delete_topic (PP_fixed_topic);
    result = dp->delete_topic (PP_array_topic);
    result = dp->delete_topic (PP_quit_topic);
    result = dpf->delete_participant (dp);
    
    return 0;
}
