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

            WriterState[] WriterStateList = null;
            SampleInfo[] ws_infoSeq = null;
            Msg[] msgList = null;
            SampleInfo[] infoSeq = null;

            //------------------ WriterState topic --------------------//
            DDSEntityManager mgrWs = new DDSEntityManager();

            // Create domain participant
            String partitionName = "Lifecycle";
            mgrWs.createParticipant(partitionName);

            // Create Type
            WriterStateTypeSupport wst = new WriterStateTypeSupport();
            mgrWs.registerType(wst);

            // Create Topic
            String wsTopicName = "WriterState_Msg";
            mgrWs.createTopic(wsTopicName, "Lifecycle");

            // Create Subscriber
            mgrWs.createSubscriber();

            // Create DataReader
            mgrWs.createReader("Lifecycle", false);

            IDataReader dwsReader = mgrWs.getReader();
            WriterStateDataReader WriterStateReader = dwsReader as WriterStateDataReader;
            ErrorHandler.checkHandle(WriterStateReader, "WriterStateDataReader narrow");

            //------------------ Msg topic --------------------//
            DDSEntityManager mgr = new DDSEntityManager();

            // Create domain participant
            mgr.createParticipant(partitionName);

            // Create type
            MsgTypeSupport mt = new MsgTypeSupport();
            mgr.registerType(mt);

            // Create Topic
            String topicName = "Lifecycle_Msg";
            mgr.createTopic(topicName, "Lifecycle");

            // Create Subscriber
            mgr.createSubscriber();
            mgr.createReader("Lifecycle", false);

            IDataReader dreader = mgr.getReader();
            MsgDataReader LifecycleReader = dreader as MsgDataReader;
            ErrorHandler.checkHandle(WriterStateReader, "MsgDataReader narrow");

            Console.WriteLine("=== [Subscriber] Ready ...");

            bool closed = false;
            bool writerStateChg = true;
            String sWriterState = "";
            ReturnCode status = ReturnCode.Error;

            while (!closed)
            {
                // WriterState topic
                status = WriterStateReader.Take(ref WriterStateList, ref ws_infoSeq, Length.Unlimited, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                ErrorHandler.checkStatus(status, "WriterStateReader.Take");

                for (int j = 0; j < WriterStateList.Length; j++)
                {
                    if (ws_infoSeq[j].ValidData)
                    {
                        Console.WriteLine("--- WriterState : {0}", WriterStateList[j].state);
                        closed = WriterStateList[j].state.Equals("STOPPING SUBSCRIBER");
                        writerStateChg = sWriterState.Equals(WriterStateList[j].state);
                        sWriterState = WriterStateList[j].state;
                        Thread.Sleep(200);
                    }
                }
                status = WriterStateReader.ReturnLoan(ref WriterStateList, ref ws_infoSeq);
                ErrorHandler.checkStatus(status, "WriterStateDataReader.ReturnLoan");
                Thread.Sleep(2);

                // Lifecycle Topic
                if (writerStateChg)
                {
                    status = LifecycleReader.Read(ref msgList, ref infoSeq, Length.Unlimited, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                    ErrorHandler.checkStatus(status, "MsgDataReader.Read");

                    for (int j = 0; j < msgList.Length; j++)
                    {
                        Console.WriteLine(" Message        : {0}", msgList[j].message);
                        Console.WriteLine(" valida data    : {0}", infoSeq[j].ValidData);
                        Console.WriteLine(" sample_state   : {0}", sSampleState[index((int)infoSeq[j].SampleState)]);
                        Console.WriteLine(" view_state     : {0}", sViewState[index((int)infoSeq[j].ViewState)]);
                        Console.WriteLine(" instance_state : {0}", sInstanceState[index((int)infoSeq[j].InstanceState)]);
                        Thread.Sleep(200);
                    }
                    status = LifecycleReader.ReturnLoan(ref msgList, ref infoSeq);
                    ErrorHandler.checkStatus(status, "MsgDataReader.ReturnLoan");
                    Thread.Sleep (2);
                    writerStateChg = false;
                }
            }

            // Clean-up Msg Entities
            mgr.getSubscriber().DeleteDataReader(LifecycleReader);
            mgr.deleteSubscriber();
            mgr.deleteTopic();
            mgr.deleteParticipant();

            // Clean-up WriterState Entities
            mgrWs.getSubscriber().DeleteDataReader(WriterStateReader);
            mgrWs.deleteSubscriber();
            mgrWs.deleteTopic();
            mgrWs.deleteParticipant();
        }
    }
}
