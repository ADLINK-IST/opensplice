
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
using DDS.OpenSplice;

namespace PingPong
{
    public class pinger
    {

        public pinger()
        {
        }

        /*
         * configurable parameters ( through cmdline)
         */

        private char topic_id = 's';

        private String write_partition = "PING";
        private String read_partition = "PONG";

        /*
         * Global Variables
         */

        private int myDomain = DDS.DomainId.Default;
        private DDS.DomainParticipantFactory dpf;
        private DDS.IDomainParticipant dp;
        private DDS.IPublisher p;
        private DDS.ISubscriber s;

        private pingpong.PP_min_msgDataWriter PP_min_writer;
        private pingpong.PP_seq_msgDataWriter PP_seq_writer;
        private pingpong.PP_string_msgDataWriter PP_string_writer;
        private pingpong.PP_fixed_msgDataWriter PP_fixed_writer;
        private pingpong.PP_array_msgDataWriter PP_array_writer;
        private pingpong.PP_quit_msgDataWriter PP_quit_writer;

        private pingpong.PP_min_msgDataReader PP_min_reader;
        private pingpong.PP_seq_msgDataReader PP_seq_reader;
        private pingpong.PP_string_msgDataReader PP_string_reader;
        private pingpong.PP_fixed_msgDataReader PP_fixed_reader;
        private pingpong.PP_array_msgDataReader PP_array_reader;

        private pingpong.PP_min_msgTypeSupport PP_min_dt;
        private pingpong.PP_seq_msgTypeSupport PP_seq_dt;
        private pingpong.PP_string_msgTypeSupport PP_string_dt;
        private pingpong.PP_fixed_msgTypeSupport PP_fixed_dt;
        private pingpong.PP_array_msgTypeSupport PP_array_dt;
        private pingpong.PP_quit_msgTypeSupport PP_quit_dt;

        private pingpong.PP_min_msg[] PP_min_dataList = null;
        private pingpong.PP_seq_msg[] PP_seq_dataList = null;
        private pingpong.PP_string_msg[] PP_string_dataList = null;
        private pingpong.PP_fixed_msg[] PP_fixed_dataList = null;
        private pingpong.PP_array_msg[] PP_array_dataList = null;

        private DDS.IStatusCondition PP_min_sc;
        private DDS.IStatusCondition PP_seq_sc;
        private DDS.IStatusCondition PP_string_sc;
        private DDS.IStatusCondition PP_fixed_sc;
        private DDS.IStatusCondition PP_array_sc;

        private DDS.ITopic PP_min_topic;
        private DDS.ITopic PP_seq_topic;
        private DDS.ITopic PP_string_topic;
        private DDS.ITopic PP_fixed_topic;
        private DDS.ITopic PP_array_topic;
        private DDS.ITopic PP_quit_topic;

        private stats roundtrip = new stats("roundtrip");
        private stats write_access = new stats("write_access");
        private stats read_access = new stats("read_access");
        private time roundTripTime = new time();
        private time preWriteTime = new time();
        private time postWriteTime = new time();
        private time preTakeTime = new time();
        private time postTakeTime = new time();
        
        /*
         * P I N G
         */

