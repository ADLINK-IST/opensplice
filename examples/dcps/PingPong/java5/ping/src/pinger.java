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

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Condition;
import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.OwnershipStrength;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.WriterDataLifecycle;
import org.omg.dds.core.status.DataAvailableStatus;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.domain.DomainParticipantQos;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.Publisher;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import pingpong.*;


class pinger {

    /*
     * configurable parameters ( through cmdline)
     */


    private char topic_id  = 's';

    private String write_partition = "PING";
    private String read_partition  = "PONG";

    ServiceEnvironment env = null;
    /*
     * Global Variables
     */

    private DomainParticipantFactory       dpf;
    private DomainParticipant              dp;
    private Publisher                      p;
    private Subscriber                     s;

    private DataWriter<PP_min_msg>      PP_min_writer;
    private DataWriter<PP_seq_msg>      PP_seq_writer;
    private DataWriter<PP_string_msg>   PP_string_writer;
    private DataWriter<PP_fixed_msg>    PP_fixed_writer;
    private DataWriter<PP_array_msg>    PP_array_writer;
    private DataWriter<PP_quit_msg>     PP_quit_writer;

    private DataReader<PP_min_msg>      PP_min_reader;
    private DataReader<PP_seq_msg>      PP_seq_reader;
    private DataReader<PP_string_msg>   PP_string_reader;
    private DataReader<PP_fixed_msg>    PP_fixed_reader;
    private DataReader<PP_array_msg>    PP_array_reader;

    private StatusCondition<DataReader<PP_min_msg>>                PP_min_sc;
    private StatusCondition<DataReader<PP_seq_msg>>                PP_seq_sc;
    private StatusCondition<DataReader<PP_string_msg>>             PP_string_sc;
    private StatusCondition<DataReader<PP_fixed_msg>>              PP_fixed_sc;
    private StatusCondition<DataReader<PP_array_msg>>              PP_array_sc;

