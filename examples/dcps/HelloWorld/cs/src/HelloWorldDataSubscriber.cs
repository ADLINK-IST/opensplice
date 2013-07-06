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
