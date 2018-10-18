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
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.omg.dds.core.AlreadyClosedException;
import org.omg.dds.core.Condition;
import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.status.DataAvailableStatus;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
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

import pingpong.PP_array_msg;
import pingpong.PP_fixed_msg;
import pingpong.PP_min_msg;
import pingpong.PP_quit_msg;
import pingpong.PP_seq_msg;
import pingpong.PP_string_msg;

public class ponger {

    /*
     * Configurable parameters (through cmdline)
     * These are the default settings
     */
    private String write_partition = "PONG";
    private String read_partition  = "PING";

    @SuppressWarnings("unchecked")
    public void run (String args[]) {
        try {
            System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                    "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment.createInstance(ponger.class.getClassLoader());
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            DomainParticipantFactory       dpf;
            DomainParticipant              dp;
            Publisher                      p;
            Subscriber                     s;

            DataWriter<PP_min_msg>      PP_min_writer;
            DataWriter<PP_seq_msg>      PP_seq_writer;
            DataWriter<PP_string_msg>   PP_string_writer;
            DataWriter<PP_fixed_msg>    PP_fixed_writer;
            DataWriter<PP_array_msg>    PP_array_writer;

            DataReader<PP_min_msg>      PP_min_reader;
            DataReader<PP_seq_msg>      PP_seq_reader;
            DataReader<PP_string_msg>   PP_string_reader;
            DataReader<PP_fixed_msg>    PP_fixed_reader;
            DataReader<PP_array_msg>    PP_array_reader;
            DataReader<PP_quit_msg>     PP_quit_reader;

            StatusCondition<DataReader<PP_min_msg>>                PP_min_sc;
            StatusCondition<DataReader<PP_seq_msg>>                PP_seq_sc;
            StatusCondition<DataReader<PP_string_msg>>             PP_string_sc;
            StatusCondition<DataReader<PP_fixed_msg>>              PP_fixed_sc;
            StatusCondition<DataReader<PP_array_msg>>              PP_array_sc;
            StatusCondition<DataReader<PP_quit_msg>>               PP_quit_sc;

            Topic<PP_min_msg>            PP_min_topic;
            Topic<PP_seq_msg>            PP_seq_topic;
            Topic<PP_string_msg>         PP_string_topic;
            Topic<PP_fixed_msg>          PP_fixed_topic;
            Topic<PP_array_msg>          PP_array_topic;
            Topic<PP_quit_msg>           PP_quit_topic;

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

            System.out.println ("Starting pong example");

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

            Reliability reliable = policyFactory.Reliability().withBestEffort();
            DataWriterQos dwQos = p.getDefaultDataWriterQos().withPolicies(reliable);
            DataReaderQos drQos = s.getDefaultDataReaderQos();

            /*
             * PP_min_msg
             */

            /*  Create Topic */
            PP_min_topic = dp.createTopic("PP_min_topic", PP_min_msg.class);

            /* Create datawriter */
            PP_min_writer = p.createDataWriter(PP_min_topic,dwQos);

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
            PP_seq_writer = p.createDataWriter(PP_seq_topic,dwQos);

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
            PP_string_writer = p.createDataWriter(PP_string_topic,dwQos);

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
            PP_fixed_writer = p.createDataWriter(PP_fixed_topic,dwQos);

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
            PP_array_writer = p.createDataWriter(PP_array_topic,dwQos);

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
            PP_quit_reader = s.createDataReader(PP_quit_topic,drQos);
            /* Add datareader statuscondition to waitset */
            PP_quit_sc = PP_quit_reader.getStatusCondition();
            PP_quit_sc.setEnabledStatuses (DataAvailableStatus.class);
            w.attachCondition (PP_quit_sc);
            HashSet<Condition> triggeredConditions = new HashSet<Condition>();

            while (!terminate) {
                Duration wait_timeout = Duration.infiniteDuration(env);

                /* System.out.println ("PONG: waiting for PING"); */
                try {
                    w.waitForConditions(triggeredConditions, wait_timeout);
                    for (Condition cond : triggeredConditions) {
                        if (cond == PP_min_sc) {
                            List<Sample<PP_min_msg>> samples = new ArrayList<Sample<PP_min_msg>>();
                            PP_min_reader.take(samples);
                            if (samples.size() != 0) {
                                for (Sample<PP_min_msg> sample : samples) {
                                    PP_min_msg msg = sample.getData();
                                    if (msg != null) { /* Check if the sample is valid. */
                                        PP_min_writer.write(msg,InstanceHandle.nilHandle(env));
                                    }
                                }
                            } else {
                                System.out.println ("PONG: PING_min triggered, but no data available ");
                            }
                        } else if (cond == PP_seq_sc) {
                            List<Sample<PP_seq_msg>> samples = new ArrayList<Sample<PP_seq_msg>>();
                            PP_seq_reader.take(samples);
                            if (samples.size() != 0) {
                                for (Sample<PP_seq_msg> sample : samples) {
                                    PP_seq_msg msg = sample.getData();
                                    if (msg != null) { /* Check if the sample is valid. */
                                        PP_seq_writer.write(msg,InstanceHandle.nilHandle(env));
                                    }
                                }
                            } else {
                                System.out.println ("PONG: PING_seq triggered, but no data available ");
                            }
                        } else if (cond == PP_string_sc) {
                            List<Sample<PP_string_msg>> samples = new ArrayList<Sample<PP_string_msg>>();
                            PP_string_reader.take(samples);
                            if (samples.size() != 0) {
                                for (Sample<PP_string_msg> sample : samples) {
                                    PP_string_msg msg = sample.getData();
                                    if (msg != null) { /* Check if the sample is valid. */
                                        PP_string_writer.write(msg,InstanceHandle.nilHandle(env));
                                    }
                                }
                            } else {
                                System.out.println ("PONG: PING_string triggered, but no data available");
                            }
                        } else if (cond == PP_fixed_sc) {
                            List<Sample<PP_fixed_msg>> samples = new ArrayList<Sample<PP_fixed_msg>>();
                            PP_fixed_reader.take(samples);
                            if (samples.size() != 0) {
                                for (Sample<PP_fixed_msg> sample : samples) {
                                    PP_fixed_msg msg = sample.getData();
                                    if (msg != null) { /* Check if the sample is valid. */
                                        PP_fixed_writer.write(msg,InstanceHandle.nilHandle(env));
                                    }
                                }
                            } else {
                                System.out.println ("PONG: PING_fixed triggered, but no data available");
                            }
                        } else if (cond == PP_array_sc) {
                            List<Sample<PP_array_msg>> samples = new ArrayList<Sample<PP_array_msg>>();
                            PP_array_reader.take(samples);
                            if (samples.size() != 0) {
                                for (Sample<PP_array_msg> sample : samples) {
                                    PP_array_msg msg = sample.getData();
                                    if (msg != null) { /* Check if the sample is valid. */
                                        PP_array_writer.write(msg,InstanceHandle.nilHandle(env));
                                    }
                                }
                            } else {
                                System.out.println ("PONG: PING_array triggered, but no data available");
                            }
                        } else if (cond == PP_quit_sc) {
                            List<Sample<PP_quit_msg>> samples = new ArrayList<Sample<PP_quit_msg>>();
                            PP_quit_reader.take(samples);
                            for (Sample<PP_quit_msg> sample : samples) {
                                PP_quit_msg msg = sample.getData();
                                if (msg == null) { /* Check if the sample is invalid. */
                                      System.out.println ("PONG: PING_quit triggered, but no data available");
                                }
                            }
                            terminate = true;
                        } else {
                            System.out.println ("PONG: unknown condition triggered ");
                        }
                    }
                } catch(AlreadyClosedException ace) {
                    terminate = true;
                } catch(Exception e) {
                    System.out.println("Error occured: " + e.getMessage());
                    e.printStackTrace();
                }
            }

            p.close();
            System.out.println ("Completed pong example");
        } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
