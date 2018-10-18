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

#include "ccpp_dds_dcps.h"
#include "ccpp_pingpong.h"

#include <iostream>
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#else
#ifndef _WRS_KERNEL
#include <sys/time.h>
#else
#include <version.h>
/* VxWorks versions 6 and newer define _WRS_VXWORKS_MAJOR */
#if ! ( ( _WRS_VXWORKS_MAJOR >= 7 ) || ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR >= 9 ) ) || defined _WRS_KERNEL
#include <sys/times.h>
#endif
#endif
#endif

#include "example_main.h"

#define SEQ_PAYLOAD_SIZE (1000)
#define MAX_NR_OF_TIMEOUTS_BEFORE_QUIT (60)

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

typedef bool pong_handler (unsigned int max_count);

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

static DomainId_t                        myDomain;
static DomainParticipantFactory_var      dpf;
static DomainParticipant_var             dp;
static Publisher_var                     p;
static Subscriber_var                    s;
static DataWriter_ptr                    dw;
static DataReader_ptr                    dr;

static PP_min_msgDataWriter_var          PP_min_writer;
static PP_seq_msgDataWriter_var          PP_seq_writer;
static PP_string_msgDataWriter_var       PP_string_writer;
static PP_fixed_msgDataWriter_var        PP_fixed_writer;
static PP_array_msgDataWriter_var        PP_array_writer;
static PP_bseq_msgDataWriter_var         PP_bseq_writer;
static PP_quit_msgDataWriter_var         PP_quit_writer;

static PP_min_msgDataReader_var          PP_min_reader;
static PP_seq_msgDataReader_var          PP_seq_reader;
static PP_string_msgDataReader_var       PP_string_reader;
static PP_fixed_msgDataReader_var        PP_fixed_reader;
static PP_array_msgDataReader_var        PP_array_reader;
static PP_bseq_msgDataReader_var         PP_bseq_reader;

static PP_min_msgTypeSupport             PP_min_dt;
static PP_seq_msgTypeSupport             PP_seq_dt;
static PP_string_msgTypeSupport          PP_string_dt;
static PP_fixed_msgTypeSupport           PP_fixed_dt;
static PP_array_msgTypeSupport           PP_array_dt;
static PP_bseq_msgTypeSupport            PP_bseq_dt;
static PP_quit_msgTypeSupport            PP_quit_dt;

static PP_min_msgSeq_var                 PP_min_dataList;
static PP_seq_msgSeq_var                 PP_seq_dataList;
static PP_string_msgSeq_var              PP_string_dataList;
static PP_fixed_msgSeq_var               PP_fixed_dataList;
static PP_array_msgSeq_var               PP_array_dataList;
static PP_bseq_msgSeq_var                PP_bseq_dataList;
static PP_quit_msgSeq_var                PP_quit_dataList;

static StatusCondition_var               PP_min_sc;
static StatusCondition_var               PP_seq_sc;
static StatusCondition_var               PP_string_sc;
static StatusCondition_var               PP_fixed_sc;
static StatusCondition_var               PP_array_sc;
static StatusCondition_var               PP_bseq_sc;

static Topic_var                         PP_min_topic;
static Topic_var                         PP_seq_topic;
static Topic_var                         PP_string_topic;
static Topic_var                         PP_fixed_topic;
static Topic_var                         PP_array_topic;
static Topic_var                         PP_bseq_topic;
static Topic_var                         PP_quit_topic;

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

#ifdef _WIN32
static LONGLONG                          clock_frequency;
static void
init_clock(
    void
    )
{
    LARGE_INTEGER frequency;

    QueryPerformanceFrequency(&frequency);
    clock_frequency = frequency.QuadPart;
}
static void sleep(int secs)
{
    Sleep(secs * 1000);
}
#endif

