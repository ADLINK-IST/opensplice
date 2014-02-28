using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using ListenerData;
using DDSAPIHelper;

namespace ListenerDataPublisher
{
    class ListenerDataPublisher
    {
        static void Main(string[] args)
        {
            DDSEntityManager mgr = new DDSEntityManager("Listener");
            String partitionName = "Listener Example";

            // create Domain Participant
            mgr.createParticipant(partitionName);

            // create Type
            MsgTypeSupport msgTS = new MsgTypeSupport();
            mgr.registerType(msgTS);

            // create Topic
            mgr.createTopic("ListenerData_Msg");

            // create Publisher
            mgr.createPublisher();

            // create DataWriter
            mgr.createWriter();

            // Publish Events
            IDataWriter dwriter = mgr.getWriter();
            MsgDataWriter listenerWriter = dwriter as MsgDataWriter;

            ReturnCode status = ReturnCode.Error;
            Msg msgInstance = new Msg();
            msgInstance.userID = 1;
            msgInstance.message = "Hello World";

            Console.WriteLine("=== [ListenerDataPublisher] writing a message containing :");
            Console.WriteLine("    userID  : {0}", msgInstance.userID);
            Console.WriteLine("    Message : \"" + msgInstance.message + "\"");

            status = listenerWriter.Write(msgInstance, InstanceHandle.Nil);
            ErrorHandler.checkStatus(status, "DataWriter.Write");

            Thread.Sleep(2000);

            // clean up
            mgr.getPublisher().DeleteDataWriter(listenerWriter);
            mgr.deletePublisher();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }
    }
}
