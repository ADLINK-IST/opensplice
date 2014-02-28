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
 * LOGICAL_NAME:    ContentFilteredTopicDataSubscriber.cs
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ***********************************************************************/
using System;
using System.Threading;

using DDS;
using DDSAPIHelper;
using StockMarket;

namespace ContentFilteredSubscriber
{
    class ContentFilteredSubscriber
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("*** [ContentFilteredTopicDataSubscriber] Subscription string not specified");
                Console.WriteLine("*** usage : ContentFilteredTopicDataSubscriber <subscription_string>");
            }
            else
            {
                ITopic topic;
                DDSEntityManager mgr = new DDSEntityManager("ContentFilteredTopic");
                String partitionName = "ContentFilteredTopic example";

                // Create DomainParticipant
                mgr.createParticipant(partitionName);

                // Create Type
                StockTypeSupport msgTS = new StockTypeSupport();
                mgr.registerType(msgTS);

                // Create Topic
                topic = mgr.createTopic("StockTrackerExclusive");

                // ContentFilteredTopic
                mgr.createContentFilter("MyStockTopic", topic, args[0]);
                Console.WriteLine("=== [ContentFilteredTopicDataSubscriber] Subscription filter : ticker = '{0}'", args[0]);
                // create Subscriber
                mgr.createSubscriber();

                // create DataReader
                mgr.createReader(true);

                // Read Events
                IDataReader dreader = mgr.getReader();
                StockDataReader WaitSetReader = dreader as StockDataReader;

                // Create WaitSet
                IWaitSet w = new WaitSet();

                // Status Condition
                IStatusCondition status;
                status = dreader.StatusCondition;
                status.SetEnabledStatuses(StatusKind.DataAvailable);

                ReturnCode result = ReturnCode.Error;
                result = w.AttachCondition(status);
                ErrorHandler.checkHandle(status, "DataReader.StatusCondition");
                ErrorHandler.checkStatus(result, "WaitSet.AttachCondition");

                DDS.ICondition[] conditionList;
                DDS.SampleInfo[] infoSeq;
                Stock[] stockSeq;

                Console.WriteLine("=== [ContentFilteredTopicDataSubscriber] Ready ...");

                bool terminate = false;
                int count = 0;
                while (!terminate && count < 1500)
                {
                    /**
                     * Wait until data will be available
                     */
                    Duration wait_timeout = new Duration(Duration.InfiniteSec, Duration.InfiniteNanoSec);
                    conditionList = null;
                    result = w.Wait(ref conditionList, wait_timeout);
                    ErrorHandler.checkStatus(result, "WaitSet.Wait");

                    stockSeq = null;
                    infoSeq = null;

                    /**
                     * Read Data
                     */
                    result = WaitSetReader.Take(ref stockSeq, ref infoSeq, Length.Unlimited, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                    ErrorHandler.checkStatus(result, "StockDataReader.Take");

                    /**
                     * Display Data
                     */
                    for (int i = 0; i < stockSeq.Length; i++)
                    {
                        if (infoSeq[i].ValidData)
                        {
                            if (stockSeq[i].price == -1)
                            {
                                // We read the last expected sample => exit
                                terminate = true;
                                break;
                            }
                            Console.WriteLine("=== [ContentFilteredTopicDataSubscriber] receives stockQuote :  ( {0} , {1} )",
                                              stockSeq[i].ticker, stockSeq[i].price);
                        }
                    }
                    result = WaitSetReader.ReturnLoan(ref stockSeq, ref infoSeq);
                    ErrorHandler.checkStatus(result, "StockDataReader.ReturnLoan");
                    Thread.Sleep(200);
                    ++count;
                }

                Console.WriteLine("=== [ContentFilteredTopicDataSubscriber] Market Closed");

                // clean up
                mgr.getSubscriber().DeleteDataReader(WaitSetReader);
                mgr.deleteSubscriber();
                mgr.deleteContentFilteredTopic();
                mgr.deleteTopic();
                mgr.deleteParticipant();
            }
        }
    }
}
