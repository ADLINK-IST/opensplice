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
#include <time.h>
#include <sys/time.h>

#define SEQ_PAYLOAD_SIZE (1000)

// What it does: 
//   It send a message on the "PING" partition, which the PONG test is waiting for.
//   The PONG test will send the same message back on the "PONG" partition, which 
//   the PING test is waiting for. This sequence is repeated a configurable number 
//   of times.
//   The PING tests measures:
//                write_access-time: time the write() method took.
//                read_access-time:  time the take() method took.
//                round_trip-time:   time between the call to the write() method 
//                                   and the return of the take() method.
//   PING calculates min/max/average statistics on these values over configurable 
//   data blocks.
//
// Configurable:
//   - blocksize: number of roundtrips in each statistics calculation
//   - #blocks:   how many times such a statistics calculation is run
//   - topic:     for the topic, there's a choice between several preconfigured
//                topics.
//   - PING and PONG partition: this enables to use several PING-PONG pairs
//     simultanious with them interfering with each other. It also enables 
//     creating larger loops, by chaining several PONG tests to one PING test.

using namespace std;
using namespace DDS;
using namespace CORBA;
using namespace pingpong;

//
// Type Definitions
//

typedef struct
{
    char name[20];
    double average;
    double min;
    double max;
    int count;
} stats_type;

typedef Boolean pong_handler (unsigned int max_count);

//
// configurable parameters ( through cmdline)
//

static unsigned int nof_cycles = 100;
static unsigned int nof_blocks = 20;
static char topic_id  = 's';

static const char * write_partition = "PING";
static const char * read_partition  = "PONG";

//
// Global Variables
//

static DomainId_t                   	 myDomain           = NULL;
static DomainParticipantFactory_ptr 	 dpf                = NULL;
static DomainParticipant_ptr        	 dp                 = NULL;
static Publisher_ptr                	 p                  = NULL;
static Subscriber_ptr               	 s                  = NULL;
static DataWriter_ptr               	 dw                 = NULL;
static DataReader_ptr               	 dr                 = NULL;

static PP_min_msgDataWriter_ptr          PP_min_writer      = NULL;
static PP_seq_msgDataWriter_ptr          PP_seq_writer      = NULL;
static PP_string_msgDataWriter_ptr       PP_string_writer   = NULL;
static PP_fixed_msgDataWriter_ptr        PP_fixed_writer    = NULL;
static PP_array_msgDataWriter_ptr        PP_array_writer    = NULL;
static PP_quit_msgDataWriter_ptr         PP_quit_writer     = NULL;

static PP_min_msgDataReader_ptr          PP_min_reader      = NULL;
static PP_seq_msgDataReader_ptr          PP_seq_reader      = NULL;
static PP_string_msgDataReader_ptr       PP_string_reader   = NULL;
static PP_fixed_msgDataReader_ptr        PP_fixed_reader    = NULL;
static PP_array_msgDataReader_ptr        PP_array_reader    = NULL;

static PP_min_msgTypeSupport             PP_min_dt;
static PP_seq_msgTypeSupport             PP_seq_dt;
static PP_string_msgTypeSupport          PP_string_dt;
static PP_fixed_msgTypeSupport           PP_fixed_dt;
static PP_array_msgTypeSupport           PP_array_dt;
static PP_quit_msgTypeSupport            PP_quit_dt;

static PP_min_msgSeq_var                 PP_min_dataList    = new PP_min_msgSeq;
static PP_seq_msgSeq_var                 PP_seq_dataList    = new PP_seq_msgSeq;
static PP_string_msgSeq_var              PP_string_dataList = new PP_string_msgSeq;
static PP_fixed_msgSeq_var               PP_fixed_dataList  = new PP_fixed_msgSeq;
static PP_array_msgSeq_var               PP_array_dataList  = new PP_array_msgSeq;
static PP_quit_msgSeq_var                PP_quit_dataList   = new PP_quit_msgSeq;

static StatusCondition_ptr               PP_min_sc          = NULL;
static StatusCondition_ptr               PP_seq_sc          = NULL;
static StatusCondition_ptr               PP_string_sc       = NULL;
static StatusCondition_ptr               PP_fixed_sc        = NULL;
static StatusCondition_ptr               PP_array_sc        = NULL;