struct timeval
timeGet (
    void
    )
{
    struct timeval current_time;

#ifdef USE_CLOCK_GETTIME
    struct timespec tv;

    clock_gettime (CLOCK_REALTIME, &tv);
    current_time.tv_sec = tv.tv_sec;
    current_time.tv_usec = tv.tv_nsec / 1000;
#elif defined _WIN32
    LARGE_INTEGER timebuffer;

    QueryPerformanceCounter(&timebuffer);
    current_time.tv_sec = (os_int32)(timebuffer.QuadPart / clock_frequency);
    current_time.tv_usec = (os_int32)(((timebuffer.QuadPart % clock_frequency)*1000000)/clock_frequency);
#else

    gettimeofday (&current_time, NULL);
#endif

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

static bool
PP_min_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    bool           result = false;

    // cout << "PING: PING_min arrived" << endl;

    preTakeTime = timeGet ();
    PP_min_reader->take (PP_min_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();

    amount = PP_min_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_min_dataList->length () << " msg received" << endl;
        }
        PP_min_dataList[(DDS::ULong)0U].count++;
        if (PP_min_dataList[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            (void)PP_min_writer->write (PP_min_dataList[0], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats(read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats(roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        PP_min_reader->return_loan (PP_min_dataList, infoList);
    } else {
        cout << "PING: PING_min triggered, but no data available" << endl;
    }
    return result;
}

static bool
PP_seq_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    bool           result = false;

    // cout << "PING: PING_seq arrived" << endl;

    preTakeTime = timeGet ();
    PP_seq_reader->take (PP_seq_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();

    amount = PP_seq_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_seq_dataList->length () << " msg received" << endl;
        }
        PP_seq_dataList[(DDS::ULong)0U].count++;
        if (PP_seq_dataList[(DDS::ULong)0U].count < nof_cycles) {
            preWriteTime = timeGet ();
            PP_seq_writer->write (PP_seq_dataList[(DDS::ULong)0U], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        PP_seq_reader->return_loan (PP_seq_dataList, infoList);
    } else {
        cout << "PING: PING_seq triggered, but no data available" << endl;
    }

    return result;
}

static bool
PP_string_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    bool           result = false;

    // cout << "PING: PING_string arrived" << endl;

    preTakeTime = timeGet ();
    PP_string_reader->take (PP_string_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();

    amount = PP_string_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_string_dataList->length () << " msg received" << endl;
        }
        PP_string_dataList[(DDS::ULong)0U].count++;
        if (PP_string_dataList[(DDS::ULong)0U].count < nof_cycles) {
            preWriteTime = timeGet ();
            PP_string_writer->write (PP_string_dataList[(DDS::ULong)0U], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        PP_string_reader->return_loan (PP_string_dataList, infoList);
    } else {
        cout << "PING: PING_string triggered, but no data available" << endl;
    }
    return result;
}

static bool
PP_fixed_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    bool           result = false;

    // cout << "PING: PING_fixed arrived" << endl;

    preTakeTime = timeGet ();
    PP_fixed_reader->take (PP_fixed_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();

    amount = PP_fixed_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_fixed_dataList->length () << " msg received" << endl;
        }
        PP_fixed_dataList[(DDS::ULong)0U].count++;
        if (PP_fixed_dataList[(DDS::ULong)0U].count < nof_cycles ) {
            preWriteTime = timeGet ();
            PP_fixed_writer->write (PP_fixed_dataList[(DDS::ULong)0U], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        PP_fixed_reader->return_loan (PP_fixed_dataList, infoList);
    } else {
        cout << "PING: PING_fixed triggered, but no data available" << endl;
    }
    return result;
}

static
bool PP_array_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    bool           result = false;

    // cout << "PING: PING_array arrived" << endl;

    preTakeTime = timeGet ();
    PP_array_reader->take (PP_array_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();

    amount = PP_array_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_array_dataList->length () << " msg received" << endl;
        }
        PP_array_dataList[0].count++;
        if (PP_array_dataList[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            PP_array_writer->write (PP_array_dataList[0], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        PP_array_reader->return_loan (PP_array_dataList, infoList);
    } else {
        cout << "PING: PING_array triggered, but no data available" << endl;
    }
    return result;
}

static
bool PP_bseq_handler (
    unsigned int nof_cycles
    )
{
    SampleInfoSeq_var infoList = new SampleInfoSeq;
    int               amount;
    bool           result = false;
    DDS::ULong   zeroIndex = 0;


    // cout << "PING: PING_bseq arrived" << endl;

    preTakeTime = timeGet ();
    PP_bseq_reader->take (PP_bseq_dataList, infoList, LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();

    amount = PP_bseq_dataList->length ();
    if (amount != 0) {
        if (amount > 1) {
            cout << "PING: Ignore excess messages : " << PP_bseq_dataList->length () << " msg received" << endl;
        }
        PP_bseq_dataList[zeroIndex].count++;
        if (PP_bseq_dataList[zeroIndex].count < nof_cycles) {
            preWriteTime = timeGet ();
            PP_bseq_writer->write (PP_bseq_dataList[zeroIndex], HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = true;
        }
        add_stats (read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        PP_bseq_reader->return_loan (PP_bseq_dataList, infoList);
    } else {
        cout << "PING: PING_bseq triggered, but no data available" << endl;
    }
    return result;
}

//
// M A I N
//

#ifdef _WRS_KERNEL /* Entry points for VxWorks kernel build */
int ping_main (int argc, char ** argv);
extern "C" {
   int ping (char * args);
}
int ping (char * args)
{
   int argc=1;
   char *argv[256];
   char *str1;
   argv[0] = (char*) strdup ("ping");
   str1 = (char*) strtok(args, " ");
   while (str1)
   {
      argv[argc] = (char*) strdup (str1);
      argc++;
      str1 = strtok(NULL, " ");
   }
   return ping_main (argc, argv);
}
int ping_main (int argc, char ** argv)
#else
int OSPL_MAIN (int argc, char ** argv)
#endif
{
    ConditionSeq_var                     conditionList = new ConditionSeq();
    WaitSet                              w;
    Condition_ptr                        exp_condition = DDS::Condition::_nil();
    pong_handler                        *active_handler;

    DomainParticipantQos                 dpQos;
    TopicQos                             tQos;
    PublisherQos                         pQos;
    DataWriterQos                        dwQos;
    SubscriberQos                        sQos;
    DataReaderQos                        drQos;

    Duration_t                           wait_timeout = {3,0};

    bool                         finish_flag = false;
    bool                         timeout_flag = false;
    int                          timeout_count = 0;
    bool                         terminate = false;
    DDS::ReturnCode_t                    result;

    int                                  imax = 1;
    int                                  i;
    int                                  block;

    myDomain           = DOMAIN_ID_DEFAULT;
    dp                 = NULL;
    p                  = NULL;
    s                  = NULL;
    dw                 = NULL;
    dr                 = NULL;

    PP_min_writer      = NULL;
    PP_seq_writer      = NULL;
    PP_string_writer   = NULL;
    PP_fixed_writer    = NULL;
    PP_array_writer    = NULL;
    PP_bseq_writer     = NULL;
    PP_quit_writer     = NULL;

    PP_min_reader      = NULL;
    PP_seq_reader      = NULL;
    PP_string_reader   = NULL;
    PP_fixed_reader    = NULL;
    PP_array_reader    = NULL;
    PP_bseq_reader     = NULL;

    PP_min_sc          = NULL;
    PP_seq_sc          = NULL;
    PP_string_sc       = NULL;
    PP_fixed_sc        = NULL;
    PP_array_sc        = NULL;
    PP_bseq_sc         = NULL;

    PP_min_topic       = NULL;
    PP_seq_topic       = NULL;
    PP_string_topic    = NULL;
    PP_fixed_topic     = NULL;
    PP_array_topic     = NULL;
    PP_bseq_topic      = NULL;
    PP_quit_topic      = NULL;

    PP_min_dataList      = new PP_min_msgSeq;
    PP_seq_dataList      = new PP_seq_msgSeq;
    PP_string_dataList   = new PP_string_msgSeq;
    PP_fixed_dataList    = new PP_fixed_msgSeq;
    PP_array_dataList    = new PP_array_msgSeq;
    PP_bseq_dataList     = new PP_bseq_msgSeq;
    PP_quit_dataList     = new PP_quit_msgSeq;

    printf ("Starting ping example\n");
    fflush(stdout);
    //
    // init timing statistics
    //
    init_stats (roundtrip,    "round_trip");
    init_stats (write_access, "write_access");
    init_stats (read_access,  "read_access");

    //
    // Evaluate cmdline arguments
    //
#ifdef INTEGRITY
    if ( argc == 1 )
    {
       nof_blocks = 10;
       nof_cycles = 100;
       write_partition = "PongRead";
       read_partition = "PongWrite";
#if defined (PING1)
       topic_id = 'm';
#elif defined (PING2)
       topic_id = 'q';
#elif defined (PING3)
       topic_id = 's';
#elif defined (PING4)
       topic_id = 'f';
#elif defined (PING5)
       topic_id = 'b';
#elif defined (PING6)
       topic_id = 't';
#endif
    }
#endif
    
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

#ifdef _WIN32
     init_clock();
#endif

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
    pQos.partition.name[0] = DDS::string_dup (write_partition);
    p = dp->create_publisher (pQos, NULL, DDS::STATUS_MASK_NONE);

    //
    // Create PONG subscriber
    //
    dp->get_default_subscriber_qos (sQos);
    sQos.partition.name.length (1);
    sQos.partition.name[0] = DDS::string_dup (read_partition);
    s = dp->create_subscriber (sQos, NULL, DDS::STATUS_MASK_NONE);

    p->get_default_datawriter_qos (dwQos);
    dwQos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
    dwQos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;

    switch (topic_id) {
    case 'm':
        //
        // PP_min_msg
        //

        //  Create Topic
        PP_min_dt.register_type (dp, "pingpong::PP_min_msg");
        PP_min_topic = dp->create_topic ("PP_min_topic", "pingpong::PP_min_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

        // Create datawriter
        dw = p->create_datawriter (PP_min_topic, dwQos, NULL, DDS::STATUS_MASK_NONE);
        PP_min_writer = dynamic_cast<PP_min_msgDataWriter_ptr>( dw );

        // Create datareader
        dr = s->create_datareader (PP_min_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
        PP_min_reader = dynamic_cast<PP_min_msgDataReader_ptr>(dr);

        // Add datareader statuscondition to waitset
        PP_min_sc = PP_min_reader->get_statuscondition ();
        PP_min_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
        w.attach_condition (PP_min_sc);
    break;
    case 'q':
        //
        // PP_seq_msg
        //

        //  Create Topic
        PP_seq_dt.register_type (dp, "pingpong::PP_seq_msg");
        PP_seq_topic = dp->create_topic ("PP_seq_topic", "pingpong::PP_seq_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

        // Create datawriter
        dw = p->create_datawriter (PP_seq_topic, dwQos, NULL, DDS::STATUS_MASK_NONE);
        PP_seq_writer = dynamic_cast<PP_seq_msgDataWriter_ptr> (dw);

        // Create datareader
        dr = s->create_datareader (PP_seq_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
        PP_seq_reader = dynamic_cast<PP_seq_msgDataReader_ptr>(dr);

        // Add datareader statuscondition to waitset
        PP_seq_sc = PP_seq_reader->get_statuscondition ();
        PP_seq_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
        w.attach_condition (PP_seq_sc);
    break;
    case 's':
        //
        // PP_string_msg
        //

        //  Create Topic
        PP_string_dt.register_type (dp, "pingpong::PP_string_msg");
        PP_string_topic = dp->create_topic ("PP_string_topic", "pingpong::PP_string_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

        // Create datawriter
        dw = p->create_datawriter (PP_string_topic, dwQos, NULL, DDS::STATUS_MASK_NONE);
        PP_string_writer = dynamic_cast<PP_string_msgDataWriter_ptr> (dw);

        // Create datareader
        dr = s->create_datareader (PP_string_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
        PP_string_reader = dynamic_cast<PP_string_msgDataReader_ptr> (dr);

        // Add datareader statuscondition to waitset
        PP_string_sc = PP_string_reader->get_statuscondition ();
        PP_string_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
        w.attach_condition (PP_string_sc);
    break;
    case 'f':
        //
        // PP_fixed_msg
        //

        //  Create Topic
        PP_fixed_dt.register_type (dp, "pingpong::PP_fixed_msg");
        PP_fixed_topic = dp->create_topic ("PP_fixed_topic", "pingpong::PP_fixed_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

        // Create datawriter
        dw = p->create_datawriter (PP_fixed_topic, dwQos, NULL, DDS::STATUS_MASK_NONE);
        PP_fixed_writer = dynamic_cast<PP_fixed_msgDataWriter_ptr> (dw);

        // Create datareader
        dr = s->create_datareader (PP_fixed_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
        PP_fixed_reader = dynamic_cast<PP_fixed_msgDataReader_ptr> (dr);

        // Add datareader statuscondition to waitset
        PP_fixed_sc = PP_fixed_reader->get_statuscondition ();
        PP_fixed_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
        w.attach_condition (PP_fixed_sc);
    break;
    case 'a':
        //
        // PP_array_msg
        //

        //  Create Topic
        PP_array_dt.register_type (dp, "pingpong::PP_array_msg");
        PP_array_topic = dp->create_topic ("PP_array_topic", "pingpong::PP_array_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

        // Create datawriter
        dw = p->create_datawriter (PP_array_topic, dwQos, NULL, DDS::STATUS_MASK_NONE);
        PP_array_writer = dynamic_cast<PP_array_msgDataWriter_ptr> (dw);

        // Create datareader
        dr = s->create_datareader (PP_array_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
        PP_array_reader = dynamic_cast<PP_array_msgDataReader_ptr> (dr);

        // Add datareader statuscondition to waitset
        PP_array_sc = PP_array_reader->get_statuscondition ();
        PP_array_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
        w.attach_condition (PP_array_sc);
    break;
    case 'b':
        //
        // PP_bounded_seq_msg
        //

        //  Create Topic
        PP_bseq_dt.register_type (dp, "pingpong::PP_bounded_seq_msg");
        PP_bseq_topic = dp->create_topic ("PP_bseq_topic", "pingpong::PP_bounded_seq_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

        // Create datawriter
        dw = p->create_datawriter (PP_bseq_topic, dwQos, NULL, DDS::STATUS_MASK_NONE);
        PP_bseq_writer = dynamic_cast<PP_bseq_msgDataWriter_ptr> (dw);

        // Create datareader
        dr = s->create_datareader (PP_bseq_topic, DATAREADER_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
        PP_bseq_reader = dynamic_cast<PP_bseq_msgDataReader_ptr> (dr);

        // Add datareader statuscondition to waitset
        PP_bseq_sc = PP_bseq_reader->get_statuscondition ();
        PP_bseq_sc->set_enabled_statuses (DATA_AVAILABLE_STATUS);
        w.attach_condition (PP_bseq_sc);
    break;
    case 't':
        //
        // PP_quit_msg
        //

        //  Create Topic
        PP_quit_dt.register_type (dp, "pingpong::PP_quit_msg");
        PP_quit_topic = dp->create_topic("PP_quit_topic", "pingpong::PP_quit_msg", TOPIC_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);

        // Create datawriter
        dw = p->create_datawriter(PP_quit_topic, dwQos, NULL, DDS::STATUS_MASK_NONE);
        PP_quit_writer = dynamic_cast<PP_quit_msgDataWriter_ptr>(dw);
    break;
    default:
        printf("Invalid topic-id\n");
        exit(1);
    }
    for (block = 0; block < (int)nof_blocks && !terminate; block++) {
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
                        memset (&PPdata, 0, sizeof(PPdata));
                        PPdata.count = 0;
                        PPdata.block = block;
                        preWriteTime = timeGet ();
                        PP_min_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 'q':
                    {
                        // cout << "PING: sending initial ping_seq" << endl;
                        PP_seq_msg PPdata;
                        exp_condition = PP_seq_sc;
                        active_handler = &PP_seq_handler;
                        memset (&PPdata, 0, sizeof(PPdata));
                        PPdata.count = 0;
                        PPdata.block = block;
                        preWriteTime = timeGet ();
                        PP_seq_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 's':
                    {
                        // cout << "PING: sending initial ping_string" << endl;
                        PP_string_msg PPdata;
                        exp_condition = PP_string_sc;
                        active_handler = &PP_string_handler;
                        memset (&PPdata, 0, sizeof(PPdata));
                        PPdata.count = 0;
                        PPdata.block = block;
                        PPdata.a_string = DDS::string_dup("a_string");
                        preWriteTime = timeGet ();
                        PP_string_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 'f':
                    {
                        // cout << "PING: sending initial ping_fixed" << endl;
                        PP_fixed_msg PPdata;
                        exp_condition = PP_fixed_sc;
                        active_handler = &PP_fixed_handler;
                        memset (&PPdata, 0, sizeof(PPdata));
                        PPdata.count = 0;
                        PPdata.block = block;
                        PPdata.a_bstring = DDS::string_dup("a_bstring");
                        preWriteTime = timeGet ();
                        PP_fixed_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 'a':
                    {
                        // cout << "PING: sending initial ping_array" << endl;
                        PP_array_msg PPdata;
                        exp_condition = PP_array_sc;
                        active_handler = &PP_array_handler;
                        memset (&PPdata, 0, sizeof(PPdata));
                        PPdata.count = 0;
                        PPdata.block = block;
                        preWriteTime = timeGet ();
                        PP_array_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 'b':
                    {
                        // cout << "PING: sending initial ping_bseq" << endl;
                        PP_bseq_msg PPdata;
                        exp_condition = PP_bseq_sc;
                        active_handler = &PP_bseq_handler;
                        memset (&PPdata, 0, sizeof(PPdata));
                        PPdata.count = 0;
                        PPdata.block = block;
                        preWriteTime = timeGet ();
                        PP_bseq_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                    }
                    break;
                case 't':
                    {
                        // cout << "PING: sending initial ping_quit" << endl;
                        PP_quit_msg PPdata;
                        memset (&PPdata, 0, sizeof(PPdata));
                        PPdata.quit = true;
                        terminate = true;
                        finish_flag = true;
                        sleep(1);
                        preWriteTime = timeGet ();
                        PP_quit_writer->write (PPdata, HANDLE_NIL);
                        postWriteTime = timeGet ();
                        sleep(1);
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
                    if(result == DDS::RETCODE_OK || result == DDS::RETCODE_NO_DATA || result == DDS::RETCODE_TIMEOUT)
                    {
                        imax = conditionList->length ();
                        if (imax != 0) {
                            timeout_count = 0;
                            for (i = 0; i < imax; i++) {
                                if ((*conditionList)[i] == exp_condition) {
                                    finish_flag = active_handler (nof_cycles);
                                } else {
                                    cout << "PING: unexpected condition triggered: "<<  (*conditionList)[i] << ", terminating." << endl;
                                    finish_flag = true;
                                    terminate = true;
                                }
                            }
                        } else {
                            cout << "PING: TIMEOUT - message lost " << timeout_count << endl;
                            timeout_flag = true;
                            if (timeout_count++ > MAX_NR_OF_TIMEOUTS_BEFORE_QUIT) {
                                cout << "PING: " << timeout_count << " consecutive timeouts, terminating" << endl;
                                finish_flag = true;
                                terminate = true;
                            }
                        }
                    } else
                    {
                        cout << "PING: Waitset wait failed (code "<< result <<"), terminating." << endl;
                        finish_flag = true;
                        terminate = true;
                    }
                }
            }
        }
        if (!terminate) {
            finish_flag = false;
            if (block == 0) {
                printf ("# PING PONG measurements (in us) \n");
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
            fflush (stdout);
            init_stats (write_access, "write_access");
            init_stats (read_access,  "read_access");
            init_stats (roundtrip,    "round_trip");
        }
    }
    switch (topic_id) {
        case 'm':
            w.detach_condition (PP_min_sc);
            s->delete_datareader (PP_min_reader);
            p->delete_datawriter (PP_min_writer);
            dp->delete_topic (PP_min_topic);
            break;
        case 'q':
            w.detach_condition (PP_seq_sc);
            s->delete_datareader (PP_seq_reader);
            p->delete_datawriter (PP_seq_writer);
            dp->delete_topic (PP_seq_topic);
            break;
        case 's':
            w.detach_condition (PP_string_sc);
            s->delete_datareader (PP_string_reader);
            p->delete_datawriter (PP_string_writer);
            dp->delete_topic (PP_string_topic);
            break;
        case 'f':
            w.detach_condition (PP_fixed_sc);
            s->delete_datareader (PP_fixed_reader);
            p->delete_datawriter (PP_fixed_writer);
            dp->delete_topic (PP_fixed_topic);
            break;
        case 'a':
            w.detach_condition (PP_array_sc);
            s->delete_datareader (PP_array_reader);
            p->delete_datawriter (PP_array_writer);
            dp->delete_topic (PP_array_topic);
            break;
        case 'b':
            w.detach_condition (PP_bseq_sc);
            s->delete_datareader (PP_bseq_reader);
            p->delete_datawriter (PP_bseq_writer);
            dp->delete_topic (PP_bseq_topic);
            break;
        case 't':
            p->delete_datawriter (PP_quit_writer);
            dp->delete_topic (PP_quit_topic);
            break;
    }
    dp->delete_subscriber (s);
    dp->delete_publisher (p);
    dpf->delete_participant (dp);
    printf ("Completed ping example\n");
    fflush(stdout);
    return 0;
}