    private Topic<PP_min_msg>            PP_min_topic;
    private Topic<PP_seq_msg>            PP_seq_topic;
    private Topic<PP_string_msg>         PP_string_topic;
    private Topic<PP_fixed_msg>          PP_fixed_topic;
    private Topic<PP_array_msg>          PP_array_topic;
    private Topic<PP_quit_msg>           PP_quit_topic;

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
        int value)
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
        long value)
    {
        String val = java.lang.Long.toString (value);
        int i;

        for (i = 0; i < (width - val.length()); i++) {
            System.out.print (" ");
        }
        System.out.print (val);
    }

    private boolean PP_min_handler (
        int nof_cycles)
    {
        int                    amount;
        boolean                result = false;
        /* System.out.println "PING: PING_min arrived"); */
        try {
            preTakeTime.timeGet ();
            List<Sample<PP_min_msg>> samples = new ArrayList<Sample<PP_min_msg>>();
            PP_min_reader.take(samples);
            postTakeTime.timeGet ();

            amount = samples.size();
            if (amount != 0) {
                if (amount > 1) {
                    System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
                }
                samples.get(0).getData().count++;
                if (samples.get(0).getData().count < nof_cycles) {
                    preWriteTime.timeGet ();
                    PP_min_writer.write (samples.get(0).getData(), InstanceHandle.nilHandle(env));
                    postWriteTime.timeGet ();
                    write_access.add_stats (postWriteTime.sub(preWriteTime));
                } else {
                    result = true;
                }
                read_access.add_stats (postTakeTime.sub(preTakeTime));
                roundtrip.add_stats (postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
            } else {
                System.out.println ("PING: PING_min triggered, but no data available");
            }
        } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
        return result;
    }

    private boolean PP_seq_handler (
        int nof_cycles)
    {
        int                     amount;
        boolean                 result = false;

        /* System.out.println "PING: PING_seq arrived"); */
        try {
            preTakeTime.timeGet ();
            List<Sample<PP_seq_msg>> samples = new ArrayList<Sample<PP_seq_msg>>();
            PP_seq_reader.take(samples);
            postTakeTime.timeGet ();

            amount = samples.size();
            if (amount != 0) {
                if (amount > 1) {
                    System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
                }
                samples.get(0).getData().count++;
                if (samples.get(0).getData().count < nof_cycles) {
                    preWriteTime.timeGet ();
                    PP_seq_writer.write (samples.get(0).getData(), InstanceHandle.nilHandle(env));
                    postWriteTime.timeGet ();
                    write_access.add_stats (postWriteTime.sub(preWriteTime));
                } else {
                    result = true;
                }
                read_access.add_stats (postTakeTime.sub(preTakeTime));
                roundtrip.add_stats (postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
            } else {
                System.out.println ("PING: PING_seq triggered, but no data available");
            }
        } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }

        return result;
    }

    private boolean PP_string_handler (
        int nof_cycles)
    {
        int                     amount;
        boolean                 result = false;

        /* System.out.println "PING: PING_string arrived"); */
        try {
            preTakeTime.timeGet ();
            List<Sample<PP_string_msg>> samples = new ArrayList<Sample<PP_string_msg>>();
            PP_string_reader.take(samples);
            postTakeTime.timeGet ();

            amount = samples.size();
            if (amount != 0) {
                if (amount > 1) {
                    System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
                }
                samples.get(0).getData().count++;
                if (samples.get(0).getData().count < nof_cycles) {
                    preWriteTime.timeGet ();
                    PP_string_writer.write (samples.get(0).getData(), InstanceHandle.nilHandle(env));
                    postWriteTime.timeGet ();
                    write_access.add_stats (postWriteTime.sub(preWriteTime));
                } else {
                    result = true;
                }
                read_access.add_stats (postTakeTime.sub(preTakeTime));
                roundtrip.add_stats (postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
            } else {
                System.out.println ("PING: PING_string triggered, but no data available");
            }
        } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
        return result;
    }

    private boolean PP_fixed_handler (
        int nof_cycles)
    {
        int                     amount;
        boolean                 result = false;
        /* System.out.println "PING: PING_fixed arrived"); */

        try {
            preTakeTime.timeGet ();
            List<Sample<PP_fixed_msg>> samples = new ArrayList<Sample<PP_fixed_msg>>();
            PP_fixed_reader.take(samples);
            postTakeTime.timeGet ();

            amount = samples.size();
            if (amount != 0) {
                if (amount > 1) {
                    System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
                }
                samples.get(0).getData().count++;
                if (samples.get(0).getData().count < nof_cycles) {
                    preWriteTime.timeGet ();
                    PP_fixed_writer.write (samples.get(0).getData(), InstanceHandle.nilHandle(env));
                    postWriteTime.timeGet ();
                    write_access.add_stats (postWriteTime.sub(preWriteTime));
                } else {
                    result = true;
                }
                read_access.add_stats (postTakeTime.sub(preTakeTime));
                roundtrip.add_stats (postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
            } else {
                System.out.println ("PING: PING_fixed triggered, but no data available");
            }
        } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
        return result;
    }

    private boolean PP_array_handler (
        int nof_cycles)
    {
        int                     amount;
        boolean                 result = false;

        /* System.out.println "PING: PING_array arrived"); */
        try {
            preTakeTime.timeGet ();
            List<Sample<PP_array_msg>> samples = new ArrayList<Sample<PP_array_msg>>();
            PP_array_reader.take(samples);
            postTakeTime.timeGet ();

            amount = samples.size();
            if (amount != 0) {
                if (amount > 1) {
                    System.out.println ("PING: Ignore excess messages : " + amount + " msg received");
                }
                samples.get(0).getData().count++;
                if (samples.get(0).getData().count < nof_cycles) {
                    preWriteTime.timeGet ();
                    PP_array_writer.write (samples.get(0).getData(), InstanceHandle.nilHandle(env));
                    postWriteTime.timeGet ();
                    write_access.add_stats (postWriteTime.sub(preWriteTime));
                } else {
                    result = true;
                }
                read_access.add_stats (postTakeTime.sub(preTakeTime));
                roundtrip.add_stats (postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
            } else {
                System.out.println ("PING: PING_array triggered, but no data available");
            }
        } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
        return result;
    }

    /*
     * P I N G
     */

    @SuppressWarnings("unchecked")
    public void run (String args[]) {

        try {
            System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                    "org.opensplice.dds.core.OsplServiceEnvironment");
            env = ServiceEnvironment.createInstance(pinger.class.getClassLoader());

            Duration wait_timeout = Duration.newDuration(3, TimeUnit.SECONDS, env);
            int result;
            boolean finish_flag = false;
            boolean timeout_flag = false;
            boolean terminate = false;

            int imax = 1;
            int i;
            int block;
            int nof_cycles = 100;
            int nof_blocks = 20;
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            /*
             * Evaluate cmdline arguments
             */
            if (args.length != 0) {
                if (args.length != 5) {
                    System.out.println("Invalid.....");
                    System.out.println("Usage: java ping [blocks blocksize topic_id WRITE_PARTITION READ_PARTITION]");
                    return;
                }
                nof_blocks = java.lang.Integer.parseInt(args[0]);
                nof_cycles = java.lang.Integer.parseInt(args[1]);
                topic_id = args[2].charAt(0);
                write_partition = args[3];
                read_partition = args[4];
            }

            System.out.println ("Starting ping example");

            /*
             * Create WaitSet
             */
            WaitSet w     = WaitSet.newWaitSet(env);


            /*
             * Create participant
             */
            dpf = DomainParticipantFactory.getInstance(env);
            dp = dpf.createParticipant();

            /*
             * Create PING publisher
             */
            Partition partition = policyFactory.Partition().withName(write_partition);
            PublisherQos pubQos = dp.getDefaultPublisherQos().withPolicy(partition);
            p = dp.createPublisher(pubQos);

            /*
             * Create PONG subscriber
             */
            partition = policyFactory.Partition().withName(read_partition);
            SubscriberQos subQos = dp.getDefaultSubscriberQos().withPolicy(partition);
            s = dp.createSubscriber(subQos);

            /*
             * Get default DataReader and DataWriter QoS settings
             */
            Reliability reliable = policyFactory.Reliability().withBestEffort();
            History history = policyFactory.History().withKeepAll();
            DataWriterQos dwQos = p.getDefaultDataWriterQos().withPolicies(reliable,history);
            DataReaderQos drQos = s.getDefaultDataReaderQos();


            /*
             * PP_min_msg
             */

            /*  Create Topic */
            PP_min_topic = dp.createTopic("PP_min_topic", PP_min_msg.class);

            /* Create datawriter */
            PP_min_writer = p.createDataWriter(PP_min_topic, dwQos);

            /* Create datareader */
            PP_min_reader = s.createDataReader(PP_min_topic,drQos);

            /* Add datareader statuscondition to waitset */
            PP_min_sc = PP_min_reader.getStatusCondition();
            PP_min_sc.setEnabledStatuses (DataAvailableStatus.class);
            w.attachCondition (PP_min_sc);

            /*
             * PP_seq_msg
             */

            /*  Create Topic */
            PP_seq_topic = dp.createTopic("PP_seq_topic", PP_seq_msg.class);

            /* Create datawriter */
            PP_seq_writer = p.createDataWriter(PP_seq_topic, dwQos);

            /* Create datareader */
            PP_seq_reader = s.createDataReader(PP_seq_topic,drQos);

            /* Add datareader statuscondition to waitset */
            PP_seq_sc = PP_seq_reader.getStatusCondition();
            PP_seq_sc.setEnabledStatuses (DataAvailableStatus.class);
            w.attachCondition (PP_seq_sc);

            /*
             * PP_string_msg
             */

            /*  Create Topic */
            PP_string_topic = dp.createTopic("PP_string_topic", PP_string_msg.class);

            /* Create datawriter */
            PP_string_writer = p.createDataWriter(PP_string_topic, dwQos);

            /* Create datareader */
            PP_string_reader = s.createDataReader(PP_string_topic,drQos);

            /* Add datareader statuscondition to waitset */
            PP_string_sc = PP_string_reader.getStatusCondition();
            PP_string_sc.setEnabledStatuses (DataAvailableStatus.class);
            w.attachCondition (PP_string_sc);

            /*
             * PP_fixed_msg
             */

            /*  Create Topic */
            PP_fixed_topic = dp.createTopic("PP_fixed_topic", PP_fixed_msg.class);

            /* Create datawriter */
            PP_fixed_writer = p.createDataWriter(PP_fixed_topic, dwQos);

            /* Create datareader */
            PP_fixed_reader = s.createDataReader(PP_fixed_topic,drQos);

            /* Add datareader statuscondition to waitset */
            PP_fixed_sc = PP_fixed_reader.getStatusCondition();
            PP_fixed_sc.setEnabledStatuses (DataAvailableStatus.class);
            w.attachCondition (PP_fixed_sc);

            /*
             * PP_array_msg
             */

            /*  Create Topic */
            PP_array_topic = dp.createTopic("PP_array_topic", PP_array_msg.class);

            /* Create datawriter */
            PP_array_writer = p.createDataWriter(PP_array_topic, dwQos);

            /* Create datareader */
            PP_array_reader = s.createDataReader(PP_array_topic,drQos);

            /* Add datareader statuscondition to waitset */
            PP_array_sc = PP_array_reader.getStatusCondition();
            PP_array_sc.setEnabledStatuses (DataAvailableStatus.class);
            w.attachCondition (PP_array_sc);

            /*
             * PP_quit_msg
             */

            /*  Create Topic */
            PP_quit_topic = dp.createTopic("PP_quit_topic", PP_quit_msg.class);
            /* Create datawriter */
            PP_quit_writer = p.createDataWriter(PP_quit_topic, dwQos);

            for (block = 0; block < nof_blocks && !terminate; block++) {
                while (!finish_flag) {
                    /*
                     * Send Initial message
                     */
                    timeout_flag = false;

                    switch(topic_id) {
                        case 'm':
                            {
                                /* System.out.println ("PING: sending initial ping_min"); */
                                PP_min_msg PPdata = new PP_min_msg();
                                PPdata.count = 0;
                                PPdata.block = block;
                                preWriteTime.timeGet ();
                                PP_min_writer.write (PPdata, InstanceHandle.nilHandle(env));
                                postWriteTime.timeGet ();
                            }
                            break;
                        case 'q':
                            {
                                /* System.out.println ("PING: sending initial ping_seq"); */
                                PP_seq_msg PPdata = new PP_seq_msg();
                                PPdata.count = 0;
                                PPdata.block = block;
                                preWriteTime.timeGet ();
                                PP_seq_writer.write (PPdata,InstanceHandle.nilHandle(env));
                                postWriteTime.timeGet ();
                            }
                            break;
                        case 's':
                            {
                                /* System.out.println ("PING: sending initial ping_string"); */
                                PP_string_msg PPdata = new PP_string_msg();
                                PPdata.count = 0;
                                PPdata.block = block;
                                PPdata.a_string = "a_string";
                                preWriteTime.timeGet ();
                                PP_string_writer.write (PPdata, InstanceHandle.nilHandle(env));
                                postWriteTime.timeGet ();
                            }
                            break;
                        case 'f':
                            {
                                /* System.out.println ("PING: sending initial ping_fixed"); */
                                PP_fixed_msg PPdata = new PP_fixed_msg ();
                                PPdata.count = 0;
                                PPdata.block = block;
                                PPdata.a_bstring = "a_bstring";
                                preWriteTime.timeGet ();
                                PP_fixed_writer.write (PPdata, InstanceHandle.nilHandle(env));
                                postWriteTime.timeGet ();
                            }
                            break;
                        case 'a':
                            {
                                /* System.out.println ("PING: sending initial ping_array"); */
                                PP_array_msg PPdata = new PP_array_msg ();
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
                                PP_array_writer.write (PPdata, InstanceHandle.nilHandle(env));
                                postWriteTime.timeGet ();
                            }
                            break;
                        case 't':
                            {
                                /* System.out.println ("PING: sending initial ping_quit"); */
                                PP_quit_msg PPdata = new PP_quit_msg ();
                                PPdata.quit = true;
                                terminate = true;
                                finish_flag = true;
                                try {
                                    Thread.sleep (1000);
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                                preWriteTime.timeGet ();
                                PP_quit_writer.write (PPdata, InstanceHandle.nilHandle(env));
                                postWriteTime.timeGet ();
                                try {
                                    Thread.sleep (1000);
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                            break;
                        default:
                            System.out.println("Invalid topic-id");
                            return;
                    }

                    if (!terminate) {
                        roundTripTime.set(preWriteTime.get());
                        write_access.add_stats (postWriteTime.sub(preWriteTime));
                        HashSet<Condition> triggeredConditions = new HashSet<Condition>();

                        /*
                         * Wait for response, calculate timing, and send another data if not ready
                         */
                        while (!(timeout_flag || finish_flag)) {
                            try {
                                w.waitForConditions(triggeredConditions, wait_timeout);
                                for (Condition cond : triggeredConditions) {
                                    if (cond == PP_min_sc) {
                                        finish_flag = PP_min_handler (nof_cycles);
                                    } else if (cond == PP_seq_sc) {
                                        finish_flag = PP_seq_handler (nof_cycles);
                                    } else if (cond == PP_string_sc) {
                                        finish_flag = PP_string_handler (nof_cycles);
                                    } else if (cond == PP_fixed_sc) {
                                        finish_flag = PP_fixed_handler (nof_cycles);
                                    } else if (cond == PP_array_sc) {
                                        finish_flag = PP_array_handler (nof_cycles);
                                    } else {
                                        System.out.println ("PING: unexpected condition triggered: " +
                                                cond + ", terminating.");
                                        finish_flag = true;
                                        terminate = true;
                                    }
                                }
                            } catch(TimeoutException te) {
                                System.out.println ("PING: TIMEOUT - message lost");
                                timeout_flag = true;
                            } catch (Exception e) {
                                System.out.println ("PING: Waitset wait failed ("+e.getMessage()+"), terminating.");
                                finish_flag = true;
                                terminate = true;
                            }
                        }
                    }
                }
                if (!terminate) {
                    finish_flag = false;
                    if (block == 0) {
                        System.out.println ("# PING PONG measurements (in us)");
                        //System.out.print ("# Executed at: ");
                        //System.out.println (DateFormat.getDateTimeInstance().format(new java.util.Date()));
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
            p.close();

            System.out.println ("Completed ping example");
        } catch (Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
    }


}