static Topic_ptr                         PP_min_topic       = NULL;
static Topic_ptr                         PP_seq_topic       = NULL;
static Topic_ptr                         PP_string_topic    = NULL;
static Topic_ptr                         PP_fixed_topic     = NULL;
static Topic_ptr                         PP_array_topic     = NULL;
static Topic_ptr                         PP_quit_topic      = NULL;

static struct timeval                    roundTripTime;
static struct timeval                    preWriteTime;
static struct timeval                    postWriteTime;
static struct timeval                    preTakeTime;
static struct timeval                    postTakeTime;

static stats_type                        roundtrip;
static stats_type                        write_access;
static stats_type                        read_access;

//
// Static functions
//

static void
add_stats (
    stats_type& stats,
    double data
    )
{
    stats.average = (stats.count * stats.average + data)/(stats.count + 1);
    stats.min     = (stats.count == 0 || data < stats.min) ? data : stats.min;
    stats.max     = (stats.count == 0 || data > stats.max) ? data : stats.max;
    stats.count++;
}

static void
init_stats (
    stats_type& stats,
    const char *name)
{
    strncpy ((char *)stats.name, name, 19);
    stats.name[19] = '\0';
    stats.count    = 0;
    stats.average  = 0.0;
    stats.min      = 0.0;
    stats.max      = 0.0; 
}

struct timeval
timeGet (
    void
    )
{
    struct timeval current_time;

    gettimeofday (&current_time, NULL);

    return current_time;
}

static struct timeval
timeSub (
    struct timeval t1,
    struct timeval t2)
{
    struct timeval tr;

    if (t1.tv_usec >= t2.tv_usec) {
        tr.tv_usec = t1.tv_usec - t2.tv_usec;
        tr.tv_sec = t1.tv_sec - t2.tv_sec;
    } else {
        tr.tv_usec = t1.tv_usec - t2.tv_usec + 1000000;
        tr.tv_sec = t1.tv_sec - t2.tv_sec - 1;
    }
    return tr;
}

static double
timeToReal (
    struct timeval t)
{
    double tr;

    tr = (double)t.tv_sec + (double)t.tv_usec / (double)1000000.0;
    return tr;
}

