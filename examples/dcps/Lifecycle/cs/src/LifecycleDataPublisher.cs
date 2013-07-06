using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using LifecycleData;
using DDSAPIHelper;

namespace LifecycleDataPublisher
{
    class LifecycleDataPublisher
    {
        static void usage()
        {
            Console.WriteLine("*** ERROR ***");
            Console.WriteLine("*** Usage: LifeCycleDataPublisher <autodispose_flag> <writer_action>");
            Console.WriteLine("***        autodispose_flag = false | true");
            Console.WriteLine("***        writer_action = dispose | unregister | stoppub");
        }

        static void Main(string[] args)
        {
            bool autodispose_flag = false;
            String writer_action;
            ReturnCode status = ReturnCode.Error;

            if (args.Length < 2)
            {
                usage();
            }
            else if ((!args[0].Equals("true")) &&
                (!args[0].Equals("false")) &&
                (!args[1].Equals("dispose")) &&
                (!args[1].Equals("unregister")) &&
                (!args[1].Equals("stoppub")))
            {
                usage();
            }
            else
            {
                autodispose_flag = Boolean.Parse(args[0]);
                writer_action = args[1];

                // DDS Helper class instance for Msg Topic
                DDSEntityManager mgr = new DDSEntityManager("Lifecycle");

                // Create domain participant
                String partitionName = "Lifecycle example";
                mgr.createParticipant(partitionName);

                // Set AutoDispose Flag
                mgr.setAutoDispose(autodispose_flag);

                // Create type
                MsgTypeSupport mt = new MsgTypeSupport();
                mgr.registerType(mt);

                // Create Topic
                String topicName = "Lifecycle_Msg";
                mgr.createTopic(topicName);

                // Create Publisher
                mgr.createPublisher();

                // Create DataWriter(s)
                mgr.createWriter();

                // Publish Samples
                IDataWriter dwriter = mgr.getWriter();
                MsgDataWriter LifecycleWriter = dwriter as MsgDataWriter;
                IDataWriter dwriter_stopper = mgr.getWriterStopper();
                MsgDataWriter LifecycleWriter_stopper = dwriter_stopper as MsgDataWriter;

                if (writer_action.Equals("dispose"))
                {
                    // Send Msg (topic to monitor)
                    Msg msgInstance = new Msg();
                    msgInstance.userID = 1;
                    msgInstance.message = "Lifecycle_1";
                    msgInstance.writerStates = "SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED";
                    Console.WriteLine("=== [Publisher]  :");
                    Console.WriteLine("    userID   : {0}", msgInstance.userID);
                    Console.WriteLine("    Message  : \"{0}\"", msgInstance.message);
                    Console.WriteLine("    writerStates  : \"{0}\"", msgInstance.writerStates);
                    status = LifecycleWriter.Write(msgInstance, InstanceHandle.Nil);
                    ErrorHandler.checkStatus(status, "MsDataWriter.Write");
                    Thread.Sleep(500);
                    Console.WriteLine("=== [Publisher]  : SAMPLE_SENT");

                    // Dispose instance
                    status = LifecycleWriter.Dispose(msgInstance, InstanceHandle.Nil);
                    ErrorHandler.checkStatus(status, "MsDataWriter.Dispose");
                    Console.WriteLine("=== [Publisher]  : INSTANCE_DISPOSED");
                }
                else if (writer_action.Equals("unregister"))
                {
                    // Send Msg (topic to monitor)
                    Msg msgInstance = new Msg();
                    msgInstance.userID = 2;
                    msgInstance.message = "Lifecycle_2";
                    msgInstance.writerStates = "SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED";
                    Console.WriteLine("=== [Publisher]  :");
                    Console.WriteLine("    userID   : {0}", msgInstance.userID);
                    Console.WriteLine("    Message  : \"{0}\"", msgInstance.message);
                    Console.WriteLine("    writerStates  : \"{0}\"", msgInstance.writerStates);
                    status = LifecycleWriter.Write(msgInstance, InstanceHandle.Nil);
                    ErrorHandler.checkStatus(status, "MsDataWriter.Write");
                    Thread.Sleep(500);
                    Console.WriteLine("=== [Publisher]  : SAMPLE_SENT");

                    // Unregister instance : the auto_dispose_unregistered_instances flag
                    // is currently ignored and the instance is never disposed automatically
                    status = LifecycleWriter.UnregisterInstance(msgInstance, InstanceHandle.Nil);
                    ErrorHandler.checkStatus(status, "MsDataWriter.UnregisterInstance");
                    Console.WriteLine("=== [Publisher]  : INSTANCE_UNREGISTERED");
                }
                else if (writer_action.Equals("stoppub"))
                {
                    Msg msgInstance = new Msg();
                    msgInstance.userID = 3;
                    msgInstance.message = "Lifecycle_3";
                    msgInstance.writerStates = "SAMPLE_SENT -> DATAWRITER_DELETED";
                    Console.WriteLine("=== [Publisher]  :");
                    Console.WriteLine("    userID   : {0}", msgInstance.userID);
                    Console.WriteLine("    Message  : \"{0}\"", msgInstance.message);
                    Console.WriteLine("    writerStates  : \"{0}\"", msgInstance.writerStates);
                    status = LifecycleWriter.Write(msgInstance, InstanceHandle.Nil);
                    ErrorHandler.checkStatus(status, "MsDataWriter.Write");
                    Thread.Sleep(500);
                    Console.WriteLine("=== [Publisher]  : SAMPLE_SENT");
                }

                // Let the subscriber treat the previous writer state !!!
                Console.WriteLine("=== [Publisher] waiting 500ms to let the subscriber treat the previous write state ...");
                Thread.Sleep(500);

                // Remove the DataWriter
                mgr.deleteWriter(LifecycleWriter);
                Thread.Sleep(500);
                Console.WriteLine("=== [Publisher]  : DATAWRITER_DELETED");

                // Stop the subscriber
                Msg stopMsg = new Msg();
                stopMsg.userID = 4;
                stopMsg.message = "Lifecycle_4";
                stopMsg.writerStates = "STOPPING_SUBSCRIBER";
                Console.WriteLine("=== [Publisher]  :");
                Console.WriteLine("    userID   : {0}", stopMsg.userID);
                Console.WriteLine("    Message  : \"{0}\"", stopMsg.message);
                Console.WriteLine("    writerStates  : \"{0}\"", stopMsg.writerStates);
                status = LifecycleWriter_stopper.Write(stopMsg, InstanceHandle.Nil);
                ErrorHandler.checkStatus(status, "MsDataWriter.Write");
                Thread.Sleep(500);

                // Remove the DataWriter stopper
                mgr.deleteWriter(LifecycleWriter_stopper);

                // Remove the Publisher
                mgr.deletePublisher();

                // Remove the Topic
                mgr.deleteTopic();

                // Remove the participant
                mgr.deleteParticipant();
            }
        }
    }
}
