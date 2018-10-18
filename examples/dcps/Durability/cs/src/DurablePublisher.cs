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
 * LOGICAL_NAME:    DurabilityDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation of the Publisher for the 'Durability' example.
 *
 ***/
using System;
using System.Threading;

using DDS;
using DDS.OpenSplice;
using DurabilityData;
using DDSAPIHelper;
using System.IO;


namespace DurablePublisher
{
    class DurablePublisher
    {
        static void Main(string[] args)
        {
            if(args.Length != 3)
            {
                Console.WriteLine("Insufficient number of arguments.");
                usage();
            }
            else
            {
                String durabilityKind = args[0];
                Boolean autodisposeFlag = Boolean.Parse(args[1].ToString());
                Boolean automaticFlag = Boolean.Parse(args[2].ToString());

                DDSEntityManager mgr = new DDSEntityManager("Durability");
                String partitionName = "Durability example";

                // Set the Durability Kind
                mgr.setDurabilityKind(durabilityKind);

                // Set the auto dispose flag
                mgr.setAutoDispose(autodisposeFlag);

                Thread.Sleep(1000);

                // create Domain Participant
                mgr.createParticipant(partitionName);

                // create Type
                MsgTypeSupport stkTS = new MsgTypeSupport();
                mgr.registerType(stkTS);

                // create Topic
                if (args[0].Equals("persistent")) {
                    mgr.createTopic("PersistentCSDurabilityData_Msg");
                } else {
                    mgr.createTopic("CSDurabilityData_Msg");
                }

                // create Publisher
                mgr.createPublisher();

                // create DataWriter
                mgr.createWriter();

                // Publish Events
                IDataWriter dwriter = mgr.getWriter();
                MsgDataWriter msgWriter = dwriter as MsgDataWriter;

                Msg [] DurabilityDataMsg = new Msg[10];
                InstanceHandle [] handle = new InstanceHandle [10];

                ReturnCode status = ReturnCode.Error;

                for (int x = 0; x < 10; x++)
                {
                    DurabilityDataMsg[x] = new Msg();
                    DurabilityDataMsg[x].id = x;
                    DurabilityDataMsg[x].content = x.ToString();

                    Console.WriteLine(DurabilityDataMsg[x].content);

                    handle[x] = msgWriter.RegisterInstance(DurabilityDataMsg[x]);
                    ErrorHandler.checkHandle(handle[x], "DataWriter.RegisterInstance");
                    status = msgWriter.Write(DurabilityDataMsg[x], handle[x]);
                    ErrorHandler.checkStatus(status, "DataWriter.Write");
                }

                if (!automaticFlag)
                {
                    char c = (char)0;
                    Console.WriteLine("Enter E to exit");
                    while (c != 'E')
                    {
                        c = (char)Console.Read();
                    }
                }
                else
                {
                    //Console.WriteLine("=== sleeping 20s...");
                    Thread.Sleep(30000);
                }


                // Clean up
                status = mgr.getPublisher().DeleteDataWriter(msgWriter);
                ErrorHandler.checkStatus(status, "Publisher.DeleteDatWriter");
                mgr.deletePublisher();
                mgr.deleteTopic();
                mgr.deleteParticipant();
            }
        }

        private static void usage()
        {
            Console.WriteLine("*** Error***");
            Console.WriteLine("*** Usage: DurabilityDataPublisher <durability_kind> <autodispose_flag> <automatic_flag>");
            Console.WriteLine("*** durability_kind [transient | persistent] ");
            Console.WriteLine("*** autodispose_flag [false | true] ");
            Console.WriteLine("*** automatic_flag  [false | true]");
        }
    }
}
