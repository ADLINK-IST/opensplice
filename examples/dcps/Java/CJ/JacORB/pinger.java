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

import java.text.DateFormat;

import DDS.RETCODE_OK;

class pinger {

    /*
     * configurable parameters ( through cmdline)
     */


    private char topic_id  = 's';

    private String write_partition = "PING";
    private String read_partition  = "PONG";

    /*
     * Global Variables
     */

    private String                             myDomain = null;
    private DDS.DomainParticipantFactory       dpf;
    private DDS.DomainParticipant              dp;
    private DDS.Publisher                      p;
    private DDS.Subscriber                     s;

    private pingpong.PP_min_msgDataWriter      PP_min_writer;
    private pingpong.PP_seq_msgDataWriter      PP_seq_writer;
    private pingpong.PP_string_msgDataWriter   PP_string_writer;
    private pingpong.PP_fixed_msgDataWriter    PP_fixed_writer;
    private pingpong.PP_array_msgDataWriter    PP_array_writer;
    private pingpong.PP_quit_msgDataWriter     PP_quit_writer;

    private pingpong.PP_min_msgDataReader      PP_min_reader;
    private pingpong.PP_seq_msgDataReader      PP_seq_reader;
    private pingpong.PP_string_msgDataReader   PP_string_reader;
    private pingpong.PP_fixed_msgDataReader    PP_fixed_reader;
    private pingpong.PP_array_msgDataReader    PP_array_reader;

    private pingpong.PP_min_msgTypeSupport     PP_min_dt;
    private pingpong.PP_seq_msgTypeSupport     PP_seq_dt;
    private pingpong.PP_string_msgTypeSupport  PP_string_dt;
    private pingpong.PP_fixed_msgTypeSupport   PP_fixed_dt;
    private pingpong.PP_array_msgTypeSupport   PP_array_dt;
    private pingpong.PP_quit_msgTypeSupport    PP_quit_dt;

    private pingpong.PP_min_msgSeqHolder       PP_min_dataList    = new pingpong.PP_min_msgSeqHolder();
    private pingpong.PP_seq_msgSeqHolder       PP_seq_dataList    = new pingpong.PP_seq_msgSeqHolder();
    private pingpong.PP_string_msgSeqHolder    PP_string_dataList = new pingpong.PP_string_msgSeqHolder();
    private pingpong.PP_fixed_msgSeqHolder     PP_fixed_dataList  = new pingpong.PP_fixed_msgSeqHolder();
    private pingpong.PP_array_msgSeqHolder     PP_array_dataList  = new pingpong.PP_array_msgSeqHolder();
    private pingpong.PP_quit_msgSeqHolder      PP_quit_dataList   = new pingpong.PP_quit_msgSeqHolder();

    private DDS.StatusCondition                PP_min_sc;
    private DDS.StatusCondition                PP_seq_sc;
    private DDS.StatusCondition                PP_string_sc;
    private DDS.StatusCondition                PP_fixed_sc;
    private DDS.StatusCondition                PP_array_sc;

    private DDS.Topic                          PP_min_topic;
    private DDS.Topic                          PP_seq_topic;
    private DDS.Topic                          PP_string_topic;
    private DDS.Topic                          PP_fixed_topic;
    private DDS.Topic                          PP_array_topic;
    private DDS.Topic                          PP_quit_topic;

    private stats                              roundtrip     = new stats("roundtrip");
    private stats                              write_access  = new stats("write_access");
    private stats                              read_access   = new stats("read_access");
    private time                               roundTripTime = new time();
    private time                               preWriteTime  = new time();
    private time                               postWriteTime = new time();
    private time                               preTakeTime   = new time();
    private time                               postTakeTime  = new time();

    private static void print_formatted (
	int width,
	int value
	)
    {
	String val = java.lang.Integer.toString (value);
	int i;

	for (i = 0; i < (width - val.length()); i++) {
	    System.out.print (" ");
	}
	System.out.print (val);
    }

    private static void print_formatted (
	int width,
	long value
	)
    {
	String val = java.lang.Long.toString (value);
	int i;

	for (i = 0; i < (width - val.length()); i++) {
	    System.out.print (" ");
	}
	System.out.print (val);
    }

