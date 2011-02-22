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

#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#define SEQ_PAYLOAD_SIZE 100

/*
 * What it does: 
 *   It send a message on the "PING" partition, which the PONG test is waiting for.
 *   The PONG test will send the same message back on the "PONG" partition, which 
 *   the PING test is waiting for. This sequence is repeated a configurable number 
 *   of times.
 *   The PING tests measures:
 *                write_access-time: time the write() method took.
 *                read_access-time:  time the take() method took.
 *                round_trip-time:   time between the call to the write() method 
 *                                   and the return of the take() method.
 *   PING calculates min/max/average statistics on these values over configurable 
 *   data blocks.
 *
 * Configurable:
 *   - blocksize: number of roundtrips in each statistics calculation
 *   - #blocks:   how many times such a statistics calculation is run
 *   - topic:     for the topic, there's a choice between several preconfigured
 *                topics.
 *   - PING and PONG partition: this enables to use several PING-PONG pairs
 *     simultanious with them interfering with each other. It also enables 
 *     creating larger loops, by chaining several PONG tests to one PING test.
 */

/*
 * Type Definitions
 */

typedef struct
{
    char name[20];
    double average;
    double min;
    double max;
    int count;
} stats_type;

typedef DDS_boolean pong_handler (unsigned int max_count);

/*
 * configurable parameters ( through cmdline)
 */

static unsigned int nof_cycles = 100;
static unsigned int nof_blocks = 20;
static char topic_id  = 's';

static char * write_partition = "PING";
static char * read_partition  = "PONG";

/*
 * Global Variables
 */

static DDS_DomainId_t                   	  myDomain           = DDS_OBJECT_NIL;
static DDS_DomainParticipantFactory 		  dpf                = DDS_OBJECT_NIL;
static DDS_DomainParticipant        		  dp                 = DDS_OBJECT_NIL;
static DDS_Publisher                	          p                  = DDS_OBJECT_NIL;
static DDS_Subscriber               	          s                  = DDS_OBJECT_NIL;

static pingpong_PP_min_msgDataWriter              PP_min_writer      = DDS_OBJECT_NIL;
static pingpong_PP_seq_msgDataWriter              PP_seq_writer      = DDS_OBJECT_NIL;
static pingpong_PP_string_msgDataWriter           PP_string_writer   = DDS_OBJECT_NIL;
static pingpong_PP_fixed_msgDataWriter            PP_fixed_writer    = DDS_OBJECT_NIL;
static pingpong_PP_array_msgDataWriter            PP_array_writer    = DDS_OBJECT_NIL;
static pingpong_PP_bseq_msgDataWriter             PP_bseq_writer    = DDS_OBJECT_NIL;
static pingpong_PP_quit_msgDataWriter             PP_quit_writer     = DDS_OBJECT_NIL;

static pingpong_PP_min_msgDataReader              PP_min_reader      = DDS_OBJECT_NIL;
static pingpong_PP_seq_msgDataReader              PP_seq_reader      = DDS_OBJECT_NIL;
static pingpong_PP_string_msgDataReader           PP_string_reader   = DDS_OBJECT_NIL;
static pingpong_PP_fixed_msgDataReader            PP_fixed_reader    = DDS_OBJECT_NIL;
static pingpong_PP_array_msgDataReader            PP_array_reader    = DDS_OBJECT_NIL;
static pingpong_PP_bseq_msgDataReader             PP_bseq_reader    = DDS_OBJECT_NIL;

static pingpong_PP_min_msgTypeSupport             PP_min_dt;
static pingpong_PP_seq_msgTypeSupport             PP_seq_dt;
static pingpong_PP_string_msgTypeSupport          PP_string_dt;
static pingpong_PP_fixed_msgTypeSupport           PP_fixed_dt;
static pingpong_PP_array_msgTypeSupport           PP_array_dt;
static pingpong_PP_bseq_msgTypeSupport            PP_bseq_dt;
static pingpong_PP_quit_msgTypeSupport            PP_quit_dt;

