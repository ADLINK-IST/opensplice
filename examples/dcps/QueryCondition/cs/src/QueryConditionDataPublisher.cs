/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
 * LOGICAL_NAME:    QueryConditionDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'QueryConditionDataPublisher' executable.
 * 
 ***/
using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using StockMarket;
using DDSAPIHelper;


namespace QueryConditionDataPublisher
{
    class QueryConditionDataPublisher
    {
        static void Main(string[] args)
        {
            DDSEntityManager mgr = new DDSEntityManager("QueryCondition");
            String partitionName = "QueryCondition example";

            // create Domain Participant
            mgr.createParticipant(partitionName);

            // create Type
            StockTypeSupport msgTS = new StockTypeSupport();
            mgr.registerType(msgTS);

            // create Topic
            mgr.createTopic("StockTrackerExclusive");

            // create Publisher
            mgr.createPublisher();

            // create DataWriter
            mgr.createWriter();

            // Publish Events

            IDataWriter dwriter = mgr.getWriter();
            StockDataWriter QueryConditionDataWriter = dwriter as StockDataWriter;
            Stock geStock = new Stock();
            Stock msftStock = new Stock();
            ReturnCode writeStatus;

            geStock.ticker = "GE";
            geStock.price = 12.00f;
            msftStock.ticker = "MSFT";
            msftStock.price = 25.00f;

            // Register Instances
            InstanceHandle geHandle = QueryConditionDataWriter.RegisterInstance(geStock);
            ErrorHandler.checkHandle(geHandle, "DataWriter.RegisterInstance (GE)");
            InstanceHandle msHandle = QueryConditionDataWriter.RegisterInstance(msftStock);
            ErrorHandler.checkHandle(msHandle, "DataWriter.RegisterInstance (MSFT)");

            for (int i = 0; i < 20; i++)
            {
                geStock.price += 0.5f;
                msftStock.price += 1.5f;
                writeStatus = QueryConditionDataWriter.Write(geStock, InstanceHandle.Nil);
                ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Write");
                writeStatus = QueryConditionDataWriter.Write(msftStock, InstanceHandle.Nil);
                ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Write");
                Thread.Sleep(100);
                Console.WriteLine("GE : {0} MSFT {1}",String.Format("{0:0.#}", geStock.price),
                                  String.Format("{0:0.#}", msftStock.price));
            }

            geStock.price = -1.0f;
            msftStock.price = -1.0f;

            // Write samples
            writeStatus = QueryConditionDataWriter.Write(geStock, InstanceHandle.Nil);
            ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Write (GE)");
            writeStatus = QueryConditionDataWriter.Write(msftStock, InstanceHandle.Nil);
            ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Write (MS)");           

            // Dispose Instances
            writeStatus = QueryConditionDataWriter.Dispose(geStock, geHandle);
            ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Dispose (GE)");
            writeStatus = QueryConditionDataWriter.Dispose(msftStock, msHandle);
            ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Dispose (MS)");

            // Unregister Instances
            writeStatus = QueryConditionDataWriter.UnregisterInstance(geStock, geHandle);
            ErrorHandler.checkStatus(writeStatus, "StockDataWriter.UnregisterInstance (GE)");
            writeStatus = QueryConditionDataWriter.UnregisterInstance(msftStock, msHandle);
            ErrorHandler.checkStatus(writeStatus, "StockDataWriter.UnregisterInstance(MS)");

            // Clean up
            mgr.getPublisher().DeleteDataWriter(QueryConditionDataWriter);
            mgr.deletePublisher();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