    private boolean
    PP_min_handler (
        int nof_cycles
        )
    {
        DDS.SampleInfoSeqHolder infoList = new DDS.SampleInfoSeqHolder();
        int	                amount;
        boolean                 result = false;
        int                     dds_result;

        /* System.out.println "PING: PING_min arrived"); */

        preTakeTime.timeGet ();
        dds_result = PP_min_reader.take (
            PP_min_dataList,
            infoList,
            DDS.LENGTH_UNLIMITED.value,
            DDS.ANY_SAMPLE_STATE.value,
            DDS.ANY_VIEW_STATE.value,
            DDS.ANY_INSTANCE_STATE.value);

        assert(dds_result == RETCODE_OK.value);

        postTakeTime.timeGet ();

        amount = PP_min_dataList.value.length;
        if (amount != 0) {
            if (amount > 1) {
                System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
            }
            PP_min_dataList.value[0].count++;
            if (PP_min_dataList.value[0].count < nof_cycles) {
                preWriteTime.timeGet ();
                dds_result = PP_min_writer.write (PP_min_dataList.value[0], DDS.HANDLE_NIL.value);
                assert(dds_result == RETCODE_OK.value);
                postWriteTime.timeGet ();
                write_access.add_stats (postWriteTime.sub(preWriteTime));
            } else {
                result = true;
            }
            read_access.add_stats (postTakeTime.sub(preTakeTime));
            roundtrip.add_stats (postTakeTime.sub(roundTripTime));
            roundTripTime.set(preWriteTime.get());
            dds_result = PP_min_reader.return_loan (PP_min_dataList, infoList);
            assert(dds_result == RETCODE_OK.value);
        } else {
            System.out.println ("PING: PING_min triggered, but no data available");
        }
        return result;
    }

    private boolean
    PP_seq_handler (
        int nof_cycles
        )
    {
        DDS.SampleInfoSeqHolder infoList = new DDS.SampleInfoSeqHolder();
        int                     amount;
        boolean                 result = false;
        int                     dds_result;

        /* System.out.println "PING: PING_seq arrived"); */

        preTakeTime.timeGet ();
        dds_result = PP_seq_reader.take (
            PP_seq_dataList,
            infoList,
            DDS.LENGTH_UNLIMITED.value,
            DDS.ANY_SAMPLE_STATE.value,
            DDS.ANY_VIEW_STATE.value,
            DDS.ANY_INSTANCE_STATE.value);
        assert(dds_result == RETCODE_OK.value);
        postTakeTime.timeGet ();

        amount = PP_seq_dataList.value.length;
        if (amount != 0) {
            if (amount > 1) {
                System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
            }
            PP_seq_dataList.value[0].count++;
            if (PP_seq_dataList.value[0].count < nof_cycles) {
                preWriteTime.timeGet ();
                dds_result = PP_seq_writer.write (PP_seq_dataList.value[0], DDS.HANDLE_NIL.value);
                assert(dds_result == RETCODE_OK.value);
                postWriteTime.timeGet ();
                write_access.add_stats (postWriteTime.sub(preWriteTime));
            } else {
                result = true;
            }
            read_access.add_stats (postTakeTime.sub(preTakeTime));
            roundtrip.add_stats (postTakeTime.sub(roundTripTime));
            roundTripTime.set(preWriteTime.get());
            dds_result = PP_seq_reader.return_loan (PP_seq_dataList, infoList);
            assert(dds_result == RETCODE_OK.value);
        } else {
            System.out.println ("PING: PING_seq triggered, but no data available");
        }

        return result;
    }

    private boolean
    PP_string_handler (
        int nof_cycles
        )
    {
        DDS.SampleInfoSeqHolder infoList = new DDS.SampleInfoSeqHolder();
        int                     amount;
        boolean                 result = false;
        int                     dds_result;

        /* System.out.println "PING: PING_string arrived"); */

        preTakeTime.timeGet ();
        dds_result = PP_string_reader.take (
            PP_string_dataList,
            infoList,
            DDS.LENGTH_UNLIMITED.value,
            DDS.ANY_SAMPLE_STATE.value,
            DDS.ANY_VIEW_STATE.value,
            DDS.ANY_INSTANCE_STATE.value);
        assert(dds_result == RETCODE_OK.value);
        postTakeTime.timeGet ();

        amount = PP_string_dataList.value.length;
        if (amount != 0) {
            if (amount > 1) {
                System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
            }
            PP_string_dataList.value[0].count++;
            if (PP_string_dataList.value[0].count < nof_cycles) {
                preWriteTime.timeGet ();
                dds_result = PP_string_writer.write (PP_string_dataList.value[0], DDS.HANDLE_NIL.value);
                assert(dds_result == RETCODE_OK.value);
                postWriteTime.timeGet ();
                write_access.add_stats (postWriteTime.sub(preWriteTime));
            } else {
                result = true;
            }
            read_access.add_stats (postTakeTime.sub(preTakeTime));
            roundtrip.add_stats (postTakeTime.sub(roundTripTime));
            roundTripTime.set(preWriteTime.get());
            dds_result = PP_string_reader.return_loan (PP_string_dataList, infoList);
            assert(dds_result == RETCODE_OK.value);
        } else {
            System.out.println ("PING: PING_string triggered, but no data available");
        }
        return result;
    }

