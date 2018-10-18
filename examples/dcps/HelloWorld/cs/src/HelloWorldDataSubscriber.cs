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
 * LOGICAL_NAME:    HelloWorldDataPSubscriber.cs
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'HelloWorldDataPublisher' executable.
 *
 ***/
using System;
using System.Threading;

using DDS;
using DDS.OpenSplice;

using HelloWorldData;
using DDSAPIHelper;
using System.IO;

namespace HelloWorldDataSubscriber
{
    class HelloWorldDataSubscriber
    {
        static void Main(string[] args)
        {
            DDSEntityManager mgr = new DDSEntityManager("HelloWorld");
            String partitionName = "HelloWorld example";

            // create Domain Participant
            mgr.createParticipant(partitionName);

            // create Type
            MsgTypeSupport msgTS = new MsgTypeSupport();
            mgr.registerType(msgTS);

            // create Topic
            mgr.createTopic("HelloWorldData_Msg");

            // create Subscriber
            mgr.createSubscriber();

            // create DataReader
            mgr.createReader(false);

            IDataReader dreader = mgr.getReader();
            MsgDataReader HelloWorldDataReader = dreader as MsgDataReader;

            Msg[] msgSeq = null;
            DDS.SampleInfo[] infoSeq = null;
            Boolean terminate = false;
            ReturnCode status;

            Console.WriteLine("=== [Subscriber] Ready ...");
            int count = 0;
            while (!terminate && count < 1500)
            {
                status = HelloWorldDataReader.Take(ref msgSeq, ref infoSeq, Length.Unlimited, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                ErrorHandler.checkStatus(status, "DataReader.Take");
                for (int i = 0; i < msgSeq.Length; i++)
                {
                    if (infoSeq[i].ValidData)
                    {
                        Console.WriteLine("=== [Subscriber] message received :");
                        Console.WriteLine("    userID  : {0}", msgSeq[i].userID);
                        Console.WriteLine("    Message : \"" + msgSeq[i].message + "\"");

                        terminate = true;
                    }                                        
                }
                status = HelloWorldDataReader.ReturnLoan(ref msgSeq, ref infoSeq);
                ErrorHandler.checkStatus(status, "DataReader.ReturnLoan");
                Thread.Sleep(200);
                ++count;
            }

            Thread.Sleep(2);

            // clean up
            mgr.getSubscriber().DeleteDataReader(HelloWorldDataReader);
            mgr.deleteSubscriber();       
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