        public void run(String[] args)
        {
            System.Console.WriteLine("starting PING");
            DDS.ICondition[] conditionList = null;
            DDS.IWaitSet w;

            DDS.DomainParticipantQos dpQos;
            DDS.TopicQos tQos;
            DDS.PublisherQos pQos;
            DDS.DataWriterQos dwQos;
            DDS.SubscriberQos sQos;
            DDS.DataReaderQos drQos;

            DDS.Duration wait_timeout = new DDS.Duration(3, 0);

            DDS.ReturnCode result;
            Boolean finish_flag = false;
            Boolean timeout_flag = false;
            Boolean terminate = false;

            int imax = 1;
            int i;
            uint block;
            int nof_cycles = 100;
            int nof_blocks = 20;

            /**
             * Command Line argument processing
             */
            if (args.Length != 0)
            {
                if (args.Length != 5)
                {
                    System.Console.WriteLine("Invalid.....");
                    System.Console.WriteLine("Usage: ping blocks blocksize topic_id WRITE_PARTITION READ_PARTITION ");
                    Environment.Exit(1);
                }
                nof_blocks = int.Parse(args[0]);
                nof_cycles = int.Parse(args[1]);
                topic_id = args[2][0];
                write_partition = args[3];
                read_partition = args[4];
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
            
            ErrorHandler.checkHandle(dpf, "DDS.DomainParticipantFactory.Instance");

            dp = dpf.CreateParticipant(myDomain, dpQos);
            if (dp == null)
            {
                System.Console.WriteLine("PING: ERROR - Splice Daemon not running");
                return;
            }

            /*
              * Create PING publisher
              */
            dp.GetDefaultPublisherQos(ref pQos);
            pQos.Partition.Name = new String[1];
            pQos.Partition.Name[0] = write_partition;
            p = dp.CreatePublisher(pQos);

            /*
             * Create PONG subscriber
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

            /*match data reader/writer qos with topic qos*/
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
            ErrorHandler.checkStatus(result, "attach condition pp_min_reader");
            //assert (result == RETCODE_OK.value);

            /*
             * PP_seq_msg
             */
            /* Create Topic */
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
            ErrorHandler.checkStatus(result, "attach condition pp_seq_reader");
            //assert (result == RETCODE_OK.value);

            /*
             * PP_string_msg
             */

            /* Create Topic */
            PP_string_dt = new pingpong.PP_string_msgTypeSupport();
            PP_string_dt.RegisterType(dp, "pingpong::PP_string_msg");
            PP_string_topic = dp.CreateTopic("PP_string_topic", "pingpong::PP_string_msg", tQos);

            /* Create datawriter */
            PP_string_writer = p.CreateDataWriter(PP_string_topic, dwQos) as pingpong.PP_string_msgDataWriter;

            /* Create datareader */
            PP_string_reader = s.CreateDataReader(PP_string_topic, drQos) as pingpong.PP_string_msgDataReader;

            /* Add datareader statuscondition to waitset */
            PP_string_sc = PP_string_reader.StatusCondition;
            PP_string_sc.SetEnabledStatuses(DDS.StatusKind.DataAvailable);

            result = w.AttachCondition(PP_string_sc);
            ErrorHandler.checkStatus(result, "attach condition pp_string_reader");
            //assert (result == RETCODE_OK.value);

            /*
             * PP_fixed_msg
             */

            /* Create Topic */
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
            ErrorHandler.checkStatus(result, "attach condition pp_fixed_reader");
            //assert (result == RETCODE_OK.value);

            /*
             * PP_array_msg
             */

            /* Create Topic */
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

            ErrorHandler.checkStatus(result, "attach condition pp_array_reader");
            //assert (result == RETCODE_OK.value);

            /*
             * PP_quit_msg
             */

            /* Create Topic */
            PP_quit_dt = new pingpong.PP_quit_msgTypeSupport();
            PP_quit_dt.RegisterType(dp, "pingpong::PP_quit_msg");
            PP_quit_topic = dp.CreateTopic("PP_quit_topic", "pingpong::PP_quit_msg", tQos);

            /* Create datawriter */
            PP_quit_writer = p.CreateDataWriter(PP_quit_topic, dwQos) as pingpong.PP_quit_msgDataWriter;

            for (block = 0; block < nof_blocks && !terminate; block++)
            {
                while (!finish_flag)
                {
                    /*
                     * Send Initial message
                     */
                    timeout_flag = false;

                    switch (topic_id)
                    {
                        case 'm':
                            {
                                /* System.Console.WriteLine ("PING: sending initial ping_min"); */
                                pingpong.PP_min_msg PPdata = new pingpong.PP_min_msg();
                                PPdata.count = 0;
                                PPdata.block = block;
                                preWriteTime.timeGet();
                                result = PP_min_writer.Write(PPdata);
                                postWriteTime.timeGet();
                            }
                            break;
                        case 'q':
                            {
                                /* System.Console.WriteLine ("PING: sending initial ping_seq"); */
                                pingpong.PP_seq_msg PPdata = new pingpong.PP_seq_msg();
                                PPdata.count = 0;
                                PPdata.block = block;
                                preWriteTime.timeGet();
                                result = PP_seq_writer.Write(PPdata);
                                postWriteTime.timeGet();
                            }
                            break;
                        case 's':
                            {
                                //System.Console.WriteLine ("PING: sending initial ping_string");
                                pingpong.PP_string_msg PPdata = new pingpong.PP_string_msg();
                                PPdata.count = 0;
                                PPdata.block = block;
                                PPdata.a_string = "a_string " + block.ToString();
                                preWriteTime.timeGet();
                                result = PP_string_writer.Write(PPdata);
                                ErrorHandler.checkStatus(result, "writing PPData in case S");
                                postWriteTime.timeGet();
                            }
                            break;
                        case 'f':
                            {
                                /* System.Console.WriteLine ("PING: sending initial ping_fixed"); */
                                pingpong.PP_fixed_msg PPdata = new pingpong.PP_fixed_msg();
                                PPdata.count = 0;
                                PPdata.block = block;
                                preWriteTime.timeGet();
                                result = PP_fixed_writer
                                        .Write(PPdata);
                                postWriteTime.timeGet();
                            }
                            break;
                        case 'a':
                            {
                                /* System.Console.WriteLine ("PING: sending initial ping_array"); */
                                pingpong.PP_array_msg PPdata = new pingpong.PP_array_msg();
                                PPdata.count = 0;
                                PPdata.block = block;
                                PPdata.str_arr_char = new char[10];
                                PPdata.str_arr_octet = new byte[10];
                                PPdata.str_arr_short = new short[10];
                                PPdata.str_arr_ushort = new ushort[10];
                                PPdata.str_arr_long = new int[10];
                                PPdata.str_arr_ulong = new uint[10];
                                PPdata.str_arr_longlong = new long[10];
                                PPdata.str_arr_ulonglong = new ulong[10];
                                PPdata.str_arr_float = new float[10];
                                PPdata.str_arr_double = new double[10];
                                PPdata.str_arr_boolean = new Boolean[11];
                                preWriteTime.timeGet();
                                result = PP_array_writer
                                        .Write(PPdata);
                                postWriteTime.timeGet();
                            }
                            break;
                        case 't':
                            {
                                /* System.Console.WriteLine ("PING: sending initial ping_quit"); */
                                pingpong.PP_quit_msg PPdata = new pingpong.PP_quit_msg();
                                PPdata.quit = true;
                                terminate = true;
                                finish_flag = true;
                                System.Threading.Thread.Sleep(1000); 
                                preWriteTime.timeGet();
                                result = PP_quit_writer.Write(PPdata);
                                postWriteTime.timeGet();
                                System.Threading.Thread.Sleep(1000);           
                            }
                            break;
                        default:
                            System.Console.WriteLine("Invalid topic-id");
                            return;
                    }

                    if (!terminate)
                    {
                        roundTripTime.set(preWriteTime.get());
                        write_access.add_stats(postWriteTime.sub(preWriteTime));

                        /*
                         * Wait for response, calculate timing, and send another
                         * data if not ready
                         */
                        while (!(timeout_flag || finish_flag))
                        {
                            result = w.Wait(ref conditionList, wait_timeout);
							if(result == DDS.ReturnCode.Ok || result == DDS.ReturnCode.NoData || result == DDS.ReturnCode.Timeout)
							{
								//ErrorHandler.checkStatus(result, "wait did not work condition list is probably null!");
								if (conditionList != null)
								{
									imax = conditionList.Length;
									if (imax != 0)
									{
										for (i = 0; i < imax; i++)
										{
											if (conditionList[i] == PP_min_sc)
											{
												finish_flag = PP_min_handler(nof_cycles);
											}
											else if (conditionList[i] == PP_seq_sc)
											{
												finish_flag = PP_seq_handler(nof_cycles);
											}
											else if (conditionList[i] == PP_string_sc)
											{
												finish_flag = PP_string_handler(nof_cycles);
											}
											else if (conditionList[i] == PP_fixed_sc)
											{
												finish_flag = PP_fixed_handler(nof_cycles);
											}
											else if (conditionList[i] == PP_array_sc)
											{
												finish_flag = PP_array_handler(nof_cycles);
											}
											else
											{
												System.Console.WriteLine("PING: unexpected condition triggered: "
																+ conditionList[i]);
											}
										}
									}
									else
									{
										System.Console.WriteLine("PING: TIMEOUT - message lost (1)");
										timeout_flag = true;
									}
								}
								else
								{
									System.Console.WriteLine("PING: TIMEOUT - message lost (2)");
									timeout_flag = true;
								}
							} else
							{
								System.Console.WriteLine("PING: Waitset wait failed (code "+result+"), terminating");
								finish_flag = true;
								terminate = true;
							}
						}
                    }
                }
                if (!terminate)
                {
                    finish_flag = false;
                    if (block == 0)
                    {
                        System.Console.WriteLine("# PING PONG measurements (in us)");
                        System.Console.WriteLine("# Executed at: ");
                        System.Console.WriteLine("#           Roundtrip time [us]             Write-access time [us]          Read-access time [us]");
                        System.Console.WriteLine("# Block     Count   mean    min    max      Count   mean    min    max      Count   mean    min    max");
                    }

                    System.Console.WriteLine(String.Format("{0,-6} {1, 10} {2, 6} {3, 6} {4, 6} {5, 10} {6, 6} {7, 6} {8, 6} {9, 10} {10, 6} {11, 6} {12, 6}",
                        block,roundtrip.count,roundtrip.average,roundtrip.min, roundtrip.max,write_access.count,
                        write_access.average,write_access.min, write_access.max, read_access.count, read_access.average,
                        read_access.min, read_access.max));
                    Console.Out.Flush();
                    write_access.init_stats();
                    read_access.init_stats();
                    roundtrip.init_stats();
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
            result = p.DeleteDataWriter(PP_quit_writer);
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

        private static void print_formatted(int width, long value)
        {

            String val = Convert.ToString(value);
            int i;

            for (i = 0; i < (width - val.Length); i++)
            {
                System.Console.WriteLine(" ");
            }
            System.Console.WriteLine(val);
        }

        private Boolean PP_min_handler(int nof_cycles)
        {
            DDS.SampleInfo[] infoList = null;
            int amount;
            Boolean result = false;
            DDS.ReturnCode dds_result;



            preTakeTime.timeGet();

            dds_result = PP_min_reader.Take(ref PP_min_dataList, ref infoList,
                    DDS.SampleStateKind.Any,
                    DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);


            //assert (dds_result == RETCODE_OK.value);

            postTakeTime.timeGet();

            amount = PP_min_dataList.Length;
            if (amount != 0)
            {
                if (amount > 1)
                {
                    System.Console.WriteLine("PING: Ignore excess messages : " + amount
                            + " msg received");
                }
                PP_min_dataList[0].count++;
                if (PP_min_dataList[0].count < nof_cycles)
                {
                    preWriteTime.timeGet();
                    dds_result = PP_min_writer.Write(PP_min_dataList[0], DDS.InstanceHandle.Nil);
                    //assert (dds_result == RETCODE_OK.value);
                    postWriteTime.timeGet();
                    write_access.add_stats(postWriteTime.sub(preWriteTime));
                }
                else
                {
                    result = true;
                }
                read_access.add_stats(postTakeTime.sub(preTakeTime));
                roundtrip.add_stats(postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
                dds_result = PP_min_reader.ReturnLoan(ref PP_min_dataList, ref infoList);
                //assert (dds_result == RETCODE_OK.value);
            }
            else
            {
                System.Console
                        .WriteLine("PING: PING_min triggered, but no data available");
            }
            return result;
        }
        private Boolean PP_fixed_handler(int nof_cycles)
        {
            DDS.SampleInfo[] infoList = null;
            int amount;
            Boolean result = false;
            DDS.ReturnCode dds_result;

            /* System.out.println "PING: PING_fixed arrived"); */

            preTakeTime.timeGet();
            dds_result = PP_fixed_reader.Take(ref PP_fixed_dataList, ref infoList,
                     DDS.SampleStateKind.Any,
                    DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
            //assert (dds_result == RETCODE_OK.value);
            postTakeTime.timeGet();

            amount = PP_fixed_dataList.Length;
            if (amount != 0)
            {
                if (amount > 1)
                {
                    System.Console.WriteLine("PING: Ignore excess messages : " + amount
                            + " msg received");
                }
                PP_fixed_dataList[0].count++;
                if (PP_fixed_dataList[0].count < nof_cycles)
                {
                    preWriteTime.timeGet();
                    dds_result = PP_fixed_writer.Write(PP_fixed_dataList[0],
                            DDS.InstanceHandle.Nil);
                    //assert (dds_result == RETCODE_OK.value);
                    postWriteTime.timeGet();
                    write_access.add_stats(postWriteTime.sub(preWriteTime));
                }
                else
                {
                    result = true;
                }
                read_access.add_stats(postTakeTime.sub(preTakeTime));
                roundtrip.add_stats(postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
                dds_result = PP_fixed_reader.ReturnLoan(ref PP_fixed_dataList, ref infoList);
                //assert (dds_result == RETCODE_OK.value);
            }
            else
            {
                System.Console.WriteLine("PING: PING_fixed triggered, but no data available");
            }
            return result;
        }

        private Boolean PP_array_handler(int nof_cycles)
        {
            DDS.SampleInfo[] infoList = null;
            int amount;
            Boolean result = false;
            DDS.ReturnCode dds_result;

            /* System.out.println "PING: PING_array arrived"); */

            preTakeTime.timeGet();
            dds_result = PP_array_reader.Take(ref PP_array_dataList, ref infoList,
                    DDS.SampleStateKind.Any,
                    DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
            //assert (dds_result == RETCODE_OK.value);
            postTakeTime.timeGet();

            amount = PP_array_dataList.Length;
            if (amount != 0)
            {
                if (amount > 1)
                {
                    System.Console.WriteLine("PING: Ignore excess messages : " + amount
                            + " msg received");
                }
                PP_array_dataList[0].count++;
                if (PP_array_dataList[0].count < nof_cycles)
                {
                    preWriteTime.timeGet();
                    dds_result = PP_array_writer.Write(PP_array_dataList[0],
                            DDS.InstanceHandle.Nil);
                    //assert (dds_result == RETCODE_OK.value);
                    postWriteTime.timeGet();
                    write_access.add_stats(postWriteTime.sub(preWriteTime));
                }
                else
                {
                    result = true;
                }
                read_access.add_stats(postTakeTime.sub(preTakeTime));
                roundtrip.add_stats(postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
                dds_result = PP_array_reader.ReturnLoan(ref PP_array_dataList, ref infoList);
                //assert (dds_result == RETCODE_OK.value);
            }
            else
            {
                System.Console.WriteLine("PING: PING_array triggered, but no data available");
            }
            return result;
        }
        private Boolean PP_seq_handler(int nof_cycles)
        {
            DDS.SampleInfo[] infoList = null;
            int amount;
            Boolean result = false;
            DDS.ReturnCode dds_result;

            /* System.out.println "PING: PING_seq arrived"); */

            preTakeTime.timeGet();
            dds_result = PP_seq_reader.Take(ref PP_seq_dataList, ref infoList,
                    DDS.SampleStateKind.Any,
                    DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
            //assert (dds_result == RETCODE_OK.value);
            postTakeTime.timeGet();

            amount = PP_seq_dataList.Length;
            if (amount != 0)
            {
                if (amount > 1)
                {
                    System.Console.WriteLine("PING: Ignore excess messages : " + amount
                            + " msg received");
                }
                PP_seq_dataList[0].count++;
                if (PP_seq_dataList[0].count < nof_cycles)
                {
                    preWriteTime.timeGet();
                    dds_result = PP_seq_writer.Write(PP_seq_dataList[0],
                            DDS.InstanceHandle.Nil);
                    //assert (dds_result == RETCODE_OK.value);
                    postWriteTime.timeGet();
                    write_access.add_stats(postWriteTime.sub(preWriteTime));
                }
                else
                {
                    result = true;
                }
                read_access.add_stats(postTakeTime.sub(preTakeTime));
                roundtrip.add_stats(postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
                dds_result = PP_seq_reader.ReturnLoan(ref PP_seq_dataList, ref infoList);
                //assert (dds_result == RETCODE_OK.value);
            }
            else
            {
                System.Console.WriteLine("PING: PING_seq triggered, but no data available");
            }

            return result;
        }

        private Boolean PP_string_handler(int nof_cycles)
        {
            DDS.SampleInfo[] infoList = null;
            int amount;
            Boolean result = false;
            DDS.ReturnCode dds_result;

            /* System.out.println "PING: PING_string arrived"); */

            preTakeTime.timeGet();
            dds_result = PP_string_reader.Take(ref PP_string_dataList, ref infoList,
                    DDS.Length.Unlimited, DDS.SampleStateKind.Any,
                    DDS.ViewStateKind.Any, DDS.InstanceStateKind.Any);
            //assert (dds_result == RETCODE_OK.value);
            postTakeTime.timeGet();

            amount = PP_string_dataList.Length;
            if (amount != 0)
            {
                if (amount > 1)
                {
                    System.Console.WriteLine("PING: Ignore excess messages : " + amount
                            + " msg received");
                }
                PP_string_dataList[0].count++;
                if (PP_string_dataList[0].count < nof_cycles)
                {
                    preWriteTime.timeGet();
                    dds_result = PP_string_writer.Write(
                            PP_string_dataList[0], DDS.InstanceHandle.Nil);
                    //assert (dds_result == RETCODE_OK.value);
                    postWriteTime.timeGet();
                    write_access.add_stats(postWriteTime.sub(preWriteTime));
                }
                else
                {
                    result = true;
                }
                read_access.add_stats(postTakeTime.sub(preTakeTime));
                roundtrip.add_stats(postTakeTime.sub(roundTripTime));
                roundTripTime.set(preWriteTime.get());
                dds_result = PP_string_reader.ReturnLoan(ref PP_string_dataList,
                        ref infoList);
                //assert (dds_result == RETCODE_OK.value);
            }
            else
            {
                System.Console.WriteLine("PING: PING_string triggered, but no data available");
            }
            return result;
        }
    }
}
