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

/************************************************************************
 * LOGICAL_NAME:    DurabilityDataSubscriber.cs
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation of the Subscriber for the 'Durability' example.
 *
 ***/
using System;
using System.Threading;

using DDS;
using DDS.OpenSplice;
using DurabilityData;
using DDSAPIHelper;
using System.IO;

namespace DurableSubscriber
{
    class DurableSubscriber
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Insufficient number of arguments.");
                usage();
            }
            else
            {
                DDSEntityManager mgr = new DDSEntityManager("Durability");
                String partitionName = "Durability Example";
                DDS.Duration timeout = new DDS.Duration(40, 0);
                ReturnCode status;

                String durabilityKind = args[0];

                // create Domain Participant
                mgr.createParticipant(partitionName);

                // set the durability kind
                mgr.setDurabilityKind(durabilityKind);

                // create Type
                MsgTypeSupport msgTS = new MsgTypeSupport();
                mgr.registerType(msgTS);

                // create Topic
                if (args[0].Equals("persistent")) {
                    mgr.createTopic("PersistentCSDurabilityData_Msg");
                } else {
                    mgr.createTopic("CSDurabilityData_Msg");
                }

                // create Subscriber
                mgr.createSubscriber();
                mgr.createReader(false);

                IDataReader dreader = mgr.getReader();
                MsgDataReader DurabilityDataReader = dreader as MsgDataReader;

				status = DurabilityDataReader.WaitForHistoricalData(timeout);
                ErrorHandler.checkStatus(status, "DurabilityDataReader.WaitForHistoricalData");

                Msg[] msgSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                Boolean terminate = false;
                Console.WriteLine("=== [Subscriber] Ready ...");
                while (!terminate)
                {
                    status = DurabilityDataReader.Take(ref msgSeq, ref infoSeq, Length.Unlimited, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                    ErrorHandler.checkStatus(status, "DataReader.Take");
                    for (int i = 0; i < msgSeq.Length; i++)
                    {
                        if (infoSeq[i].ValidData)
                        {
                            Console.WriteLine(msgSeq[i].content);
                            if (msgSeq[i].content.Equals("9"))
                            {
                                terminate = true;
                                break;
                            }
                        }
                    }
                    status = DurabilityDataReader.ReturnLoan(ref msgSeq, ref infoSeq);
                    ErrorHandler.checkStatus(status, "MsgDataReader.ReturnLoan");
                }

                // For single process mode wait some time to ensure persistent data is stored to disk
                Thread.Sleep(2000);

                // clean up
                mgr.getSubscriber().DeleteDataReader(DurabilityDataReader);
                mgr.deleteSubscriber();
                mgr.deleteTopic();
                mgr.deleteParticipant();
            }
        }

        private static void usage()
        {
            Console.WriteLine("*** Error***");
            Console.WriteLine("*** Usage: DurableSubscriber <durability_kind> ");
            Console.WriteLine("*** durability_kind [transient | persistent] ");
        }
    }
}
