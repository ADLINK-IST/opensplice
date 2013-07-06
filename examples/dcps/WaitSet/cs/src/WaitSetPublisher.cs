/*************************************************************************
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
            msgInstance.message = "First hello";

            Console.WriteLine("=== [Publisher] writing a message containing :");
            Console.WriteLine("    userID  : {0}", msgInstance.userID);
            Console.WriteLine("    Message : \" {0} ", msgInstance.message);

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
