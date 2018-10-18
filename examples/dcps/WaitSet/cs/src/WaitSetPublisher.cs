/*************************************************************************
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
 * LOGICAL_NAME:    WaitSetDataPublisher.cs
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'WaitSetDataPublisher' executable.
 * 
 ***/
using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using WaitSetData;
using DDSAPIHelper;

namespace WaitSetPublisher
{
    class WaitSetPublisher
    {
        static void Main(string[] args)
        {
            DDSEntityManager mgr = new DDSEntityManager("WaitSet");
            String partitionName = "WaitSet example";

            // create Domain Participant
            mgr.createParticipant(partitionName);

            // create Type
            MsgTypeSupport msgTS = new MsgTypeSupport();
            mgr.registerType(msgTS);

            // create Topic
            mgr.createTopic("WaitSetData_Msg");

            // create Publisher
            mgr.createPublisher();

            // create DataWriter
            mgr.createWriter();

            // Publish Events

            IDataWriter dwriter = mgr.getWriter();
            MsgDataWriter WaitSetWriter = dwriter as MsgDataWriter;

            // Write the first message
            Msg msgInstance = new Msg();
            msgInstance.userID = 1;
            msgInstance.message = "First Hello";

            Console.WriteLine("=== [Publisher] writing a message containing :");
            Console.WriteLine("    userID  : {0}", msgInstance.userID);
            Console.WriteLine("    Message : \"{0} ", msgInstance.message);

            ReturnCode status = WaitSetWriter.Write(msgInstance, InstanceHandle.Nil);
            ErrorHandler.checkStatus(status, "MsgDataWriter.Write");

            Thread.Sleep(500);

            // Write another message
            msgInstance.message = "Hello again";
            status = WaitSetWriter.Write(msgInstance, InstanceHandle.Nil);
            ErrorHandler.checkStatus(status, "MsgDataWriter.Write");

            Console.WriteLine("=== [Publisher] writing a message containing :");
            Console.WriteLine("    userID  : {0}", msgInstance.userID);
            Console.WriteLine("    Message : {0}", msgInstance.message);
            Thread.Sleep(500);

            // Clean up
            mgr.getPublisher().DeleteDataWriter(WaitSetWriter);
            mgr.deletePublisher();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