    private boolean
    PP_fixed_handler (
        int nof_cycles
        )
    {
        DDS.SampleInfoSeqHolder infoList = new DDS.SampleInfoSeqHolder();
        int                     amount;
        boolean                 result = false;
        int                     dds_result;

        /* System.out.println "PING: PING_fixed arrived"); */

        preTakeTime.timeGet ();
        dds_result = PP_fixed_reader.take (
            PP_fixed_dataList,
            infoList,
            DDS.LENGTH_UNLIMITED.value,
            DDS.ANY_SAMPLE_STATE.value,
            DDS.ANY_VIEW_STATE.value,
            DDS.ANY_INSTANCE_STATE.value);
        assert(dds_result == RETCODE_OK.value);
        postTakeTime.timeGet ();

        amount = PP_fixed_dataList.value.length;
        if (amount != 0) {
            if (amount > 1) {
                System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
            }
            PP_fixed_dataList.value[0].count++;
            if (PP_fixed_dataList.value[0].count < nof_cycles ) {
                preWriteTime.timeGet ();
                dds_result = PP_fixed_writer.write (PP_fixed_dataList.value[0], DDS.HANDLE_NIL.value);
                assert(dds_result == RETCODE_OK.value);
                postWriteTime.timeGet ();
                write_access.add_stats (postWriteTime.sub(preWriteTime));
            } else {
                result = true;
            }
            read_access.add_stats (postTakeTime.sub(preTakeTime));
            roundtrip.add_stats (postTakeTime.sub(roundTripTime));
            roundTripTime.set(preWriteTime.get());
            dds_result = PP_fixed_reader.return_loan (PP_fixed_dataList, infoList);
            assert(dds_result == RETCODE_OK.value);
        } else {
            System.out.println ("PING: PING_fixed triggered, but no data available");
        }
        return result;
    }

    private boolean
    PP_array_handler (
        int nof_cycles
        )
    {
        DDS.SampleInfoSeqHolder infoList = new DDS.SampleInfoSeqHolder();
        int                     amount;
        boolean                 result = false;
        int                     dds_result;

        /* System.out.println "PING: PING_array arrived"); */

        preTakeTime.timeGet ();
        dds_result = PP_array_reader.take (
            PP_array_dataList,
            infoList,
            DDS.LENGTH_UNLIMITED.value,
            DDS.ANY_SAMPLE_STATE.value,
            DDS.ANY_VIEW_STATE.value,
            DDS.ANY_INSTANCE_STATE.value);
        assert(dds_result == RETCODE_OK.value);
        postTakeTime.timeGet ();

        amount = PP_array_dataList.value.length;
        if (amount != 0) {
            if (amount > 1) {
                System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
            }
            PP_array_dataList.value[0].count++;
            if (PP_array_dataList.value[0].count < nof_cycles) {
                preWriteTime.timeGet ();
                dds_result = PP_array_writer.write (PP_array_dataList.value[0], DDS.HANDLE_NIL.value);
                assert(dds_result == RETCODE_OK.value);
                postWriteTime.timeGet ();
                write_access.add_stats (postWriteTime.sub(preWriteTime));
            } else {
                result = true;
            }
            read_access.add_stats (postTakeTime.sub(preTakeTime));
            roundtrip.add_stats (postTakeTime.sub(roundTripTime));
            roundTripTime.set(preWriteTime.get());
            dds_result = PP_array_reader.return_loan (PP_array_dataList, infoList);
            assert(dds_result == RETCODE_OK.value);
        } else {
            System.out.println ("PING: PING_array triggered, but no data available");
        }
        return result;
    }

