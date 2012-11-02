using System;
using System.IO;

using DDS;
using DDS.OpenSplice;

using ListenerData;
using DDSAPIHelper;

namespace ListenerDataSubscriber
{
    class ListenerDataListener : IDataReaderListener
    {
        public GuardCondition guardCond = new GuardCondition();        
        public bool terminated = false;        
        ReturnCode status = ReturnCode.Error;

        /* Type specific DDS entity */
        private MsgDataReader msgDR;

        public MsgDataReader MsgDR
        {
            get { return msgDR; }
            set { msgDR = value; }
        }
	
        #region IDataReaderListener Members

        public void OnDataAvailable(IDataReader entityInterface)
        {
            Msg[] msgList = null ;
            SampleInfo[] infoSeq = null;
        
            status = msgDR.Read(ref msgList, ref infoSeq, Length.Unlimited, SampleStateKind.Any, ViewStateKind.New, InstanceStateKind.Any);
            ErrorHandler.checkStatus(status, "DataReader.Read");

            if (msgList != null && msgList.Length > 0)
            {
                Console.WriteLine("=== [ListenerDataListener::OnDataAvailable] - msgList.Length : {0}", msgList.Length);
                foreach (Msg msg in msgList)
                {
                    Console.WriteLine("    --- Message Received ---");
                    Console.WriteLine("    userId : {0}", msg.userID);
                    Console.WriteLine("    message : \\ {0}",msg.message);
                }
                status = msgDR.ReturnLoan(ref msgList, ref infoSeq);
                ErrorHandler.checkStatus(status, "DataReader.ReturnLoan");
            }
            guardCond.SetTriggerValue(true);
        }

        public void OnLivelinessChanged(IDataReader entityInterface, LivelinessChangedStatus status)
        {
            Console.WriteLine("=== [ListenerDataListener::OnlivelinessChanged] triggered");
        }

        public void OnRequestedDeadlineMissed(IDataReader entityInterface, RequestedDeadlineMissedStatus status)
        {
            Console.WriteLine("=== [ListenerDataListener::OnRequestedDeadlineMissed] : triggered");
            Console.WriteLine("=== [ListenerDataListener::OnRequestedDeadlineMissed] : stopping");
            terminated = true;
            // unblock the waitset in Subscriber main loop
            guardCond.SetTriggerValue(true);
        }

        public void OnRequestedIncompatibleQos(IDataReader entityInterface, RequestedIncompatibleQosStatus status)
        {
            Console.WriteLine("=== [ListenerDataListener::OnRequestedIncompatibleQos] : triggered");            
        }

        public void OnSampleLost(IDataReader entityInterface, SampleLostStatus status)
        {
            Console.WriteLine("=== [ListenerDataListener::OnRequestedIncompatibleQos] : triggered"); 
        }

        public void OnSampleRejected(IDataReader entityInterface, SampleRejectedStatus status)
        {
            Console.WriteLine("=== [ListenerDataListener::OnSampleRejected] : triggered"); 
        }

        public void OnSubscriptionMatched(IDataReader entityInterface, SubscriptionMatchedStatus status)
        {
            Console.WriteLine("=== [ListenerDataListener::OnSubscriptionMatched] : triggered"); 
        }

        #endregion
    }
}
