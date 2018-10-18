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
 * LOGICAL_NAME:    QueryConditionDataQuerySubscriber.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'QueryConditionDataContentSubscriber' executable.
 *
 ***/
using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using StockMarket;
using DDSAPIHelper;

namespace QueryConditionDataSubscriber
{
    class QueryConditionDataSubscriber
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Bad args number");
                Console.WriteLine("*** [QueryConditionDataQuerySubscriber] Query string not specified");
                Console.WriteLine("*** usage : QueryConditionDataQuerySubscriber <query_string>");
            }
            else
            {
                ITopic topic;
                DDSEntityManager mgr = new DDSEntityManager("QueryCondition");
                String partitionName = "QueryCondition example";
                String QueryConditionDataToSubscribe = args[0];

                // Create DomainParticipant
                mgr.createParticipant(partitionName);

                // Create Type
                StockTypeSupport msgTS = new StockTypeSupport();
                mgr.registerType(msgTS);

                // Create Topic
                topic = mgr.createTopic("StockTrackerExclusive");

                // create Subscriber
                mgr.createSubscriber();

                // create DataReader
                mgr.createReader(false);

                // Read Events
                IDataReader dreader = mgr.getReader();
                StockDataReader QueryConditionDataReader = dreader as StockDataReader;

                String[] queryStr = { QueryConditionDataToSubscribe };

                Console.WriteLine("=== [QueryConditionDataQuerySubscriber] Query : ticker = {0}", QueryConditionDataToSubscribe);
                IQueryCondition qc = QueryConditionDataReader.CreateQueryCondition(
                    SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any, "ticker=%0", queryStr);

                Console.WriteLine("=== [QueryConditionDataQuerySubscriber] Ready ...");

                DDS.SampleInfo[] infoSeq = null;
                Stock[] stockSeq = null;

                ReturnCode status = ReturnCode.Error;
                bool terminate = false;
                int count = 0;
                Console.WriteLine("=== [QueryConditionDataQuerySubscriber] Ready ...");
                while (!terminate && count < 1500)
                {
                    // Take Sample with Condition
                    status = QueryConditionDataReader.TakeWithCondition(ref stockSeq, ref infoSeq,
                        Length.Unlimited, qc);
                    ErrorHandler.checkStatus(status, "DataReader.TakeWithCondition");

                    /**
                     * Display Data
                     */
                    for (int i = 0; i < stockSeq.Length; i++)
                    {
                        if (infoSeq[i].ValidData)
                        {
                            if (stockSeq[i].price == -1.0f)
                            {
                                terminate = true;
                                break;
                            }
                            Console.WriteLine("{0} : {1}", stockSeq[i].ticker, String.Format("{0:0.#}", stockSeq[i].price));
                        }
                    }
                    status = QueryConditionDataReader.ReturnLoan(ref stockSeq, ref infoSeq);
                    ErrorHandler.checkStatus(status, "DataReader.ReturnLoan");
                    Thread.Sleep(200);
                    ++count;
                }

                Console.WriteLine("=== [QueryConditionDataQuerySubscriber] Market Closed");

                // clean up
                QueryConditionDataReader.DeleteReadCondition(qc);
                mgr.getSubscriber().DeleteDataReader(QueryConditionDataReader);
                mgr.deleteSubscriber();
                mgr.deleteTopic();
                mgr.deleteParticipant();
            }
        }
    }
}
