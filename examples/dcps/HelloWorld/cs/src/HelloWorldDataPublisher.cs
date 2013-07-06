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
