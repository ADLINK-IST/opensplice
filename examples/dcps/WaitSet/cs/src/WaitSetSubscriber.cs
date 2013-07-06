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
 * LOGICAL_NAME:    WaitSetSubscriber.cs
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'WaitSetSubscriber' executable.
 *
 ***/
using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using WaitSetData;
using DDSAPIHelper;

namespace WaitSetSubscriber
{
    class WaitSetSubscriber
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

            // create Subscriber
            mgr.createSubscriber();

            // create DataReader
            mgr.createReader(false);

            // Read Events

            IDataReader dreader = mgr.getReader();
            MsgDataReader WaitSetReader = dreader as MsgDataReader;

            // Create a WaitSet
            WaitSet w = new WaitSet();

            // Create a ReadCondition
            IReadCondition readCond = WaitSetReader.CreateReadCondition(SampleStateKind.NotRead, ViewStateKind.New, InstanceStateKind.Alive);
            ErrorHandler.checkHandle(readCond, "DataReader.CreateReadCondition");

            // Create a QueryCondition
            String[] queryStr = { "Hello again" };
            Console.WriteLine("=== [WaitSetDataSubscriber] Query : message = \"Hello again");
            IQueryCondition queryCond = WaitSetReader.CreateQueryCondition("message=%0", queryStr);
            ErrorHandler.checkHandle(queryCond, "DataReader.CreateQueryCondition");

            // Obtain a StatusCondition
            IStatusCondition statusCond;
            statusCond = dreader.StatusCondition;
            statusCond.SetEnabledStatuses(StatusKind.LivelinessChanged);
            ErrorHandler.checkHandle(statusCond, "DataReader.StatusCondition");

            GuardCondition escape;
            escape = new GuardCondition();

            ReturnCode result;
            result = w.AttachCondition(statusCond);
            ErrorHandler.checkStatus(result, "WaitSet.AttachCondition (status)");
            result = w.AttachCondition(readCond);
            ErrorHandler.checkStatus(result, "WaitSet.AttachCondition (read)");
            result = w.AttachCondition(queryCond);
            ErrorHandler.checkStatus(result, "WaitSet.AttachCondition (query)");
            result = w.AttachCondition(escape);
            ErrorHandler.checkStatus(result, "WaitSet.AttachCondition (guard)");

            /* Initialize and pre-allocate the GuardList used to obtain
               the triggered Conditions. */
            ICondition[] guardList = new ICondition[4];

            DDS.SampleInfo[] infoSeq = null;
            Msg[] msgSeq = null;

            bool escaped = false;
            int prevCount = 0;
            int count = 0;
            Console.WriteLine("=== [WaitSetDataSubscriber] Ready ...");
            while (!escaped && count < 20 )
            {
                /**
                 * Wait until data will be available
                 */
                Duration wait_timeout = new Duration(Duration.InfiniteSec, Duration.InfiniteNanoSec);
                result = w.Wait(ref guardList, wait_timeout);
                ErrorHandler.checkStatus(result, "WaitSet.Wait");

                /* Walk over all guards to display information */
                foreach (ICondition guard in guardList)
                {
                    if (guard == readCond)
                    {
                        result = WaitSetReader.ReadWithCondition(ref msgSeq, ref infoSeq,
                                                                 Length.Unlimited, readCond);
                        ErrorHandler.checkStatus(result, "WaitSetReader.ReadWithCondition");
                        foreach (Msg msg in msgSeq)
                        {
                            Console.WriteLine("   --- New message received ---   ");
                            Console.WriteLine("   userID: {0}", msg.userID);
                            Console.WriteLine("   Message :  \"{0}", msg.message);
                        }
                        result = WaitSetReader.ReturnLoan(ref msgSeq, ref infoSeq);
                        ErrorHandler.checkStatus(result, "WaitSet.ReturnLoan");
                    }
                    else if (guard == queryCond)
                    {
                        result = WaitSetReader.TakeWithCondition(ref msgSeq, ref infoSeq,
                                                                 Length.Unlimited, queryCond);
                        ErrorHandler.checkStatus(result, "WaitSetReader.TakeWithCondition");
                        foreach (Msg msg in msgSeq)
                        {
                            Console.WriteLine("   --- New message received with QueryCondition ---   ");
                            Console.WriteLine("   userID: {0}", msg.userID);
                            Console.WriteLine("   Message : \" {0}", msg.message);
                        }
                        result = WaitSetReader.ReturnLoan(ref msgSeq, ref infoSeq);
                        ErrorHandler.checkStatus(result, "WaitSet.ReturnLoan");
                    }
                    else if (guard == statusCond)
                    {
                        LivelinessChangedStatus livelinessStatus = new LivelinessChangedStatus();
                        result = WaitSetReader.GetLivelinessChangedStatus(ref livelinessStatus);
                        ErrorHandler.checkStatus(result, "DataReader.getLivelinessChangedStatus");
                        if (livelinessStatus.AliveCount < prevCount)
                        {
                            Console.WriteLine("!!! A WaitSetDataWriter lost its liveliness");
                            Console.WriteLine("Triggering escape condition.");
                            ReturnCode status = escape.SetTriggerValue(true);
                        }
                        else
                        {
                            Console.WriteLine("!!! A WaitSetDataWriter joined");
                        }
                        prevCount = livelinessStatus.AliveCount;
                    }
                    else if (guard == escape)
                    {
                        Console.WriteLine("WaitSetSubscriber has terminated.");
                        escaped = true;
                        ReturnCode status = escape.SetTriggerValue(false);
                    }
                    else
                    {
                        // Unknown Condition
                    }
                }
                ++count;
            }

            // Detach all Conditions from the WaitSet
            result = w.DetachCondition(escape);
            ErrorHandler.checkStatus(result, "WaitSet.DetachCondition (escape)");
            result = w.DetachCondition(readCond);
            ErrorHandler.checkStatus(result, "WaitSet.DetachCondition (readCond)");
            result = w.DetachCondition(queryCond);
            ErrorHandler.checkStatus(result, "WaitSet.DetachCondition (queryCond)");
            result = w.DetachCondition(statusCond);
            ErrorHandler.checkStatus(result, "WaitSet.DetachCondition (statusCond)");

            Console.WriteLine("=== [Subscriber] Closed");
            // clean up
            WaitSetReader.DeleteReadCondition(readCond);
            WaitSetReader.DeleteReadCondition(queryCond);
            mgr.getSubscriber().DeleteDataReader(WaitSetReader);
            mgr.deleteSubscriber();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
