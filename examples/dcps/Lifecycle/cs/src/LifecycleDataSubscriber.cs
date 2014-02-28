using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using LifecycleData;
using DDSAPIHelper;

namespace LifecycleDataSubscriber
{
    class LifecycleDataSubscriber
    {
        static int index(int i)
        {
            int j = (int)(Math.Log10((double)i) / Math.Log10((double)2));
            return j;
        }
        static void Main(string[] args)
        {
            String[] sSampleState = { "READ_SAMPLE_STATE", "NOT_READ_SAMPLE_STATE" };
            String[] sViewState = { "NEW_VIEW_STATE", "NOT_NEW_VIEW_STATE" };
            String[] sInstanceState = { "ALIVE_INSTANCE_STATE", "NOT_ALIVE_DISPOSED_INSTANCE_STATE", "NOT_ALIVE_NO_WRITERS_INSTANCE_STATE" };

            bool closed = false;
            int nbIter = 1;
            int nbIterMax = 100;

            ReturnCode status = ReturnCode.Error;
            
            Msg[] msgList = null;
            SampleInfo[] infoSeq = null;
            
            DDSEntityManager mgr = new DDSEntityManager("Lifecycle");

            // Create domain participant
            String partitionName = "Lifecycle example";
            mgr.createParticipant(partitionName);          

            // Create type
            MsgTypeSupport mt = new MsgTypeSupport();
            mgr.registerType(mt);

            // Create Topic
            String topicName = "Lifecycle_Msg";
            mgr.createTopic(topicName);

            // Create Subscriber
            mgr.createSubscriber();
            mgr.createReader(false);

            IDataReader dreader = mgr.getReader();
            MsgDataReader LifecycleReader = dreader as MsgDataReader;
            ErrorHandler.checkHandle(LifecycleReader, "MsgDataReader narrow");

            Console.WriteLine("=== [Subscriber] Ready ...");

            while (!closed && (nbIter < nbIterMax))
            {
                status = LifecycleReader.Read(ref msgList, ref infoSeq, Length.Unlimited, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                ErrorHandler.checkStatus(status, "MsgDataReader.Read");

                for (int j = 0; j < msgList.Length; j++)
                {
                    Console.WriteLine(" Message        : {0}", msgList[j].message);
                    Console.WriteLine(" writerStates   : {0}", msgList[j].writerStates);
                    Console.WriteLine(" valida data    : {0}", infoSeq[j].ValidData);
                    string str = "sample_state:" + sSampleState[index((int)infoSeq[j].SampleState)] + "-view_state:" + sViewState[index((int)infoSeq[j].ViewState)] + "-instance_state:" + sInstanceState[index((int)infoSeq[j].InstanceState)];
                    Console.WriteLine(str);
                    Thread.Sleep(200);
                    closed = msgList[j].writerStates.Equals("STOPPING_SUBSCRIBER");
                }
                status = LifecycleReader.ReturnLoan(ref msgList, ref infoSeq);
                ErrorHandler.checkStatus(status, "MsgDataReader.ReturnLoan");
                Thread.Sleep(20);
                nbIter++;
            }

            Console.WriteLine("=== [Subscriber] stopping after {0} iterations - closed={1}", nbIter, closed.ToString());
            if (nbIter == nbIterMax)
                Console.WriteLine("*** Error : max {0} iterations reached", nbIterMax);

            // Clean-up         
            mgr.getSubscriber().DeleteDataReader(LifecycleReader);
            mgr.deleteSubscriber();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
