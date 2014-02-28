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
 * LOGICAL_NAME:    ContentFilteredTopicDataPublisher.cs
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'ContentFilteredTopicDataPublisher' executable.
 *
 ***/
using System;
using System.Threading;

using DDS;
using DDSAPIHelper;
using StockMarket;


namespace ContentFilteredPublisher
{
    class ContentFilteredPublisher
    {
        static void Main(string[] args)
        {
            DDSEntityManager mgr = new DDSEntityManager("ContentFilteredTopic");
            String partitionName = "ContentFilteredTopic example";

            // create Domain Participant
            mgr.createParticipant(partitionName);
            mgr.setAutoDispose(true);

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
            StockDataWriter ContentFilteredTopicDataWriter = dwriter as StockDataWriter;
            Stock geStock = new Stock();
            Stock msftStock = new Stock();
            ReturnCode writeStatus;

            geStock.ticker = "GE";
            geStock.price = 12.00f;
            msftStock.ticker = "MSFT";
            msftStock.price = 25.00f;

            InstanceHandle geHandle = ContentFilteredTopicDataWriter.RegisterInstance(geStock);
            InstanceHandle msHandle = ContentFilteredTopicDataWriter.RegisterInstance(msftStock);

            for (int i = 0; i < 20; i++)
            {
                geStock.price += 0.5f ;
                msftStock.price += 1.5f;
                Console.WriteLine("=== [ContentFilteredTopicDataPublisher] sends 2 stockQuotes : (GE, {0}) (MSFT, {1})", geStock.price, msftStock.price);
                writeStatus = ContentFilteredTopicDataWriter.Write(geStock, geHandle);
                ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Write");
                writeStatus = ContentFilteredTopicDataWriter.Write(msftStock, msHandle);
                ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Write");
                Thread.Sleep(100);
            }

            geStock.price = -1.0f;
            msftStock.price = -1.0f;
            writeStatus = ContentFilteredTopicDataWriter.Write(geStock, geHandle);
            ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Write GE");
            writeStatus = ContentFilteredTopicDataWriter.Write(msftStock, msHandle);
            ErrorHandler.checkStatus(writeStatus, "StockDataWriter.Write MSFT");

            Console.WriteLine("Market Closed");

            ContentFilteredTopicDataWriter.UnregisterInstance(geStock, geHandle);
            ContentFilteredTopicDataWriter.UnregisterInstance(msftStock, msHandle);

            mgr.getPublisher().DeleteDataWriter(ContentFilteredTopicDataWriter);
            mgr.deletePublisher();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
