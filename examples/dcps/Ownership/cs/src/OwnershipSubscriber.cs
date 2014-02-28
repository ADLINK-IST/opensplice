using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;
using DDS.OpenSplice.CustomMarshalers;
using OwnershipData;
using DDSAPIHelper;

namespace OwnershipSubscriber
{
    class OwnershipSubscriber
    {
        static void Main(string[] args)
        {
            DDSEntityManager mgr = new DDSEntityManager("Ownership");
            String partitionName = "Ownership example";

            // create Domain Participant
            mgr.createParticipant(partitionName);

            // create Type
            StockTypeSupport msgTS = new StockTypeSupport();
            mgr.registerType(msgTS);

            // create Topic
            mgr.createTopic("OwnershipStockTracker");
            
            // create Subscriber
            mgr.createSubscriber();

            // create DataReader
            mgr.createReader(false);

            // Read Events
            IDataReader dreader = mgr.getReader();
            StockDataReader OwnershipDataReader = dreader as StockDataReader;

            Stock[] msgSeq = null;
            DDS.SampleInfo[] infoSeq = null;
            
            Console.WriteLine("===[Subscriber] Ready ...");
            Console.WriteLine("   Ticker   Price   Publisher   ownership strength");

            Boolean terminate = false;
            ReturnCode status = ReturnCode.Error;
            int count = 0;
            while (!terminate && count < 1500)
            {
                OwnershipDataReader.Take(ref msgSeq, ref infoSeq, Length.Unlimited, SampleStateKind.Any, 
                                          ViewStateKind.Any, InstanceStateKind.Any);
                for (int i = 0; i < msgSeq.Length; i++)
                {
                    if (infoSeq[i].ValidData)
                    {
                        if (msgSeq[i].price < -0.0f)
                        {
                            terminate = true;
                            break;
                        }
                        Console.WriteLine("   {0} {1}   {2}     {3}", msgSeq[i].ticker,
                                          msgSeq[i].price, msgSeq[i].publisher, msgSeq[i].strength);
                    }
                }
                status = OwnershipDataReader.ReturnLoan(ref msgSeq, ref infoSeq);
                ErrorHandler.checkStatus(status, "StockDataReader.ReturnLoan");
                Thread.Sleep(2);
                ++count;
                Thread.Sleep(200);
            }
            Console.WriteLine("===[Subscriber] Market Closed");

            // clean up
            mgr.getSubscriber().DeleteDataReader(OwnershipDataReader);
            mgr.deleteSubscriber();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