static Boolean
PP_min_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    Boolean           result = false;
    ReturnCode_t      dds_result;

    // cout << "PING: PING_min arrived" << endl;
    
    preTakeTime = timeGet ();
    dds_result = PP_min_reader->take (PP_min_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_min_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_min_dataList->length () << " msg received" << endl;
        }
        PP_min_dataList[0U].count++;
        if (PP_min_dataList[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            dds_result = PP_min_writer->write (PP_min_dataList[0], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats(read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats(roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        dds_result = PP_min_reader->return_loan (PP_min_dataList, infoList);
    } else {
        cout << "PING: PING_min triggered, but no data available" << endl;
    }
    return result;
}

static Boolean
PP_seq_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    Boolean           result = false;
    ReturnCode_t      dds_result;

    // cout << "PING: PING_seq arrived" << endl;

    preTakeTime = timeGet ();
    dds_result = PP_seq_reader->take (PP_seq_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();

    amount = PP_seq_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_seq_dataList->length () << " msg received" << endl;
        }
        PP_seq_dataList[0U].count++;
        if (PP_seq_dataList[0U].count < nof_cycles) {
            preWriteTime = timeGet ();
            dds_result = PP_seq_writer->write (PP_seq_dataList[0U], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        dds_result = PP_seq_reader->return_loan (PP_seq_dataList, infoList);
    } else {
        cout << "PING: PING_seq triggered, but no data available" << endl;
    }

    return result;
}

static Boolean
PP_string_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    Boolean           result = false;
    ReturnCode_t      dds_result;
    
    // cout << "PING: PING_string arrived" << endl;
    
    preTakeTime = timeGet ();
    dds_result = PP_string_reader->take (PP_string_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_string_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_string_dataList->length () << " msg received" << endl;
        }
        PP_string_dataList[0U].count++;
        if (PP_string_dataList[0U].count < nof_cycles) {
            preWriteTime = timeGet ();
            dds_result = PP_string_writer->write (PP_string_dataList[0U], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        dds_result = PP_string_reader->return_loan (PP_string_dataList, infoList);
    } else {
        cout << "PING: PING_string triggered, but no data available" << endl;
    }
    return result;
}

static Boolean
PP_fixed_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    Boolean           result = false;
    ReturnCode_t      dds_result;
    
    // cout << "PING: PING_fixed arrived" << endl;
    
    preTakeTime = timeGet ();
    dds_result = PP_fixed_reader->take (PP_fixed_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_fixed_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_fixed_dataList->length () << " msg received" << endl;
        }
        PP_fixed_dataList[0U].count++;
        if (PP_fixed_dataList[0U].count < nof_cycles ) {
            preWriteTime = timeGet ();
            dds_result = PP_fixed_writer->write (PP_fixed_dataList[0U], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        dds_result = PP_fixed_reader->return_loan (PP_fixed_dataList, infoList);
    } else {
        cout << "PING: PING_fixed triggered, but no data available" << endl;
    }
    return result;
}

static
Boolean PP_array_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    Boolean           result = false;
    ReturnCode_t      dds_result;
    
    // cout << "PING: PING_array arrived" << endl;
    
    preTakeTime = timeGet ();
    dds_result = PP_array_reader->take (PP_array_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_array_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_array_dataList->length () << " msg received" << endl;
        }
        PP_array_dataList[0].count++;
        if (PP_array_dataList[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            dds_result = PP_array_writer->write (PP_array_dataList[0], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        dds_result = PP_array_reader->return_loan (PP_array_dataList, infoList);
    } else {
        cout << "PING: PING_array triggered, but no data available" << endl;
    }
    return result;
}

//
// M A I N
//

int
main (
    int argc,
    char *argv[]
    )
{
    ConditionSeq_var                     conditionList = new ConditionSeq;
    WaitSet                              w;
    Condition_ptr                        exp_condition;
    pong_handler                        *active_handler;

    DomainParticipantQos                 dpQos;
    TopicQos                             tQos;
    PublisherQos                         pQos;
    DataWriterQos                        dwQos;
    SubscriberQos                        sQos;
    DataReaderQos                        drQos;
    
    time_t                               clock = time (NULL);
    Duration_t                           wait_timeout = {3,0};

    ReturnCode_t                         result;
    Boolean                              finish_flag = false;
    Boolean                              timeout_flag = false;
    Boolean                              terminate = false;

    int                                  imax = 1;
    int                                  i;
    int                                  block;

    //
    // init timing statistics 
    //
    init_stats (roundtrip,    "round_trip");
    init_stats (write_access, "write_access");
    init_stats (read_access,  "read_access");

    //
    // Evaluate cmdline arguments
    //
    if (argc != 1) {
	if (argc != 6) {
            printf ("Invalid.....\n Usage: %s [blocks blocksize topic_id WRITE_PARTITION READ_PARTITION]\n", argv[0]);
            exit (1);
        }
        nof_blocks      = atoi (argv[1]);
        nof_cycles      = atoi (argv[2]);
        topic_id        = argv[3][0];
        write_partition = argv[4];
        read_partition  = argv[5];
    }

    //
    // Create participant
    //
    dpf = DomainParticipantFactory::get_instance ();
    dp = dpf->create_participant (myDomain, PARTICIPANT_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    if (dp == NULL) {
        cout << argv[0] << "PING: ERROR - Splice Daemon not running";
        exit (1);
    }

    // 
    // Create PING publisher
    //
    dp->get_default_publisher_qos (pQos);
    pQos.partition.name.length (1);
    pQos.partition.name[0] = string_dup (write_partition);
    p = dp->create_publisher (pQos, NULL, DDS::STATUS_MASK_NONE);

    //
    // Create PONG subscriber
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
    PP_min_writer = dynamic_cast<PP_min_msgDataWriter_ptr>( dw );

    // Create datareader
    dr = s->create_datareader (PP_min_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_min_reader = dynamic_cast<PP_min_msgDataReader_ptr>(dr);

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
    PP_seq_reader = dynamic_cast<PP_seq_msgDataReader_ptr>(dr);
    
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
    PP_quit_topic = dp->create_topic("PP_quit_topic", "pingpong::PP_quit_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

    // Create datawriter
    dw = p->create_datawriter(PP_quit_topic, DATAWRITER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    PP_quit_writer = dynamic_cast<PP_quit_msgDataWriter_ptr>(dw);

    for (block = 0; block < nof_blocks ; block++) {
        while (!finish_flag) {
            //
            // Send Initial message
            //
            timeout_flag = false;
            
            switch(topic_id) {
                case 'm':
                    {
                        // cout << "PING: sending initial ping_min" << endl;
                        PP_min_msg PPdata;
                        exp_condition = PP_min_sc;
                        active_handler = &PP_min_handler;
                        PPdata.count = 0;
                        PPdata.block = block;
                        preWriteTime = timeGet ();
                        result = PP_min_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 'q':
                    {
                        // cout << "PING: sending initial ping_seq" << endl;
                        PP_seq_msg PPdata;
                        exp_condition = PP_seq_sc;
                        active_handler = &PP_seq_handler;
                        PPdata.count = 0;
                        PPdata.block = block;
PPdata.payload = pingpong::seq_char(SEQ_PAYLOAD_SIZE);
                        preWriteTime = timeGet ();
                        result = PP_seq_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 's':
                    {
                        // cout << "PING: sending initial ping_string" << endl;
                        PP_string_msg PPdata;
                        exp_condition = PP_string_sc;
                        active_handler = &PP_string_handler;
                        PPdata.count = 0;
                        PPdata.block = block;
                        preWriteTime = timeGet ();
                        result = PP_string_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 'f':
                    {
                        // cout << "PING: sending initial ping_fixed" << endl;
                        PP_fixed_msg PPdata;
                        exp_condition = PP_fixed_sc;
                        active_handler = &PP_fixed_handler;
                        PPdata.count = 0;
                        PPdata.block = block;
                        preWriteTime = timeGet ();
                        result = PP_fixed_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 'a':
                    {
                        // cout << "PING: sending initial ping_array" << endl;
                        PP_array_msg PPdata;
                        exp_condition = PP_array_sc;
                        active_handler = &PP_array_handler;
                        PPdata.count = 0;
                        PPdata.block = block;
                        preWriteTime = timeGet ();
                        result = PP_array_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 't':
                    {
                        // cout << "PING: sending initial ping_quit" << endl;
                        PP_quit_msg PPdata;
                        PPdata.quit = true;
			terminate = true;
			finish_flag = true;
                        preWriteTime = timeGet ();
                        result = PP_quit_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                default:
                    printf("Invalid topic-id\n");
                    exit(1);
            }

	    if (!terminate) {
                roundTripTime = preWriteTime;
                add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
            
                //
                // Wait for response, calculate timing, and send another data if not ready
                //
                while (!(timeout_flag || finish_flag)) {
                    result = w.wait (conditionList.inout(), wait_timeout);
                    imax = conditionList->length ();
                    if (imax != 0) {
                        for (i = 0; i < imax; i++) {
                            if ((*conditionList)[i] == exp_condition) {
                                finish_flag = active_handler (nof_cycles);
                            } else {
                                cout << "PING: unexpected condition triggered: "<<  (*conditionList)[i] << endl;
                            }
                        }
                    } else {
                        cout << "PING: TIMEOUT - message lost" << endl;
                        timeout_flag = true;
                    }
                }
	    }
        }
	if (!terminate) {
            finish_flag = false;
	    if (block == 0) {
	        printf ("# PING PONG measurements (in us) \n");
	        printf ("# Executed at: %s", ctime(&clock));
	        printf ("#           Roundtrip time [us]             Write-access time [us]          Read-access time [us]\n");
	        printf ("# Block     Count   mean    min    max      Count   mean    min    max      Count   mean    min    max\n");
	    }
            printf ("%6d %10d %6.0f %6.0f %6.0f %10d %6.0f %6.0f %6.0f %10d %6.0f %6.0f %6.0f\n",
		block,
                roundtrip.count,
                roundtrip.average,
                roundtrip.min,
                roundtrip.max,
                write_access.count,
                write_access.average,
                write_access.min,
                write_access.max,
                read_access.count,
                read_access.average,
                read_access.min,
                read_access.max);
            fflush (NULL);
            init_stats (write_access, "write_access");
            init_stats (read_access,  "read_access");
            init_stats (roundtrip,    "round_trip");
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
    result = p->delete_datawriter (PP_quit_writer);
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
