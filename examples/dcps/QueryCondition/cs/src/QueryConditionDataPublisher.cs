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
