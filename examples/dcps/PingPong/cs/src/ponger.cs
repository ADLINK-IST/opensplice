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

using System;
using pingpong;

namespace PingPong
{
    public class ponger
    {
        /*
         * Configurable parameters (through cmdline)
         * These are the default settings
         */
        private String write_partition = "PONG";
        private String read_partition = "PING";

        public void run(String[] args)
        {

            int myDomain = DDS.DomainId.Default;
            DDS.DomainParticipantFactory dpf;
            DDS.IDomainParticipant dp;
            DDS.IPublisher p;
            DDS.ISubscriber s;

            pingpong.PP_min_msgDataWriter PP_min_writer;
            pingpong.PP_seq_msgDataWriter PP_seq_writer;
            pingpong.PP_string_msgDataWriter PP_string_writer;
            pingpong.PP_fixed_msgDataWriter PP_fixed_writer;
            pingpong.PP_array_msgDataWriter PP_array_writer;

            pingpong.PP_min_msgDataReader PP_min_reader;
            pingpong.PP_seq_msgDataReader PP_seq_reader;
            pingpong.PP_string_msgDataReader PP_string_reader;
            pingpong.PP_fixed_msgDataReader PP_fixed_reader;
            pingpong.PP_array_msgDataReader PP_array_reader;
            pingpong.PP_quit_msgDataReader PP_quit_reader;

            pingpong.PP_min_msgTypeSupport PP_min_dt;
            pingpong.PP_seq_msgTypeSupport PP_seq_dt;
            pingpong.PP_string_msgTypeSupport PP_string_dt;
            pingpong.PP_fixed_msgTypeSupport PP_fixed_dt;
            pingpong.PP_array_msgTypeSupport PP_array_dt;
            pingpong.PP_quit_msgTypeSupport PP_quit_dt;

            pingpong.PP_min_msg[] PP_min_dataList = null;
            pingpong.PP_seq_msg[] PP_seq_dataList = null;
            pingpong.PP_string_msg[] PP_string_dataList = null;
            pingpong.PP_fixed_msg[] PP_fixed_dataList = null;
            pingpong.PP_array_msg[] PP_array_dataList = null;
            pingpong.PP_quit_msg[] PP_quit_dataList = null;

            DDS.IStatusCondition PP_min_sc;
            DDS.IStatusCondition PP_seq_sc;
            DDS.IStatusCondition PP_string_sc;
            DDS.IStatusCondition PP_fixed_sc;
            DDS.IStatusCondition PP_array_sc;
            DDS.IStatusCondition PP_quit_sc;

            DDS.ITopic PP_min_topic;
            DDS.ITopic PP_seq_topic;
            DDS.ITopic PP_string_topic;
            DDS.ITopic PP_fixed_topic;
            DDS.ITopic PP_array_topic;
            DDS.ITopic PP_quit_topic;

            DDS.ICondition[] conditionList = null;
            DDS.SampleInfo[] infoList = null;
            DDS.IWaitSet w;

            DDS.DomainParticipantQos dpQos;
            DDS.TopicQos tQos;
            DDS.PublisherQos pQos;
            DDS.DataWriterQos dwQos;
            DDS.SubscriberQos sQos;
            DDS.DataReaderQos drQos;

            Boolean terminate = false;

            DDS.ReturnCode result;
            int i;
            int imax;
            int j;
            int jmax;

            /*
             * Evaluate cmdline arguments
             */

            if (args.Length != 0)
            {
                if (args.Length != 2)
                {
                    System.Console.WriteLine("Invalid.....");
                    System.Console.WriteLine("Usage: pong [READ_PARTITION WRITE_PARTITION]");
                    Environment.Exit(1);
                }
                read_partition = args[0];
                write_partition = args[1];
            }

            /*
             * Create WaitSet
             */
            w = new DDS.WaitSet();
            /*
             * Initialize Qos variables
             */
            dpQos = new DDS.DomainParticipantQos();
            tQos = new DDS.TopicQos();
            pQos = new DDS.PublisherQos();
            dwQos = new DDS.DataWriterQos();
            sQos = new DDS.SubscriberQos();
            drQos = new DDS.DataReaderQos();

            /*
             * Create participant
             */
            dpf = DDS.DomainParticipantFactory.Instance;
            dpf.GetDefaultParticipantQos(ref dpQos);
            dp = dpf.CreateParticipant(myDomain, dpQos, null, DDS.StatusKind.Any);
            if (dp == null)
            {
                System.Console.WriteLine("PING: ERROR - Splice Daemon not running");
                return;
            }

            /* 
             * Create PONG publisher
             */
            dp.GetDefaultPublisherQos(ref pQos);
            pQos.Partition.Name = new String[1];
            pQos.Partition.Name[0] = write_partition;
            p = dp.CreatePublisher(pQos);

            /*
             * Create PING subscriber
             */
            dp.GetDefaultSubscriberQos(ref sQos);
            sQos.Partition.Name = new String[1];
            sQos.Partition.Name[0] = read_partition;
            s = dp.CreateSubscriber(sQos);
            /*
             * Get default DataReader and DataWriter QoS settings
             */
            p.GetDefaultDataWriterQos(ref dwQos);
            s.GetDefaultDataReaderQos(ref drQos);

            /*
             * Get default Topic Qos settings
             */
            dp.GetDefaultTopicQos(ref tQos);


            /* match data reader/writer qos with topic qos */
            p.CopyFromTopicQos(ref dwQos, tQos);
            s.CopyFromTopicQos(ref drQos, tQos);

            /*
             * PP_min_msg
             */

            /* Create Topic */
            PP_min_dt = new pingpong.PP_min_msgTypeSupport();
            PP_min_dt.RegisterType(dp, "pingpong::PP_min_msg");
            PP_min_topic = dp.CreateTopic("PP_min_topic", "pingpong::PP_min_msg", tQos);


            /* Create datawriter */
            PP_min_writer = p.CreateDataWriter(PP_min_topic, dwQos) as pingpong.PP_min_msgDataWriter;

            /* Create datareader */
            PP_min_reader = s.CreateDataReader(PP_min_topic, drQos) as pingpong.PP_min_msgDataReader;

            /* Add datareader statuscondition to waitset */
            PP_min_sc = PP_min_reader.StatusCondition;
            PP_min_sc.SetEnabledStatuses(DDS.StatusKind.DataAvailable);
            result = w.AttachCondition(PP_min_sc);
            // assert(result == RETCODE_OK.value);

            /*
             * PP_seq_msg
             */

            /*  Create Topic */
            PP_seq_dt = new pingpong.PP_seq_msgTypeSupport();
            PP_seq_dt.RegisterType(dp, "pingpong::PP_seq_msg");
            PP_seq_topic = dp.CreateTopic("PP_seq_topic", "pingpong::PP_seq_msg", tQos);

            /* Create datawriter */
            PP_seq_writer = p.CreateDataWriter(PP_seq_topic, dwQos) as pingpong.PP_seq_msgDataWriter;

            /* Create datareader */
            PP_seq_reader = s.CreateDataReader(PP_seq_topic, drQos) as pingpong.PP_seq_msgDataReader;

            /* Add datareader statuscondition to waitset */
            PP_seq_sc = PP_seq_reader.StatusCondition;
            PP_seq_sc.SetEnabledStatuses(DDS.StatusKind.DataAvailable);
            result = w.AttachCondition(PP_seq_sc);
            //assert(result == RETCODE_OK.value);

            /*
             * PP_string_msg
             */

            /*  Create Topic */
            PP_string_dt = new pingpong.PP_string_msgTypeSupport();
            PP_string_dt.RegisterType(dp, "pingpong::PP_string_msg");
            PP_string_topic = dp.CreateTopic("PP_string_topic", "pingpong::PP_string_msg", tQos);

            /* Create datawriter */
            PP_string_writer = p.CreateDataWriter(PP_string_topic, dwQos) as pingpong.PP_string_msgDataWriter;

            /* Create datareader */
            PP_string_reader = s.CreateDataReader(PP_string_topic,drQos) as pingpong.PP_string_msgDataReader;

            /* Add datareader statuscondition to waitset */
            PP_string_sc = PP_string_reader.StatusCondition;
            PP_string_sc.SetEnabledStatuses(DDS.StatusKind.DataAvailable);
            result = w.AttachCondition(PP_string_sc);
            //assert(result == RETCODE_OK.value);

            /*
             * PP_fixed_msg
             */

            /*  Create Topic */
            PP_fixed_dt = new pingpong.PP_fixed_msgTypeSupport();
            PP_fixed_dt.RegisterType(dp, "pingpong::PP_fixed_msg");
            PP_fixed_topic = dp.CreateTopic("PP_fixed_topic", "pingpong::PP_fixed_msg", tQos);

            /* Create datawriter */
            PP_fixed_writer = p.CreateDataWriter(PP_fixed_topic, dwQos) as pingpong.PP_fixed_msgDataWriter;

            /* Create datareader */
            PP_fixed_reader = s.CreateDataReader(PP_fixed_topic, drQos) as pingpong.PP_fixed_msgDataReader;

            /* Add datareader statuscondition to waitset */
            PP_fixed_sc = PP_fixed_reader.StatusCondition;
            PP_fixed_sc.SetEnabledStatuses(DDS.StatusKind.DataAvailable);
            result = w.AttachCondition(PP_fixed_sc);
            //assert(result == RETCODE_OK.value);

            /*
             * PP_array_msg
             */

            /*  Create Topic */
            PP_array_dt = new pingpong.PP_array_msgTypeSupport();
            PP_array_dt.RegisterType(dp, "pingpong::PP_array_msg");
            PP_array_topic = dp.CreateTopic("PP_array_topic", "pingpong::PP_array_msg", tQos);

            /* Create datawriter */
            PP_array_writer = p.CreateDataWriter(PP_array_topic, dwQos) as pingpong.PP_array_msgDataWriter;

            /* Create datareader */
            PP_array_reader = s.CreateDataReader(PP_array_topic, drQos) as pingpong.PP_array_msgDataReader;

            /* Add datareader statuscondition to waitset */
            PP_array_sc = PP_array_reader.StatusCondition;
            PP_array_sc.SetEnabledStatuses(DDS.StatusKind.DataAvailable);
            result = w.AttachCondition(PP_array_sc);
            //assert(result == RETCODE_OK.value);

            /*
             * PP_quit_msg
             */

            /*  Create Topic */
            PP_quit_dt = new pingpong.PP_quit_msgTypeSupport();
            PP_quit_dt.RegisterType(dp, "pingpong::PP_quit_msg");
            PP_quit_topic = dp.CreateTopic("PP_quit_topic", "pingpong::PP_quit_msg", tQos);

            /* Create datareader */
            PP_quit_reader = s.CreateDataReader(PP_quit_topic, drQos) as pingpong.PP_quit_msgDataReader;

            /* Add datareader statuscondition to waitset */
            PP_quit_sc = PP_quit_reader.StatusCondition;
            PP_quit_sc.SetEnabledStatuses(DDS.StatusKind.DataAvailable);
            result = w.AttachCondition(PP_quit_sc);
            //assert(result == RETCODE_OK.value);

            while (!terminate)
            {
                DDS.Duration wait_timeout = new DDS.Duration(DDS.Duration.InfiniteSec, DDS.Duration.InfiniteSec);


                result = w.Wait(ref conditionList, wait_timeout);
                ErrorHandler.checkStatus(result, "wait did not succeeed");

                if (result == DDS.ReturnCode.AlreadyDeleted)
                {
                    terminate = true;
                    continue;
                }
                if (conditionList != null)
                {
                    imax = conditionList.Length;
                    for (i = 0; i < imax; i++)
                    {
                        if (conditionList[i] == PP_min_sc)
                        {
                            result = PP_min_reader.Take(ref PP_min_dataList, ref infoList,
                                          DDS.SampleStateKind.Any, DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
                            jmax = PP_min_dataList.Length;
                            if (jmax != 0)
                            {
                                for (j = 0; j < jmax; j++)
                                {
                                    if (infoList[j].ValidData)
                                    {
                                        result = PP_min_writer.Write(PP_min_dataList[j], DDS.InstanceHandle.Nil);
                                    }
                                }
                                result = PP_min_reader.ReturnLoan(ref PP_min_dataList, ref infoList);
                            }
                            else
                            {
                                System.Console.WriteLine("PONG: PING_min triggered, but no data available");
                            }
                        }
                        else if (conditionList[i] == PP_seq_sc)
                        {
                            /*  System.Console.WriteLine ("PONG: PING_seq arrived"); */
                            result = PP_seq_reader.Take(ref PP_seq_dataList, ref infoList,
                                         DDS.SampleStateKind.Any, DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
                            jmax = PP_seq_dataList.Length;
                            if (jmax != 0)
                            {
                                for (j = 0; j < jmax; j++)
                                {
                                    if (infoList[j].ValidData)
                                    {
                                        result = PP_seq_writer.Write(PP_seq_dataList[j], DDS.InstanceHandle.Nil);
                                    }
                                }
                                result = PP_seq_reader.ReturnLoan(ref PP_seq_dataList, ref infoList);
                            }
                            else
                            {
                                System.Console.WriteLine("PONG: PING_seq triggered, but no data available");
                            }
                        }
                        else if (conditionList[i] == PP_string_sc)
                        {
                            /*  System.Console.WriteLine ("PONG: PING_string arrived"); */
                            result = PP_string_reader.Take(ref PP_string_dataList, ref infoList,
                                         DDS.SampleStateKind.Any, DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
                            jmax = PP_string_dataList.Length;
                            if (jmax != 0)
                            {
                                for (j = 0; j < jmax; j++)
                                {
                                    if (infoList[j].ValidData)
                                    {
                                        result = PP_string_writer.Write(PP_string_dataList[j], DDS.InstanceHandle.Nil);
                                    }
                                }
                                result = PP_string_reader.ReturnLoan(ref PP_string_dataList, ref infoList);
                            }
                            else
                            {
                                System.Console.WriteLine("PONG: PING_string triggered, but no data available");
                            }
                        }
                        else if (conditionList[i] == PP_fixed_sc)
                        {
                            /*  System.Console.WriteLine ("PONG: PING_fixed arrived"); */
                            result = PP_fixed_reader.Take(ref PP_fixed_dataList, ref infoList,
                                         DDS.SampleStateKind.Any, DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
                            jmax = PP_fixed_dataList.Length;
                            if (jmax != 0)
                            {
                                for (j = 0; j < jmax; j++)
                                {
                                    if (infoList[j].ValidData)
                                    {
                                        result = PP_fixed_writer.Write(PP_fixed_dataList[j], DDS.InstanceHandle.Nil);
                                    }
                                }
                                result = PP_fixed_reader.ReturnLoan(ref PP_fixed_dataList, ref infoList);
                            }
                            else
                            {
                                System.Console.WriteLine("PONG: PING_fixed triggered, but no data available");
                            }
                        }
                        else if (conditionList[i] == PP_array_sc)
                        {
                            /*  System.Console.WriteLine ("PONG: PING_array arrived"); */
                            result = PP_array_reader.Take(ref PP_array_dataList, ref infoList,
                                         DDS.SampleStateKind.Any, DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
                            jmax = PP_array_dataList.Length;
                            if (jmax != 0)
                            {
                                for (j = 0; j < jmax; j++)
                                {
                                    if (infoList[j].ValidData)
                                    {
                                        result = PP_array_writer.Write(PP_array_dataList[j], DDS.InstanceHandle.Nil);
                                    }
                                }
                                result = PP_array_reader.ReturnLoan(ref PP_array_dataList, ref infoList);
                            }
                            else
                            {
                                System.Console.WriteLine("PONG: PING_array triggered, but no data available");
                            }
                        }
                        else if (conditionList[i] == PP_quit_sc)
                        {
                            /*  System.Console.WriteLine ("PONG: PING_quit arrived"); */
                            result = PP_quit_reader.Take(ref PP_quit_dataList, ref infoList,
                                         DDS.SampleStateKind.Any, DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
                            jmax = PP_quit_dataList.Length;
                            if (jmax != 0)
                            {
                                result = PP_quit_reader.ReturnLoan(ref PP_quit_dataList, ref infoList);
                            }
                            else
                            {
                                System.Console.WriteLine("PONG: PING_quit triggered, but no data available");
                            }
                            terminate = true;
                        }
                        else
                        {
                            System.Console.WriteLine("PONG: unknown condition triggered: " + conditionList[i]);
                        }
                    }
                }
                else
                {
                    System.Console.WriteLine("PONG: unknown condition triggered");
                }
            }

            result = s.DeleteDataReader(PP_min_reader);
            result = p.DeleteDataWriter(PP_min_writer);
            result = s.DeleteDataReader(PP_seq_reader);
            result = p.DeleteDataWriter(PP_seq_writer);
            result = s.DeleteDataReader(PP_string_reader);
            result = p.DeleteDataWriter(PP_string_writer);
            result = s.DeleteDataReader(PP_fixed_reader);
            result = p.DeleteDataWriter(PP_fixed_writer);
            result = s.DeleteDataReader(PP_array_reader);
            result = p.DeleteDataWriter(PP_array_writer);
            result = s.DeleteDataReader(PP_quit_reader);
            result = dp.DeleteSubscriber(s);
            result = dp.DeletePublisher(p);
            result = dp.DeleteTopic(PP_min_topic);
            result = dp.DeleteTopic(PP_seq_topic);
            result = dp.DeleteTopic(PP_string_topic);
            result = dp.DeleteTopic(PP_fixed_topic);
            result = dp.DeleteTopic(PP_array_topic);
            result = dp.DeleteTopic(PP_quit_topic);
            result = dpf.DeleteParticipant(dp);

            return;
        }
    }
}