static DDS_sequence_pingpong_PP_min_msg           PP_min_dataList    = { 0, 0, DDS_OBJECT_NIL, FALSE };
static DDS_sequence_pingpong_PP_seq_msg           PP_seq_dataList    = { 0, 0, DDS_OBJECT_NIL, FALSE };
static DDS_sequence_pingpong_PP_string_msg        PP_string_dataList = { 0, 0, DDS_OBJECT_NIL, FALSE };
static DDS_sequence_pingpong_PP_fixed_msg         PP_fixed_dataList  = { 0, 0, DDS_OBJECT_NIL, FALSE };
static DDS_sequence_pingpong_PP_array_msg         PP_array_dataList  = { 0, 0, DDS_OBJECT_NIL, FALSE };
static DDS_sequence_pingpong_PP_bseq_msg   PP_bseq_dataList  = { 0, 0, DDS_OBJECT_NIL, FALSE };

static DDS_StatusCondition                        PP_min_sc          = DDS_OBJECT_NIL;
static DDS_StatusCondition                        PP_seq_sc          = DDS_OBJECT_NIL;
static DDS_StatusCondition                        PP_string_sc       = DDS_OBJECT_NIL;
static DDS_StatusCondition                        PP_fixed_sc        = DDS_OBJECT_NIL;
static DDS_StatusCondition                        PP_array_sc        = DDS_OBJECT_NIL;
static DDS_StatusCondition                        PP_bseq_sc  = DDS_OBJECT_NIL;

static DDS_Topic                                  PP_min_topic       = DDS_OBJECT_NIL;
static DDS_Topic                                  PP_seq_topic       = DDS_OBJECT_NIL;
static DDS_Topic                                  PP_string_topic    = DDS_OBJECT_NIL;
static DDS_Topic                                  PP_fixed_topic     = DDS_OBJECT_NIL;
static DDS_Topic                                  PP_array_topic     = DDS_OBJECT_NIL;
static DDS_Topic                                  PP_bseq_topic = DDS_OBJECT_NIL;
static DDS_Topic                                  PP_quit_topic      = DDS_OBJECT_NIL;

static struct timeval                             roundTripTime;
static struct timeval                             preWriteTime;
static struct timeval                             postWriteTime;
static struct timeval                             preTakeTime;
static struct timeval                             postTakeTime;

static stats_type                                 roundtrip;
static stats_type                                 write_access;
static stats_type                                 read_access;

/*
 * Static functions
 */

static void
add_stats (
    stats_type *stats,
    double data
    )
{
    stats->average = (stats->count * stats->average + data)/(stats->count + 1);
    stats->min     = (stats->count == 0 || data < stats->min) ? data : stats->min;
    stats->max     = (stats->count == 0 || data > stats->max) ? data : stats->max;
    stats->count++;
}

static void
init_stats (
    stats_type *stats,
    char *name)
{
    strncpy ((char *)stats->name, name, 19);
    stats->name[19] = '\0';
    stats->count    = 0;
    stats->average  = 0.0;
    stats->min      = 0.0;
    stats->max      = 0.0; 
}

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

