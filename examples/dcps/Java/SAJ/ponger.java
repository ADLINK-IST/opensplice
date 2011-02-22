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
import DDS.*;

public class ponger {

    /*
     * Configurable parameters (through cmdline)
     * These are the default settings
     */
    private String write_partition = "PING";
    private String read_partition  = "PONG";

    public void run (String args[]) {

        String                                     myDomain = null;
        DDS.DomainParticipantFactory               dpf;
        DDS.DomainParticipant                      dp;
        DDS.Publisher                              p;
        DDS.Subscriber                             s;
    
        pingpong.PP_min_msgDataWriter              PP_min_writer;
        pingpong.PP_seq_msgDataWriter              PP_seq_writer;
        pingpong.PP_string_msgDataWriter           PP_string_writer;
        pingpong.PP_fixed_msgDataWriter            PP_fixed_writer;
        pingpong.PP_array_msgDataWriter            PP_array_writer;
    
        pingpong.PP_min_msgDataReader              PP_min_reader;
        pingpong.PP_seq_msgDataReader              PP_seq_reader;
        pingpong.PP_string_msgDataReader           PP_string_reader;
        pingpong.PP_fixed_msgDataReader            PP_fixed_reader;
        pingpong.PP_array_msgDataReader            PP_array_reader;
        pingpong.PP_quit_msgDataReader             PP_quit_reader;
    
        pingpong.PP_min_msgTypeSupport             PP_min_dt;
        pingpong.PP_seq_msgTypeSupport             PP_seq_dt;
        pingpong.PP_string_msgTypeSupport          PP_string_dt;
        pingpong.PP_fixed_msgTypeSupport           PP_fixed_dt;
        pingpong.PP_array_msgTypeSupport           PP_array_dt;
        pingpong.PP_quit_msgTypeSupport            PP_quit_dt;
    
        pingpong.PP_min_msgSeqHolder               PP_min_dataList = new pingpong.PP_min_msgSeqHolder();
        pingpong.PP_seq_msgSeqHolder               PP_seq_dataList = new pingpong.PP_seq_msgSeqHolder();
        pingpong.PP_string_msgSeqHolder            PP_string_dataList = new pingpong.PP_string_msgSeqHolder();
        pingpong.PP_fixed_msgSeqHolder             PP_fixed_dataList = new pingpong.PP_fixed_msgSeqHolder();
        pingpong.PP_array_msgSeqHolder             PP_array_dataList = new pingpong.PP_array_msgSeqHolder();
        pingpong.PP_quit_msgSeqHolder              PP_quit_dataList = new pingpong.PP_quit_msgSeqHolder();
    
        DDS.StatusCondition                        PP_min_sc;
        DDS.StatusCondition                        PP_seq_sc;
        DDS.StatusCondition                        PP_string_sc;
        DDS.StatusCondition                        PP_fixed_sc;
        DDS.StatusCondition                        PP_array_sc;
        DDS.StatusCondition                        PP_quit_sc;
    
        DDS.Topic                                  PP_min_topic;
        DDS.Topic                                  PP_seq_topic;
        DDS.Topic                                  PP_string_topic;
        DDS.Topic                                  PP_fixed_topic;
        DDS.Topic                                  PP_array_topic;
        DDS.Topic                                  PP_quit_topic;
    
        DDS.ConditionSeqHolder                     conditionList = new DDS.ConditionSeqHolder();
        DDS.SampleInfoSeqHolder                    infoList = new DDS.SampleInfoSeqHolder();
        DDS.WaitSet                                w;
    
        DDS.DomainParticipantQosHolder             dpQos;
        DDS.TopicQosHolder                         tQos;
        DDS.PublisherQosHolder                     pQos;
        DDS.DataWriterQosHolder                    dwQos;
        DDS.SubscriberQosHolder                    sQos;
        DDS.DataReaderQosHolder                    drQos;
    
        boolean                                    terminate = false;
    
        int                                        result;
        int                                        i;
        int                                        imax;
        int                                        j;
        int                                        jmax;
    
        /*
         * Evaluate cmdline arguments
         */
    
        if (args.length != 0) {
            if (args.length != 2) {
                System.out.println ("Invalid.....");
		System.out.println ("Usage: java pong [READ_PARTITION WRITE_PARTITION]");
                return;
            }
            read_partition  = args[0];
            write_partition = args[1];
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
         * Create PONG publisher
         */
        dp.get_default_publisher_qos (pQos);
        pQos.value.partition.name = new String [1];
        pQos.value.partition.name[0] = write_partition;
        p = dp.create_publisher (pQos.value, null, DDS.STATUS_MASK_NONE.value);
        pQos = null;

        /*
         * Create PING subscriber
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
        
        /* Create Topic */
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

        /* Create datareader */
        PP_quit_reader = pingpong.PP_quit_msgDataReaderHelper.narrow (s.create_datareader (PP_quit_topic, drQos.value, null, DDS.STATUS_MASK_NONE.value));

        /* Add datareader statuscondition to waitset */
        PP_quit_sc = PP_quit_reader.get_statuscondition ();
        PP_quit_sc.set_enabled_statuses (DDS.DATA_AVAILABLE_STATUS.value);
        result = w.attach_condition (PP_quit_sc);
        assert(result == RETCODE_OK.value);
    
        while (!terminate) {
    	    DDS.Duration_t wait_timeout = new DDS.Duration_t (DDS.DURATION_INFINITE_SEC.value, DDS.DURATION_INFINITE_NSEC.value);
    
            /* System.out.println ("PONG: waiting for PING"); */
            result = w._wait (conditionList, wait_timeout);
            if (result == RETCODE_ALREADY_DELETED.value) {
                terminate = true;
                continue;
            }
            if (conditionList.value != null) {
    	        imax = conditionList.value.length;
                for (i = 0; i < imax; i++) {
                    if (conditionList.value[i] == PP_min_sc) {
        		/* System.out.println ("PONG: PING_min arrived"); */
                        result = PP_min_reader.take (PP_min_dataList, infoList, DDS.LENGTH_UNLIMITED.value,
                                     DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
                        jmax = PP_min_dataList.value.length;
                        if (jmax != 0) {
                            for (j = 0; j < jmax; j++) {
                                if(infoList.value[j].valid_data) {
                                    result = PP_min_writer.write (PP_min_dataList.value[j], DDS.HANDLE_NIL.value);
                                }
                            }
                            result = PP_min_reader.return_loan (PP_min_dataList, infoList);
                        } else {
                            System.out.println ("PONG: PING_min triggered, but no data available");
                        }
                    } else if (conditionList.value[i] == PP_seq_sc) {
        		/* System.out.println ("PONG: PING_seq arrived"); */
                        result = PP_seq_reader.take (PP_seq_dataList, infoList, DDS.LENGTH_UNLIMITED.value,
                                     DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
                        jmax = PP_seq_dataList.value.length;
                        if (jmax != 0) {
                            for (j = 0; j < jmax; j++) {
                                if(infoList.value[j].valid_data) {
                                    result = PP_seq_writer.write (PP_seq_dataList.value[j], DDS.HANDLE_NIL.value);
                                }
                            }
                            result = PP_seq_reader.return_loan (PP_seq_dataList, infoList);
                        } else {
                            System.out.println ("PONG: PING_seq triggered, but no data available");
                        }
                    } else if (conditionList.value[i] == PP_string_sc) {
        		/* System.out.println ("PONG: PING_string arrived"); */
                        result = PP_string_reader.take (PP_string_dataList, infoList, DDS.LENGTH_UNLIMITED.value,
                                     DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
                        jmax = PP_string_dataList.value.length;
                        if (jmax != 0) {
                            for (j = 0; j < jmax; j++) {
                                if(infoList.value[j].valid_data) {
                                    result = PP_string_writer.write (PP_string_dataList.value[j], DDS.HANDLE_NIL.value);
                                }
                            }
                            result = PP_string_reader.return_loan (PP_string_dataList, infoList);
                        } else {
                            System.out.println ("PONG: PING_string triggered, but no data available");
                        }
                    } else if (conditionList.value[i] == PP_fixed_sc) {
        		/* System.out.println ("PONG: PING_fixed arrived"); */
                        result = PP_fixed_reader.take (PP_fixed_dataList, infoList, DDS.LENGTH_UNLIMITED.value,
                                     DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
                        jmax = PP_fixed_dataList.value.length;
                        if (jmax != 0) {
                            for (j = 0; j < jmax; j++) {
                                if(infoList.value[j].valid_data) {
                                    result = PP_fixed_writer.write (PP_fixed_dataList.value[j], DDS.HANDLE_NIL.value);
                                }
                            }
                            result = PP_fixed_reader.return_loan (PP_fixed_dataList, infoList);
                        } else {
                            System.out.println ("PONG: PING_fixed triggered, but no data available");
                        }
                    } else if (conditionList.value[i] == PP_array_sc) {
        		/* System.out.println ("PONG: PING_array arrived"); */
                        result = PP_array_reader.take (PP_array_dataList, infoList, DDS.LENGTH_UNLIMITED.value,
                                     DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
                        jmax = PP_array_dataList.value.length;
                        if (jmax != 0) {
                            for (j = 0; j < jmax; j++) {
                                if(infoList.value[j].valid_data) {
                                    result = PP_array_writer.write (PP_array_dataList.value[j], DDS.HANDLE_NIL.value);
                                }
                            }
                            result = PP_array_reader.return_loan (PP_array_dataList, infoList);
                        } else {
                            System.out.println ("PONG: PING_array triggered, but no data available");
                        }
                    } else if (conditionList.value[i] == PP_quit_sc) {
        		/* System.out.println ("PONG: PING_quit arrived"); */
                        result = PP_quit_reader.take (PP_quit_dataList, infoList, DDS.LENGTH_UNLIMITED.value,
                                     DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
                        jmax = PP_quit_dataList.value.length;
                        if (jmax != 0) {
                            result = PP_quit_reader.return_loan (PP_quit_dataList, infoList);
                        } else {
                            System.out.println ("PONG: PING_quit triggered, but no data available");
                        }
                        terminate = true;
                    } else {
                        System.out.println ("PONG: unknown condition triggered: " + conditionList.value[i]);
                    }
    	        }
    	    } else {
                System.out.println ("PONG: unknown condition triggered");
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
        result = s.delete_datareader (PP_quit_reader);
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
