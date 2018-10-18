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
 * LOGICAL_NAME:    HelloWorldDataPublisher.cs
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

namespace HelloWorldDataPublisher
{
    class HelloWorldDataPublisher
    {
        static void Main(string[] args)
        {
            DDSEntityManager mgr = new DDSEntityManager("HelloWorld");
            String partitionName = "HelloWorld example";

            // create Domain Participant
            mgr.createParticipant(partitionName);
            mgr.setAutoDispose(false);

            // create Type
            MsgTypeSupport msgTS = new MsgTypeSupport();
            mgr.registerType(msgTS);

            // create Topic
            mgr.createTopic("HelloWorldData_Msg");

            // create Publisher
            mgr.createPublisher();

            // create DataWriter
            mgr.createWriter();

            // Publish Events
            IDataWriter dwriter = mgr.getWriter();
            MsgDataWriter helloWorldWriter = dwriter as MsgDataWriter;

            Msg msgInstance = new Msg();
            msgInstance.userID = 1;
            msgInstance.message = "Hello World";

            InstanceHandle handle = helloWorldWriter.RegisterInstance(msgInstance);
            ErrorHandler.checkHandle(handle, "MsgDataWriter.RegisterInstance");

            Console.WriteLine("=== [Publisher] writing a message containing :");
            Console.WriteLine("    userID  : {0}", msgInstance.userID);
            Console.WriteLine("    Message : \" {0} \"", msgInstance.message);
            ReturnCode status = helloWorldWriter.Write(msgInstance, handle);
            ErrorHandler.checkStatus(status, "MsgDataWriter.Write");

            try
            {
                Thread.Sleep(2);
            }
            catch (ArgumentOutOfRangeException ex)
            {
                Console.WriteLine(ex.ToString());
                Console.WriteLine(ex.StackTrace);
            }

            status = helloWorldWriter.UnregisterInstance(msgInstance, handle);

            // Clean up
            mgr.getPublisher().DeleteDataWriter(helloWorldWriter);
            mgr.deletePublisher();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