static DDS_boolean
PP_min_handler (
    unsigned int nof_cycles
    )
{
    DDS_SampleInfoSeq     infoList = { 0, 0, NULL, FALSE };
    int	                  amount;
    DDS_boolean           result = FALSE;

    /* printf "PING: PING_min arrived\n"); */
    
    preTakeTime = timeGet ();
    pingpong_PP_min_msgDataReader_take (
	PP_min_reader,
	&PP_min_dataList,
	&infoList,
	DDS_LENGTH_UNLIMITED,
	DDS_ANY_SAMPLE_STATE,
	DDS_ANY_VIEW_STATE,
	DDS_ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_min_dataList._length;
    if (amount != 0) {
        if (amount > 1) {
            printf ("PING: Ignore excess messages : %d msg received\n", amount);
        }
        PP_min_dataList._buffer[0].count++;
        if (PP_min_dataList._buffer[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            pingpong_PP_min_msgDataWriter_write (PP_min_writer, &PP_min_dataList._buffer[0], DDS_HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (&write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = TRUE;
        }
        add_stats(&read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats(&roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        pingpong_PP_min_msgDataReader_return_loan (PP_min_reader, &PP_min_dataList, &infoList);
    } else {
        printf ("PING: PING_min triggered, but no data available\n");
    }
    return result;
}

static DDS_boolean
PP_seq_handler (
    unsigned int nof_cycles
    )
{
    DDS_SampleInfoSeq     infoList = { 0, 0, NULL, FALSE };
    int                   amount;
    DDS_boolean           result = FALSE;

    /* printf "PING: PING_seq arrived\n"); */

    preTakeTime = timeGet ();
    pingpong_PP_seq_msgDataReader_take (
	PP_seq_reader,
	&PP_seq_dataList,
	&infoList,
	DDS_LENGTH_UNLIMITED,
	DDS_ANY_SAMPLE_STATE,
	DDS_ANY_VIEW_STATE,
	DDS_ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();

    amount = PP_seq_dataList._length;
    if (amount != 0) {
        if (amount > 1) {
            printf ("PING: Ignore excess messages : %d msg received\n", amount);
        }
        PP_seq_dataList._buffer[0].count++;
        if (PP_seq_dataList._buffer[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            pingpong_PP_seq_msgDataWriter_write (PP_seq_writer, &PP_seq_dataList._buffer[0], DDS_HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (&write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = TRUE;
        }
        add_stats (&read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (&roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        pingpong_PP_seq_msgDataReader_return_loan (PP_seq_reader, &PP_seq_dataList, &infoList);
    } else {
        printf ("PING: PING_seq triggered, but no data available\n");
    }

    return result;
}

static DDS_boolean
PP_string_handler (
    unsigned int nof_cycles
    )
{
    DDS_SampleInfoSeq     infoList = { 0, 0, NULL, FALSE };
    int                   amount;
    DDS_boolean           result = FALSE;
    
    /* printf "PING: PING_string arrived\n"); */
    
    preTakeTime = timeGet ();
    pingpong_PP_string_msgDataReader_take (
	PP_string_reader,
	&PP_string_dataList,
	&infoList,
	DDS_LENGTH_UNLIMITED,
	DDS_ANY_SAMPLE_STATE,
	DDS_ANY_VIEW_STATE,
	DDS_ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_string_dataList._length;
    if (amount != 0) {
        if (amount > 1) {
            printf ("PING: Ignore excess messages : %d msg received\n", amount);
        }
        PP_string_dataList._buffer[0].count++;
        if (PP_string_dataList._buffer[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            pingpong_PP_string_msgDataWriter_write (PP_string_writer, &PP_string_dataList._buffer[0], DDS_HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (&write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = TRUE;
        }
        add_stats (&read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (&roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        pingpong_PP_string_msgDataReader_return_loan (PP_string_reader, &PP_string_dataList, &infoList);
    } else {
        printf ("PING: PING_string triggered, but no data available\n");
    }
    return result;
}

static DDS_boolean
PP_fixed_handler (
    unsigned int nof_cycles
    )
{
    DDS_SampleInfoSeq     infoList = { 0, 0, NULL, FALSE };
    int                   amount;
    DDS_boolean           result = FALSE;
    
    /* printf "PING: PING_fixed arrived\n"); */
    
    preTakeTime = timeGet ();
    pingpong_PP_fixed_msgDataReader_take (
	PP_fixed_reader,
	&PP_fixed_dataList,
	&infoList,
	DDS_LENGTH_UNLIMITED,
	DDS_ANY_SAMPLE_STATE,
	DDS_ANY_VIEW_STATE,
	DDS_ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_fixed_dataList._length;
    if (amount != 0) {
        if (amount > 1) {
            printf ("PING: Ignore excess messages : %d msg received\n", amount);
        }
        PP_fixed_dataList._buffer[0].count++;
        if (PP_fixed_dataList._buffer[0].count < nof_cycles ) {
            preWriteTime = timeGet ();
            pingpong_PP_fixed_msgDataWriter_write (PP_fixed_writer, &PP_fixed_dataList._buffer[0], DDS_HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (&write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = TRUE;
        }
        add_stats (&read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (&roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        pingpong_PP_fixed_msgDataReader_return_loan (PP_fixed_reader, &PP_fixed_dataList, &infoList);
    } else {
        printf ("PING: PING_fixed triggered, but no data available\n");
    }
    return result;
}

static DDS_boolean
PP_array_handler (
    unsigned int nof_cycles
    )
{
    DDS_SampleInfoSeq     infoList = { 0, 0, NULL, FALSE };
    int                   amount;
    DDS_boolean           result = FALSE;
    
    /* printf "PING: PING_array arrived\n"); */
    
    preTakeTime = timeGet ();
    pingpong_PP_array_msgDataReader_take (
	PP_array_reader,
	&PP_array_dataList,
	&infoList,
	DDS_LENGTH_UNLIMITED,
	DDS_ANY_SAMPLE_STATE,
	DDS_ANY_VIEW_STATE,
	DDS_ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_array_dataList._length;
    if (amount != 0) {
        if (amount > 1) {
            printf ("PING: Ignore excess messages : %d msg received\n", amount);
        }
        PP_array_dataList._buffer[0].count++;
        if (PP_array_dataList._buffer[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            pingpong_PP_array_msgDataWriter_write (PP_array_writer, &PP_array_dataList._buffer[0], DDS_HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (&write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = TRUE;
        }
        add_stats (&read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (&roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        pingpong_PP_array_msgDataReader_return_loan (PP_array_reader, &PP_array_dataList, &infoList);
    } else {
        printf ("PING: PING_array triggered, but no data available\n");
    }
    return result;
}

static DDS_boolean
PP_bseq_handler (
    unsigned int nof_cycles
    )
{
    DDS_SampleInfoSeq     infoList = { 0, 0, NULL, FALSE };
    int                   amount;
    DDS_boolean           result = FALSE;
    
    /* printf "PING: PING_bseq arrived\n"); */
    
    preTakeTime = timeGet ();
    pingpong_PP_bseq_msgDataReader_take (
	PP_bseq_reader,
	&PP_bseq_dataList,
	&infoList,
	DDS_LENGTH_UNLIMITED,
	DDS_ANY_SAMPLE_STATE,
	DDS_ANY_VIEW_STATE,
	DDS_ANY_INSTANCE_STATE);
    postTakeTime = timeGet ();
    
    amount = PP_bseq_dataList._length;
    if (amount != 0) {
        if (amount > 1) {
            printf ("PING: Ignore excess messages : %d msg received\n", amount);
        }
        PP_bseq_dataList._buffer[0].count++;
        if (PP_bseq_dataList._buffer[0].count < nof_cycles) {
            preWriteTime = timeGet ();
            pingpong_PP_bseq_msgDataWriter_write (PP_bseq_writer, &PP_bseq_dataList._buffer[0], DDS_HANDLE_NIL);
            postWriteTime = timeGet ();
            add_stats (&write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
        } else {
            result = TRUE;
        }
        add_stats (&read_access, 1E6 * timeToReal (timeSub (postTakeTime, preTakeTime)));
        add_stats (&roundtrip,   1E6 * timeToReal (timeSub (postTakeTime, roundTripTime)));
        roundTripTime = preWriteTime;
        pingpong_PP_bseq_msgDataReader_return_loan (PP_bseq_reader, &PP_bseq_dataList, &infoList);
    } else {
        printf ("PING: PING_bseq triggered, but no data available\n");
    }
    return result;
}

/*
 * M A I N
 */
#ifdef _VXWORKS
int ping_main (int argc, char ** argv);
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
int main (int argc, char ** argv)
#endif
{
    DDS_ConditionSeq                        *conditionList;
    DDS_WaitSet                              w;
    DDS_Condition                            exp_condition;
    pong_handler                            *active_handler;

    DDS_DomainParticipantQos                 *dpQos;
    DDS_TopicQos                             *tQos;
    DDS_PublisherQos                         *pQos;
    DDS_DataWriterQos                        *dwQos;
    DDS_SubscriberQos                        *sQos;
    DDS_DataReaderQos                        *drQos;
    
    time_t                                   clock = time (NULL);
    DDS_Duration_t                           wait_timeout = {3,0};

    DDS_boolean                              finish_flag = FALSE;
    DDS_boolean                              timeout_flag = FALSE;
    DDS_boolean                              terminate = FALSE;

    int                                      imax = 1;
    int                                      i;
    unsigned int                             block;

    printf ("Starting ping example\n");
    fflush(stdout);

    /*
     * init timing statistics 
     */
    init_stats (&roundtrip,    "round_trip");
    init_stats (&write_access, "write_access");
    init_stats (&read_access,  "read_access");

    /*
     * Evaluate cmdline arguments
     */
#ifdef INTEGRITY
    nof_blocks = 100;
    nof_cycles = 100;
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
    nof_blocks = 1;
    nof_cycles = 10;
    topic_id = 't';
#endif 
    write_partition = "PongRead";
    read_partition = "PongWrite";
#else
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
#endif

    /*
     * Create WaitSet
     */
    w     = DDS_WaitSet__alloc ();
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
    if (dp == DDS_HANDLE_NIL) {
        printf ("%s PING: ERROR - Splice Daemon not running", argv[0]);
        exit (1);
    }

    /* 
     * Create PING publisher
     */
    DDS_DomainParticipant_get_default_publisher_qos (dp, pQos);
    pQos->partition.name._length = 1;
    pQos->partition.name._maximum = 1;
    pQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    pQos->partition.name._buffer[0] = DDS_string_alloc (strlen(write_partition) + 1);
    strcpy (pQos->partition.name._buffer[0], write_partition);
    p = DDS_DomainParticipant_create_publisher (dp, pQos, NULL, DDS_STATUS_MASK_NONE);
    DDS_free (pQos);

    /*
     * Create PONG subscriber
     */
    DDS_DomainParticipant_get_default_subscriber_qos (dp, sQos);
    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_alloc (strlen(read_partition) + 1);
    strcpy (sQos->partition.name._buffer[0], read_partition);
    s = DDS_DomainParticipant_create_subscriber (dp, sQos, NULL, DDS_STATUS_MASK_NONE);
    DDS_free (sQos);

    /*
     * PP_min_msg
     */

    /*  Create Topic */
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
    DDS_WaitSet_attach_condition (w, PP_min_sc);

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
    DDS_WaitSet_attach_condition (w, PP_seq_sc);
    
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
    DDS_WaitSet_attach_condition (w, PP_string_sc);
    
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
    DDS_WaitSet_attach_condition (w, PP_fixed_sc);
    
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
    DDS_WaitSet_attach_condition (w, PP_array_sc);


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
    DDS_WaitSet_attach_condition (w, PP_bseq_sc);

    /*
     * PP_quit_msg
     */
    
    /*  Create Topic */
    PP_quit_dt = pingpong_PP_quit_msgTypeSupport__alloc ();
    pingpong_PP_quit_msgTypeSupport_register_type (PP_quit_dt, dp, "pingpong::PP_quit_msg");
    PP_quit_topic = DDS_DomainParticipant_create_topic (dp, "PP_quit_topic", "pingpong::PP_quit_msg", DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Create datawriter */
    PP_quit_writer = DDS_Publisher_create_datawriter (p, PP_quit_topic, DDS_DATAWRITER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

    /* Fr: workarround for ticket dds1712 */
    conditionList = DDS_ConditionSeq__alloc();
    assert(conditionList);
    DDS_WaitSet_wait (w, conditionList, &wait_timeout);
    DDS_free(conditionList);

    for (block = 0; block < nof_blocks ; block++) {
        while (!finish_flag) {
            /*
             * Send Initial message
             */
            timeout_flag = FALSE;
            
            switch(topic_id) {
                case 'm':
                    {
                        /* printf ("PING: sending initial ping_min\n"); */
                        pingpong_PP_min_msg *PPdata = pingpong_PP_min_msg__alloc ();
                        exp_condition = PP_min_sc;
                        active_handler = &PP_min_handler;
                        PPdata->count = 0;
                        PPdata->block = block;
                        preWriteTime = timeGet ();
                        pingpong_PP_min_msgDataWriter_write (PP_min_writer, PPdata, DDS_HANDLE_NIL);
                        postWriteTime = timeGet ();
			DDS_free (PPdata);
                    }
                    break;
                case 'q':
                    {
                        /* printf ("PING: sending initial ping_seq\n"); */
                        pingpong_PP_seq_msg *PPdata = pingpong_PP_seq_msg__alloc ();
                        exp_condition = PP_seq_sc;
                        active_handler = &PP_seq_handler;
                        PPdata->count = 0;
                        PPdata->block = block;
                        {
                            int i = 0;
                            PPdata->payload._buffer = pingpong_seq_char_allocbuf(SEQ_PAYLOAD_SIZE);
                            PPdata->payload._length = SEQ_PAYLOAD_SIZE;
                            PPdata->payload._maximum = SEQ_PAYLOAD_SIZE;
                            for (i=0; i<SEQ_PAYLOAD_SIZE; i++) {
                                PPdata->payload._buffer[i] = (char)i;
                            }
                        }
                        preWriteTime = timeGet ();
                        pingpong_PP_seq_msgDataWriter_write (PP_seq_writer, PPdata, DDS_HANDLE_NIL);
                        postWriteTime = timeGet ();
			DDS_free (PPdata);
                    }
                    break;
                case 's':
                    {
                        /* printf ("PING: sending initial ping_string\n"); */
                        pingpong_PP_string_msg *PPdata = pingpong_PP_string_msg__alloc ();
                        exp_condition = PP_string_sc;
                        active_handler = &PP_string_handler;
                        PPdata->count = 0;
                        PPdata->block = block;
            			PPdata->a_string = DDS_string_alloc (8);
            			strcpy (PPdata->a_string, "a_string");
                        preWriteTime = timeGet ();
                        pingpong_PP_string_msgDataWriter_write (PP_string_writer, PPdata, DDS_HANDLE_NIL);
                        postWriteTime = timeGet ();
                        DDS_free (PPdata);
                    }
                    break;
                case 'f':
                    {
                        /* printf ("PING: sending initial ping_fixed\n"); */
                        pingpong_PP_fixed_msg *PPdata = pingpong_PP_fixed_msg__alloc ();
                        exp_condition = PP_fixed_sc;
                        active_handler = &PP_fixed_handler;
                        PPdata->count = 0;
                        PPdata->block = block;
                        PPdata->a_bstring = DDS_string_alloc (9);
                        strcpy (PPdata->a_bstring, "a_bstring");
                        preWriteTime = timeGet ();
                        pingpong_PP_fixed_msgDataWriter_write (PP_fixed_writer, PPdata, DDS_HANDLE_NIL);
                        postWriteTime = timeGet ();
                        DDS_free (PPdata);
                    }
                    break;
                case 'a':
                    {
                        /* printf ("PING: sending initial ping_array\n"); */
                        pingpong_PP_array_msg *PPdata = pingpong_PP_array_msg__alloc ();
                        exp_condition = PP_array_sc;
                        active_handler = &PP_array_handler;
                        PPdata->count = 0;
                        PPdata->block = block;
                        preWriteTime = timeGet ();
                        pingpong_PP_array_msgDataWriter_write (PP_array_writer, PPdata, DDS_HANDLE_NIL);
                        postWriteTime = timeGet ();
			DDS_free (PPdata);
                    }
                    break;
                case 'b':
                    {
                        /* printf ("PING: sending initial ping_bseq_msg\n"); */
                        pingpong_PP_bseq_msg *PPdata = pingpong_PP_bseq_msg__alloc ();
                        exp_condition = PP_bseq_sc;
                        active_handler = &PP_bseq_handler;
                        PPdata->count = 0;
                        PPdata->block = block;
                        preWriteTime = timeGet ();
                        pingpong_PP_bseq_msgDataWriter_write (PP_bseq_writer, PPdata, DDS_HANDLE_NIL);
                        postWriteTime = timeGet ();
			DDS_free (PPdata);
                    }
                    break;
                case 't':
                    {
                        /* printf ("PING: sending initial ping_quit\n"); */
                        pingpong_PP_quit_msg *PPdata = pingpong_PP_quit_msg__alloc();
                        PPdata->quit = TRUE;
			terminate = TRUE;
			finish_flag = TRUE;
                        preWriteTime = timeGet ();
                        pingpong_PP_quit_msgDataWriter_write (PP_quit_writer, PPdata, DDS_HANDLE_NIL);
                        postWriteTime = timeGet ();
			DDS_free (PPdata);
                    }
                    break;
                default:
                    printf("Invalid topic-id\n");
                    exit(1);
            }

	    if (!terminate) {
                roundTripTime = preWriteTime;
                add_stats (&write_access, 1E6 * timeToReal (timeSub (postWriteTime, preWriteTime)));
            
                /*
                 * Wait for response, calculate timing, and send another data if not ready
                 */
                while (!(timeout_flag || finish_flag)) {
                    conditionList = DDS_ConditionSeq__alloc();
                    DDS_WaitSet_wait (w, conditionList, &wait_timeout);
                    if (conditionList) {
			imax = conditionList->_length;
                        if (imax != 0) {
                            for (i = 0; i < imax; i++) {
                                if (conditionList->_buffer[i] == exp_condition) {
                                    finish_flag = active_handler (nof_cycles);
                                } else {
                                    printf ("PING: unexpected condition triggered: %lx\n",
					(unsigned long)conditionList->_buffer[i]);
                                }
                            }
                        } else {
                            printf ("PING: TIMEOUT - message lost\n");
                            timeout_flag = TRUE;
			}
                        DDS_free(conditionList);
                    } else {
                        printf ("PING: TIMEOUT - message lost\n");
                        timeout_flag = TRUE;
		    }		
                }
	    }
        }
	if (!terminate) {
            finish_flag = FALSE;
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
            init_stats (&write_access, "write_access");
            init_stats (&read_access,  "read_access");
            init_stats (&roundtrip,    "round_trip");
	}
    }
    DDS_Subscriber_delete_datareader (s, PP_min_reader);
    DDS_Publisher_delete_datawriter (p, PP_min_writer);
    DDS_Subscriber_delete_datareader (s, PP_seq_reader);
    DDS_Publisher_delete_datawriter (p, PP_seq_writer);
    DDS_Subscriber_delete_datareader (s, PP_string_reader);
    DDS_Publisher_delete_datawriter (p, PP_string_writer);
    DDS_Subscriber_delete_datareader (s, PP_fixed_reader);
    DDS_Publisher_delete_datawriter (p, PP_fixed_writer);
    DDS_Subscriber_delete_datareader (s, PP_array_reader);
    DDS_Publisher_delete_datawriter (p, PP_array_writer);
    DDS_Subscriber_delete_datareader (s, PP_bseq_reader);
    DDS_Publisher_delete_datawriter (p, PP_bseq_writer);
    DDS_Publisher_delete_datawriter (p, PP_quit_writer);
    DDS_DomainParticipant_delete_subscriber (dp, s);
    DDS_DomainParticipant_delete_publisher (dp, p);
    DDS_DomainParticipant_delete_topic (dp, PP_min_topic);
    DDS_DomainParticipant_delete_topic (dp, PP_seq_topic);
    DDS_DomainParticipant_delete_topic (dp, PP_string_topic);
    DDS_DomainParticipant_delete_topic (dp, PP_fixed_topic);
    DDS_DomainParticipant_delete_topic (dp, PP_array_topic);
    DDS_DomainParticipant_delete_topic (dp, PP_bseq_topic);
    DDS_DomainParticipant_delete_topic (dp, PP_quit_topic);
    DDS_DomainParticipantFactory_delete_participant (dpf, dp);
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

    printf ("Completed ping example\n");
    fflush(stdout);
    return 0;
}
