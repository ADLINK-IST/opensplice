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
            Console.WriteLine("*** Usage: LifeCycleDataPublisher <autodispose_flag> [-action <action_number>]");
            Console.WriteLine("***        autodispose_flag = false | true");
            Console.WriteLine("***        -action_number   = 1 | 2 | 3 | 4");
            
            Environment.Exit(-1);
        }

        static void Main(string[] args)
        {
            char choice = ' ';
            bool interactive = true;
            bool autodispose_flag = false;

            if (args.Length < 1)
                usage();
            autodispose_flag = Boolean.Parse(args[0]);

            if (args.Length > 1)
            {
                if (!(args[1].Equals("-action")))
                    usage();
                if (args.Length < 2)
                    usage();
                choice = Char.Parse(args[2]);
                if ((!choice.Equals('1')) && (!choice.Equals('2')) && (!choice.Equals('3')) && (!choice.Equals('4')))
                    usage();
                interactive = false;
                Console.WriteLine("Choice : {0}", choice);
            }

            // DDS Helper class instances for WriterState & Msg Topic
            DDSEntityManager mgrWs = new DDSEntityManager();
            DDSEntityManager mgr = new DDSEntityManager();            

            // Create domain participant
            String partitionName = "Lifecycle";
            mgrWs.createParticipant(partitionName);

            // Create Type
            WriterStateTypeSupport wst = new WriterStateTypeSupport();
            mgrWs.registerType(wst);

            // Create Topic
            String wsTopicName = "WriterState_Msg";
            mgrWs.createTopic(wsTopicName, "Lifecycle");

            // Create Publisher
            mgrWs.createPublisher();

            // Create DataWriter
            mgrWs.createWriter();

            // Publish Samples
            IDataWriter dws_writer = mgrWs.getWriter();
            WriterStateDataWriter WriterStateWriter = dws_writer as WriterStateDataWriter;

            //-------------------Msg Topic-------------------------//
            
            // Set AutoDispose Flag
            mgr.setAutoDispose(autodispose_flag);

            // Create Participant
            mgr.createParticipant(partitionName);

            // Create type
            MsgTypeSupport mt = new MsgTypeSupport();
            mgr.registerType(mt);

            // Create Topic
            String topicName = "Lifecycle_Msg";
            mgr.createTopic(topicName, "Lifecycle");

            // Create Publisher
            mgr.createPublisher();

            // Create DataWriter
            mgr.createWriter();

            // Publish Samples
            IDataWriter dwriter = mgr.getWriter();
            MsgDataWriter LifecycleWriter = dwriter as MsgDataWriter;

            // Send MsgDataWriter state
            WriterState wsInstance = new WriterState();
            wsInstance.state = "SENDING SAMPLE";
            Console.WriteLine("===[Publisher] : ");
            Console.WriteLine("   Writer state: \" {0}", wsInstance.state, "\"");
            ReturnCode status = WriterStateWriter.Write(wsInstance, InstanceHandle.Nil);
            ErrorHandler.checkStatus(status, "WriterStateDataWriter.Write");

            // Send Msg (topic to monitor)
            Msg msgInstance = new Msg();
            msgInstance.userID = 1;
            msgInstance.message = "Hello";
            Console.WriteLine("=== [Publisher]  :");
            Console.WriteLine("    userID   : {0}", msgInstance.userID);
            Console.WriteLine("    Message  : \" {0}", msgInstance.message);
            status = LifecycleWriter.Write(msgInstance, InstanceHandle.Nil);
            ErrorHandler.checkStatus(status, "MsDataWriter.Write");

            Thread.Sleep(2000);

            // Send MsgDataWriter state
            wsInstance.state = "SAMPLE SENT";
            Console.WriteLine("===[Publisher] : ");
            Console.WriteLine("   Writer state: \" {0}", wsInstance.state, "\"");
            status = WriterStateWriter.Write(wsInstance, InstanceHandle.Nil);
            ErrorHandler.checkStatus(status, "WriterStateDataWriter.Write");

            if (interactive)
            {
                while ((choice != '1') && (choice != '2') && (choice != '3') && (choice != '4'))
                {
                    Console.WriteLine("=== Choose a character to continue : ");
                    Console.WriteLine("    1  Dispose the instance");
                    Console.WriteLine("    2  Unregister the instance");
                    Console.WriteLine("    3  Stop the Publisher");
                    Console.WriteLine("    4  Stop the Subscriber");
                    choice = Char.Parse(Console.In.ReadLine());
                }
            }

            switch (choice)
            {
                case '1' :
                    {
                        // Dispose instance
                        status = LifecycleWriter.Dispose(msgInstance, InstanceHandle.Nil);
                        ErrorHandler.checkStatus(status, "MsDataWriter.Dispose");
                        // Send MsgDataWriter state
                        wsInstance.state = "INSTANCE DISPOSED";
                        Console.WriteLine("===[Publisher] : ");
                        Console.WriteLine("   Writer state: \" {0}", wsInstance.state, "\"");
                        status = WriterStateWriter.Write(wsInstance, InstanceHandle.Nil);
                        ErrorHandler.checkStatus(status, "WriterStateDataWriter.Write");

                        if (interactive)
                        {
                            Console.Write("\n=== enter a character to continue : ");
                            choice = Char.Parse(Console.In.ReadLine());
                        }
                        break;
                    }
                case '2':
                    {
                        // Unregister instance : the autoDispose_flag is currently
                        // ignored and the instance is never disposed automatically
                        status = LifecycleWriter.UnregisterInstance(msgInstance, InstanceHandle.Nil);
                        ErrorHandler.checkStatus(status, "MsgDataWriter.UnregisterInstance");

                        // send MsgDataWriter state
                        wsInstance.state = "INSTANCE UNREGISTERED";
                        Console.WriteLine("===[Publisher] : ");
                        Console.WriteLine("   Writer state: \" {0}", wsInstance.state, "\"");
                        status = WriterStateWriter.Write(wsInstance, InstanceHandle.Nil);
                        ErrorHandler.checkStatus(status, "WriterStateDataWriter.Write");

                        if (interactive)
                        {
                            Console.Write("\n=== enter a character to continue : ");
                            choice = Char.Parse(Console.In.ReadLine());
                        }
                        break;
                    }
                case '3':
                    {
                        break;
                    }
                case '4':
                    {
                        // send MsgDataWriter state
                        wsInstance.state = "STOPPING SUBSCRIBER";
                        Console.WriteLine("===[Publisher] : ");
                        Console.WriteLine("   Writer state: \" {0}", wsInstance.state, "\"");
                        status = WriterStateWriter.Write(wsInstance, InstanceHandle.Nil);
                        ErrorHandler.checkStatus(status, "WriterStateDataWriter.Write");
                        break;
                    }
            }

            // Clean-up entities for Msg topic
            // Remove the DataWriters
            mgr.getPublisher().DeleteDataWriter(LifecycleWriter);

            // send MsgDataWriter state
            wsInstance.state = "DATAWRITER DELETED";
            Console.WriteLine("===[Publisher] : ");
            Console.WriteLine("   Writer state: \" {0}", wsInstance.state, "\"");
            status = WriterStateWriter.Write(wsInstance, InstanceHandle.Nil);
            ErrorHandler.checkStatus(status, "WriterStateDataWriter.Write");

            // Remove the Publisher
            mgr.deletePublisher();

            // Remove the Topic
            mgr.deleteTopic();

            // Remove the Participant
            mgr.deleteParticipant();

            // Clean-up entities fpr WriterState topic
            // Dispose the instance
            status = WriterStateWriter.Dispose(wsInstance, InstanceHandle.Nil);
            ErrorHandler.checkStatus(status, "WriterStateWriter.Dispose");

            // Remove the DataWriter
            mgrWs.getPublisher().DeleteDataWriter(WriterStateWriter);

            // Remove the Publisher
            mgrWs.deletePublisher();

            // Remove the Topic
            mgrWs.deleteTopic();

            // Remove Participant
            mgrWs.deleteParticipant();
        }
    }
}
