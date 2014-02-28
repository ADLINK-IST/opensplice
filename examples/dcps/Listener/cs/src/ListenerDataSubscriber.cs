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
 * LOGICAL_NAME:    ListenerDataSubscriber.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'Subscriber' executable.
 */

using System;
using System.Threading;

using DDS;
using DDS.OpenSplice;

using ListenerData;
using DDSAPIHelper;
using System.IO;

namespace ListenerDataSubscriber
{
    class ListenerDataSubscriber
    {
        static void Main(string[] args)
        {
            DDSEntityManager mgr = new DDSEntityManager("Listener");
            ReturnCode status = ReturnCode.Error;
            ListenerDataListener myListener;
            String partitionName = "Listener Example";
            int count = 0;
            Duration wait_timeout = new Duration(0, 200000000);

            // create Domain Participant
            mgr.createParticipant(partitionName);

            // create Type
            MsgTypeSupport msgTS = new MsgTypeSupport();
            mgr.registerType(msgTS);

            // create Topic
            mgr.createTopic("ListenerData_Msg");

            // create Subscriber
            mgr.createSubscriber();

            // create DataReader
            mgr.createReader(false);

            IDataReader dreader = mgr.getReader();

            myListener = new ListenerDataListener();
            myListener.MsgDR = dreader as MsgDataReader;

            Console.WriteLine("=== [ListenerDataSubscriber] SetListener");
            StatusKind kind = StatusKind.DataAvailable | StatusKind.RequestedDeadlineMissed;

            status = myListener.MsgDR.SetListener(myListener, kind);
            ErrorHandler.checkStatus(status, "DataReader.SetListener");

            Console.WriteLine("=== [ListenerDataSubscriber] Ready...");
            myListener.terminated = false;

            WaitSet ws = new WaitSet();
            ws.AttachCondition(myListener.guardCond);
            ICondition[] cond = null;


            while (!myListener.terminated && count < 1500)
            {
                Console.WriteLine("=== [SubscriberUsingListener] waiting waitset ...");
                ws.Wait(ref cond, wait_timeout);
                myListener.guardCond.SetTriggerValue(false);
                ++count;
            }

            Console.WriteLine("===[ListenerDataSubscriber] Market Closed.");

            mgr.getSubscriber().DeleteDataReader(myListener.MsgDR);
            mgr.deleteSubscriber();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