    /*
     * P I N G
     */

    public void run (String args[]) {

        DDS.ConditionSeqHolder                    conditionList = new DDS.ConditionSeqHolder();
        DDS.WaitSet                               w;

        DDS.DomainParticipantQosHolder            dpQos;
        DDS.TopicQosHolder                        tQos;
        DDS.PublisherQosHolder                    pQos;
        DDS.DataWriterQosHolder                   dwQos;
        DDS.SubscriberQosHolder                   sQos;
        DDS.DataReaderQosHolder                   drQos;

        DDS.Duration_t                            wait_timeout = new DDS.Duration_t (3,0);

        int                                       result;
        boolean                                   finish_flag = false;
        boolean                                   timeout_flag = false;
        boolean                                   terminate = false;

        int                                       imax = 1;
        int                                       i;
        int                                       block;
        int nof_cycles = 100;
        int nof_blocks = 20;
        /*
         * Evaluate cmdline arguments
         */
        if (args.length != 0) {
	    if (args.length != 5) {
                System.out.println ("Invalid.....");
                System.out.println ("Usage: java ping [blocks blocksize topic_id WRITE_PARTITION READ_PARTITION]");
                return;
            }
            nof_blocks      = java.lang.Integer.parseInt(args[0]);
            nof_cycles      = java.lang.Integer.parseInt(args[1]);
            topic_id        = args[2].charAt (0);
            write_partition = args[3];
            read_partition  = args[4];
        }

        /*
         * Create WaitSet
         */
        w     = new DDS.WaitSet ();
        /*
         * Initialize Qos variables
         */
        dpQos = new DDS.DomainParticipantQosHolder();
        tQos  = new DDS.TopicQosHolder();
        pQos  = new DDS.PublisherQosHolder();
        dwQos = new DDS.DataWriterQosHolder();
        sQos  = new DDS.SubscriberQosHolder();
        drQos = new DDS.DataReaderQosHolder();

        /*
         * Create participant
         */
        dpf = DDS.DomainParticipantFactory.get_instance ();
	dpf.get_default_participant_qos (dpQos);
        dp = dpf.create_participant (myDomain, dpQos.value, null, DDS.STATUS_MASK_NONE.value);
        if (dp == null) {
            System.out.println ("PING: ERROR - Splice Daemon not running");
            return;
        }
	dpQos = null;

        /*
         * Create PING publisher
         */
        dp.get_default_publisher_qos (pQos);
	pQos.value.partition.name = new String [1];
	pQos.value.partition.name[0] = write_partition;
        p = dp.create_publisher (pQos.value, null, DDS.STATUS_MASK_NONE.value);
        pQos = null;

        /*
         * Create PONG subscriber
         */
        dp.get_default_subscriber_qos (sQos);
	sQos.value.partition.name = new String [1];
	sQos.value.partition.name[0] = read_partition;
        s = dp.create_subscriber (sQos.value, null, DDS.STATUS_MASK_NONE.value);
        sQos = null;

	/*
	 * Get default DataReader and DataWriter QoS settings
	 */
	p.get_default_datawriter_qos (dwQos);
	s.get_default_datareader_qos (drQos);

	/*
	 * Get default Topic Qos settings
	 */
	dp.get_default_topic_qos (tQos);

        /*
         * PP_min_msg
         */

        /*  Create Topic */
        PP_min_dt = new pingpong.PP_min_msgTypeSupport();
        PP_min_dt.register_type (dp, "pingpong::PP_min_msg");
        PP_min_topic = dp.create_topic ("PP_min_topic", "pingpong::PP_min_msg", tQos.value, null, DDS.STATUS_MASK_NONE.value);

        /* Create datawriter */
        PP_min_writer = pingpong.PP_min_msgDataWriterHelper.narrow (p.create_datawriter (PP_min_topic, dwQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Create datareader */
        PP_min_reader = pingpong.PP_min_msgDataReaderHelper.narrow (s.create_datareader (PP_min_topic, drQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Add datareader statuscondition to waitset */
        PP_min_sc = PP_min_reader.get_statuscondition ();
	PP_min_sc.set_enabled_statuses (DDS.DATA_AVAILABLE_STATUS.value);
        result = w.attach_condition (PP_min_sc);
        assert(result == RETCODE_OK.value);

        /*
         * PP_seq_msg
         */

        /*  Create Topic */
        PP_seq_dt = new pingpong.PP_seq_msgTypeSupport();
        PP_seq_dt.register_type (dp, "pingpong::PP_seq_msg");
        PP_seq_topic = dp.create_topic ("PP_seq_topic", "pingpong::PP_seq_msg", tQos.value, null, DDS.STATUS_MASK_NONE.value);

        /* Create datawriter */
        PP_seq_writer = pingpong.PP_seq_msgDataWriterHelper.narrow (p.create_datawriter (PP_seq_topic, dwQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Create datareader */
        PP_seq_reader = pingpong.PP_seq_msgDataReaderHelper.narrow (s.create_datareader (PP_seq_topic, drQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Add datareader statuscondition to waitset */
        PP_seq_sc = PP_seq_reader.get_statuscondition ();
	PP_seq_sc.set_enabled_statuses (DDS.DATA_AVAILABLE_STATUS.value);
        result = w.attach_condition (PP_seq_sc);
        assert(result == RETCODE_OK.value);

        /*
         * PP_string_msg
         */

        /*  Create Topic */
        PP_string_dt = new pingpong.PP_string_msgTypeSupport();
        PP_string_dt.register_type (dp, "pingpong::PP_string_msg");
        PP_string_topic = dp.create_topic ("PP_string_topic", "pingpong::PP_string_msg", tQos.value, null, DDS.STATUS_MASK_NONE.value);

        /* Create datawriter */
        PP_string_writer = pingpong.PP_string_msgDataWriterHelper.narrow (p.create_datawriter (PP_string_topic, dwQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Create datareader */
        PP_string_reader = pingpong.PP_string_msgDataReaderHelper.narrow (s.create_datareader (PP_string_topic, drQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Add datareader statuscondition to waitset */
        PP_string_sc = PP_string_reader.get_statuscondition ();
	PP_string_sc.set_enabled_statuses (DDS.DATA_AVAILABLE_STATUS.value);
        result = w.attach_condition (PP_string_sc);
        assert(result == RETCODE_OK.value);

        /*
         * PP_fixed_msg
         */

        /*  Create Topic */
        PP_fixed_dt = new pingpong.PP_fixed_msgTypeSupport();
        PP_fixed_dt.register_type (dp, "pingpong::PP_fixed_msg");
        PP_fixed_topic = dp.create_topic ("PP_fixed_topic", "pingpong::PP_fixed_msg", tQos.value, null, DDS.STATUS_MASK_NONE.value);

        /* Create datawriter */
        PP_fixed_writer = pingpong.PP_fixed_msgDataWriterHelper.narrow (p.create_datawriter (PP_fixed_topic, dwQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Create datareader */
        PP_fixed_reader = pingpong.PP_fixed_msgDataReaderHelper.narrow (s.create_datareader (PP_fixed_topic, drQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Add datareader statuscondition to waitset */
        PP_fixed_sc = PP_fixed_reader.get_statuscondition ();
	PP_fixed_sc.set_enabled_statuses (DDS.DATA_AVAILABLE_STATUS.value);
        result = w.attach_condition (PP_fixed_sc);
        assert(result == RETCODE_OK.value);

        /*
         * PP_array_msg
         */

        /*  Create Topic */
        PP_array_dt = new pingpong.PP_array_msgTypeSupport();
        PP_array_dt.register_type (dp, "pingpong::PP_array_msg");
        PP_array_topic = dp.create_topic ("PP_array_topic", "pingpong::PP_array_msg", tQos.value, null, DDS.STATUS_MASK_NONE.value);

        /* Create datawriter */
        PP_array_writer = pingpong.PP_array_msgDataWriterHelper.narrow (p.create_datawriter (PP_array_topic, dwQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Create datareader */
        PP_array_reader = pingpong.PP_array_msgDataReaderHelper.narrow (s.create_datareader (PP_array_topic, drQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Add datareader statuscondition to waitset */
        PP_array_sc = PP_array_reader.get_statuscondition ();
	PP_array_sc.set_enabled_statuses (DDS.DATA_AVAILABLE_STATUS.value);
        result = w.attach_condition (PP_array_sc);
        assert(result == RETCODE_OK.value);

        /*
         * PP_quit_msg
         */

        /*  Create Topic */
        PP_quit_dt = new pingpong.PP_quit_msgTypeSupport();
        PP_quit_dt.register_type (dp, "pingpong::PP_quit_msg");
        PP_quit_topic = dp.create_topic ("PP_quit_topic", "pingpong::PP_quit_msg", tQos.value, null, DDS.STATUS_MASK_NONE.value);

        /* Create datawriter */
        PP_quit_writer = pingpong.PP_quit_msgDataWriterHelper.narrow (p.create_datawriter (PP_quit_topic, dwQos.value, null, DDS.STATUS_MASK_NONE.value));

        for (block = 0; block < nof_blocks ; block++) {
            while (!finish_flag) {
                /*
                 * Send Initial message
                 */
                timeout_flag = false;

                switch(topic_id) {
                    case 'm':
                        {
                            /* System.out.println ("PING: sending initial ping_min"); */
                            pingpong.PP_min_msg PPdata = new pingpong.PP_min_msg ();
                            PPdata.count = 0;
                            PPdata.block = block;
                            preWriteTime.timeGet ();
                            result = PP_min_writer.write (PPdata, DDS.HANDLE_NIL.value);
                            postWriteTime.timeGet ();
                        }
                        break;
                    case 'q':
                        {
                            /* System.out.println ("PING: sending initial ping_seq"); */
                            pingpong.PP_seq_msg PPdata = new pingpong.PP_seq_msg ();
                            PPdata.count = 0;
                            PPdata.block = block;
                            preWriteTime.timeGet ();
                            result = PP_seq_writer.write (PPdata, DDS.HANDLE_NIL.value);
                            postWriteTime.timeGet ();
                        }
                        break;
                    case 's':
                        {
                            /* System.out.println ("PING: sending initial ping_string"); */
                            pingpong.PP_string_msg PPdata = new pingpong.PP_string_msg ();
                            PPdata.count = 0;
                            PPdata.block = block;
    			    PPdata.a_string = "a_string";
                            preWriteTime.timeGet ();
                            result = PP_string_writer.write (PPdata, DDS.HANDLE_NIL.value);
                            postWriteTime.timeGet ();
                        }
                        break;
                    case 'f':
                        {
                            /* System.out.println ("PING: sending initial ping_fixed"); */
                            pingpong.PP_fixed_msg PPdata = new pingpong.PP_fixed_msg ();
                            PPdata.count = 0;
                            PPdata.block = block;
                            PPdata.a_bstring = "a_bstring";
                            preWriteTime.timeGet ();
                            result = PP_fixed_writer.write (PPdata, DDS.HANDLE_NIL.value);
                            postWriteTime.timeGet ();
                        }
                        break;
                    case 'a':
                        {
                            /* System.out.println ("PING: sending initial ping_array"); */
                            pingpong.PP_array_msg PPdata = new pingpong.PP_array_msg ();
                            PPdata.count = 0;
                            PPdata.block = block;
			    PPdata.str_arr_char = new char [10];
			    PPdata.str_arr_octet = new byte [10];
			    PPdata.str_arr_short = new short [10];
			    PPdata.str_arr_ushort = new short [10];
			    PPdata.str_arr_long = new int [10];
			    PPdata.str_arr_ulong = new int [10];
			    PPdata.str_arr_longlong = new long [10];
			    PPdata.str_arr_ulonglong = new long [10];
			    PPdata.str_arr_float = new float [10];
			    PPdata.str_arr_double = new double [10];
			    PPdata.str_arr_boolean = new boolean [11];
                            preWriteTime.timeGet ();
                            result = PP_array_writer.write (PPdata, DDS.HANDLE_NIL.value);
                            postWriteTime.timeGet ();
                        }
                        break;
                    case 't':
                        {
                            /* System.out.println ("PING: sending initial ping_quit"); */
                            pingpong.PP_quit_msg PPdata = new pingpong.PP_quit_msg ();
                            PPdata.quit = true;
    			    terminate = true;
    			    finish_flag = true;
                            preWriteTime.timeGet ();
                            result = PP_quit_writer.write (PPdata, DDS.HANDLE_NIL.value);
                            postWriteTime.timeGet ();
                        }
                        break;
                    default:
                        System.out.println("Invalid topic-id");
                        return;
                }

    	        if (!terminate) {
                    roundTripTime.set(preWriteTime.get());
                    write_access.add_stats (postWriteTime.sub(preWriteTime));

                    /*
                     * Wait for response, calculate timing, and send another data if not ready
                     */
                    while (!(timeout_flag || finish_flag)) {
                        result = w._wait (conditionList, wait_timeout);
                        if (conditionList.value != null) {
    			    imax = conditionList.value.length;
                            if (imax != 0) {
                                for (i = 0; i < imax; i++) {
                                    if (conditionList.value[i] == PP_min_sc) {
                                        finish_flag = PP_min_handler (nof_cycles);
                                    } else if (conditionList.value[i] == PP_seq_sc) {
                                        finish_flag = PP_seq_handler (nof_cycles);
                                    } else if (conditionList.value[i] == PP_string_sc) {
                                        finish_flag = PP_string_handler (nof_cycles);
                                    } else if (conditionList.value[i] == PP_fixed_sc) {
                                        finish_flag = PP_fixed_handler (nof_cycles);
                                    } else if (conditionList.value[i] == PP_array_sc) {
                                        finish_flag = PP_array_handler (nof_cycles);
                                    } else {
                                        System.out.println ("PING: unexpected condition triggered: " +
    					    conditionList.value[i]);
                                    }
                                }
                            } else {
                                System.out.println ("PING: TIMEOUT - message lost");
                                timeout_flag = true;
                            }
                        } else {
                            System.out.println ("PING: TIMEOUT - message lost");
                            timeout_flag = true;
    		        }
                    }
                }
            }
    	    if (!terminate) {
                finish_flag = false;
    	        if (block == 0) {
    	            System.out.println ("# PING PONG measurements (in us)");
    	            System.out.print ("# Executed at: ");
		    System.out.println (DateFormat.getDateTimeInstance().format(new java.util.Date()));
    	            System.out.println ("#           Roundtrip time [us]             Write-access time [us]          Read-access time [us]");
    	            System.out.println ("# Block     Count   mean    min    max      Count   mean    min    max      Count   mean    min    max");
    	        }

                print_formatted ( 6, block);                 System.out.print (" ");
                print_formatted (10, roundtrip.count);       System.out.print (" ");
                print_formatted ( 6, roundtrip.average);     System.out.print (" ");
                print_formatted ( 6, roundtrip.min);         System.out.print (" ");
                print_formatted ( 6, roundtrip.max);         System.out.print (" ");
                print_formatted (10, write_access.count);    System.out.print (" ");
                print_formatted ( 6, write_access.average);  System.out.print (" ");
                print_formatted ( 6, write_access.min);      System.out.print (" ");
                print_formatted ( 6, write_access.max);      System.out.print (" ");
                print_formatted (10, read_access.count);     System.out.print (" ");
                print_formatted ( 6, read_access.average);   System.out.print (" ");
                print_formatted ( 6, read_access.min);       System.out.print (" ");
                print_formatted ( 6, read_access.max);
		System.out.println ();
                write_access.init_stats ();
                read_access.init_stats ();
                roundtrip.init_stats ();
    	    }
        }
        result = s.delete_datareader (PP_min_reader);
        result = p.delete_datawriter (PP_min_writer);
        result = s.delete_datareader (PP_seq_reader);
        result = p.delete_datawriter (PP_seq_writer);
        result = s.delete_datareader (PP_string_reader);
        result = p.delete_datawriter (PP_string_writer);
        result = s.delete_datareader (PP_fixed_reader);
        result = p.delete_datawriter (PP_fixed_writer);
        result = s.delete_datareader (PP_array_reader);
        result = p.delete_datawriter (PP_array_writer);
        result = p.delete_datawriter (PP_quit_writer);
        result = dp.delete_subscriber (s);
        result = dp.delete_publisher (p);
        result = dp.delete_topic (PP_min_topic);
        result = dp.delete_topic (PP_seq_topic);
        result = dp.delete_topic (PP_string_topic);
        result = dp.delete_topic (PP_fixed_topic);
        result = dp.delete_topic (PP_array_topic);
        result = dp.delete_topic (PP_quit_topic);
        result = dpf.delete_participant (dp);
        w = null;
        PP_min_dt = null;
        PP_seq_dt = null;
        PP_string_dt = null;
        PP_fixed_dt = null;
        PP_array_dt = null;
        PP_quit_dt = null;
        dpQos = null;
        tQos = null;
        dwQos = null;
        drQos = null;

        return;
    }

}
