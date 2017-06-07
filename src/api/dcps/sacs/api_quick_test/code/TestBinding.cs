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
 */

using System;
using DDS;
using System.Runtime.InteropServices;

using DDS.OpenSplice.CustomMarshalers;
namespace testNamespace
{
    class TestBinding : DomainParticipantListener
    {
        public static void Main()
        {
            TestBinding binding = new TestBinding();
            binding.Run();
        }

        public void Run()
        {
#if DBCallTests
            DDS.Test.DatabaseTests();
#endif

            Console.WriteLine("Press enter to enter...");
            Console.ReadLine();
            Data.DataTest detectionData = new Data.DataTest();
            detectionData.TestId = 3214;
            detectionData.Emergency = true;
            detectionData.TestStr = "not really";
            //detectionData.SeqInt[3] = 23;
#if TestMarshaling

            //Tactical.DetectionTypeSupport support = new Tactical.DetectionTypeSupport();
            //Tactical.Detection cachedObj = new Tactical.Detection();
            //support.Copy(cachedObj, detectionData);

            //SampleMarshaler marshaler = SampleMarshalerFactory.CreateMarshaler(detectionData);

            //using (SampleMarshalHelper helper = new SampleMarshalHelper(marshaler))
            //{
            //    DDS.OpenSplice.Gapi.Test.test_detection(helper.GapiPtr);

            //    Tactical.Detection detectionData2 = new Tactical.Detection();
            //    SampleMarshaler marshaler2 = SampleMarshalerFactory.CreateMarshaler(detectionData2);
            //    marshaler2.CopyOut(helper.GapiPtr, 0);
            //}

            //Duration d = new Duration(234, 2343);
            //int sec;
            //uint nanosec;
            //DDS.OpenSplice.Gapi.Test.test_duration(d, out sec, out nanosec);
            
            //LivelinessChangedStatus status;
            //DDS.OpenSplice.Gapi.Test.get_liveliness_changed_status(out status);
            
            //Time t = new Time(1, 2);
            //int size = Marshal.SizeOf(t);
            //IntPtr ptr = Marshal.AllocHGlobal(size);
            //Marshal.StructureToPtr(t, ptr, true);
#endif

            //DDS.Test.TestDataReaderQos();
			//DDS.Test.TestTopicQos();

            // Create a DomainParticipantFactory
            DomainParticipantFactory dpf = DomainParticipantFactory.Instance;
            Console.WriteLine("DomainParticipantFactory: " + dpf);

            // Tailor the DomainPartipantFactoryQos;
            DomainParticipantFactoryQos dpfQos = null;
            ReturnCode result = dpf.GetQos(ref dpfQos);
            Console.WriteLine("DomainParticipantFactory.get_qos: {0}", result);
            Console.WriteLine("DomainParticipantFactoryQos.entity_factory.autoenable_created_entities: " + dpfQos.EntityFactory.AutoenableCreatedEntities);

            dpfQos.EntityFactory.AutoenableCreatedEntities = true;
            result = dpf.SetQos(dpfQos);
            Console.WriteLine("DomainParticipantFactory.set_qos: {0}", result);

            // Get the QOS settings for the Factory...  Check values without additional changes
            DomainParticipantFactoryQos dpf2Qos = null;
            result = dpf.GetQos(ref dpf2Qos);
            Console.WriteLine("DomainParticipantFactory.get_qos: {0}", result);
			Console.WriteLine("DomainParticipantFactoryQos.entity_factory.autoenable_created_entities: " + dpf2Qos.EntityFactory.AutoenableCreatedEntities);

            // Create the domainParticipant itself.
            DomainParticipantQos dpQos = new DomainParticipantQos();
            dpQos.UserData.Value = new byte[] { (byte)1, (byte)2, (byte)3 };
            dpQos.EntityFactory.AutoenableCreatedEntities = true;
            dpQos.ListenerScheduling.SchedulingClass.Kind = SchedulingClassQosPolicyKind.ScheduleDefault;
            dpQos.ListenerScheduling.SchedulingPriorityKind.Kind = SchedulingPriorityQosPolicyKind.PriorityRelative;
            dpQos.ListenerScheduling.SchedulingPriority = 0;
            dpQos.WatchdogScheduling.SchedulingClass.Kind = SchedulingClassQosPolicyKind.ScheduleDefault;
            dpQos.WatchdogScheduling.SchedulingPriorityKind.Kind = SchedulingPriorityQosPolicyKind.PriorityRelative;
            dpQos.WatchdogScheduling.SchedulingPriority = 4;

            IDomainParticipant dp = dpf.CreateParticipant(DDS.DomainId.Default, dpQos);
            Console.Write("DomainParticipant: ");
            Console.WriteLine(dp != null ? "yes" : "no");


            if (dp.ContainsEntity(0))
            {
                Console.WriteLine("contains_entity with nil handle incorrect");
            }
            if (dp.ContainsEntity(100))
            {
                Console.WriteLine("contains_entity with incorrect handle incorrect");
            }

            Time currentTime;
            dp.GetCurrentTime(out currentTime);
            Console.WriteLine("Current Local Time: {0}", currentTime.ToDatetime().ToLocalTime());

            // And look up this DomainParticipant.
            IDomainParticipant dp2 = dpf.LookupParticipant(DDS.DomainId.Default);

            DomainParticipantQos dp2Qos = null;

            Console.Write("lookup DomainParticipant: ");
            Console.WriteLine(dp2 != null ? "Success" : "Fail");
            result = dp2.GetQos(ref dp2Qos);
            Console.WriteLine("DomainParticipant.get_qos: {0}", result);
            Console.WriteLine("DomainParticipantQos.entity_factory.autoenable_created_entities: " + dp2Qos.EntityFactory.AutoenableCreatedEntities);
            // Create a new PublisherQos and set some values...
            PublisherQos publisherQos = new PublisherQos();
            publisherQos.EntityFactory.AutoenableCreatedEntities = true;
            publisherQos.Partition.Name = new string[] { "howdy" }; //, "neighbor", "partition" };

            // true not supported in 4.1 ??
            publisherQos.Presentation.OrderedAccess = false;

            // Create the Publisher
            dp.Enable();
            IPublisher publisher = dp.CreatePublisher(publisherQos);
            Console.WriteLine("Create Publisher: {0}", publisher);

            DataWriterQos dwQos = null;
            publisher.GetDefaultDataWriterQos(ref dwQos);


            // Create a Detection Type Support and register it's type
            Data.DataTestTypeSupport support = new Data.DataTestTypeSupport();

            string test2 = support.TypeName;
            Console.WriteLine("Register Typesupport");
            result = support.RegisterType(dp, support.TypeName);
            Console.WriteLine("Register Typesupport Result: {0}", result);

            // Create a topic for the Detection type
            TopicQos topicQos = null;
            result = dp.GetDefaultTopicQos(ref topicQos);

			//topicQos.Ownership.Kind = OwnershipQosPolicyKind.ExclusiveOwnershipQos;

			//DDS.Test.TestTopicQos2(ref topicQos);

            // Add a listener to the topic
            ITopic topic = dp.CreateTopic("Data_DataTest", support.TypeName, topicQos,
                this, StatusKind.InconsistentTopic);

            topicQos.History.Depth = 5;

            ITopic topic2 = dp.CreateTopic("Data_DataTest", support.TypeName, topicQos);

            //			ErrorCode errorCode;
            //			string msg;
            //			result = ErrorInfo.Update();
            //			result = ErrorInfo.GetCode(out errorCode);
            //			result = ErrorInfo.GetMessage(out msg);

            // Create a DataWriter for the topic
            Data.IDataTestDataWriter dataWriter = publisher.CreateDataWriter(topic, dwQos) as Data.IDataTestDataWriter;

            // Create a SubscriberQos object and set the partition name
            SubscriberQos subscriberQos = null;
            result = dp.GetDefaultSubscriberQos(ref subscriberQos);
            subscriberQos.Partition.Name = new string[] { "howdy" };

            // Create the subscriber
            ISubscriber sub = dp.CreateSubscriber(subscriberQos);
            // Verify that the subsciber was created...
            if (sub == null)
            {
                Console.WriteLine("Subscriber not created");
                return;
            }

            DDS.DataReaderQos readerQos = null;
            sub.GetDefaultDataReaderQos(ref readerQos);

            //readerQos.SubscriptionKeys.KeyList = new string[] { "test" };
            //readerQos.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;
            //readerQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;

            // Create a DataReader for the Detection topic
            Data.IDataTestDataReader dataReader = sub.CreateDataReader(topic, readerQos, this, StatusKind.DataAvailable)
                as Data.IDataTestDataReader;

            // Create a filtered detection topic (only read detections that have an id != 4)
            IContentFilteredTopic filteredTopic = dp.CreateContentFilteredTopic("Another", topic, "TestId <> %0", "234");
            //IContentFilteredTopic filteredTopic = dp.CreateContentFilteredTopic("Another", topic, "TestStr = %0", "not really");
            string[] testParams = null;
            result = filteredTopic.GetExpressionParameters(ref testParams);
            if (result != DDS.ReturnCode.Ok || testParams == null || testParams.Length != 1 || !testParams[0].Equals("234"))
            {
                Console.WriteLine("filteredTopic.GetExpressionParameters does not return the expected parameter.....");
            }
            else
            {
                Console.WriteLine("filteredTopic.GetExpressionParameters returns the expected parameter.....");
            }
            
            result = filteredTopic.SetExpressionParameters("hello", "test");
            if (result != DDS.ReturnCode.Ok)
            {
                Console.WriteLine("filteredTopic.SetExpressionParameters returns not OK: result = " + result);
            }
//            result = filteredTopic.GetExpressionParameters(ref testParams);

            // Create a DataReader to read the filtered topic
            Data.IDataTestDataReader reader2 = sub.CreateDataReader(filteredTopic) as Data.IDataTestDataReader;

            IQueryCondition queryCondition = dataReader.CreateQueryCondition(
                "TestId = %0",
                "234");

            // just for testing...
            GC.Collect();

            // WaitSet
            WaitSet waitSet = new WaitSet();

            // either use status conditions...or
            IStatusCondition sc = reader2.StatusCondition;
            sc.SetEnabledStatuses(StatusKind.DataAvailable);
            waitSet.AttachCondition(sc);

            IStatusCondition sc2 = reader2.StatusCondition;

            // read conditions...
             IReadCondition readCond = reader2.CreateReadCondition();
             waitSet.AttachCondition(readCond);

            Console.WriteLine("Press enter to write data");
            Console.ReadLine();

            detectionData.SequenceTest = new int[1];// new System.Collections.Generic.List<int>();
			//detectionData.SequenceTest.Add(4);
            detectionData.SequenceTest[0] = 4;

            // Write detection data
            result = dataWriter.Write(detectionData);

            detectionData = new Data.DataTest();
            detectionData.TestId = 234;
            dataWriter.Write(detectionData, InstanceHandle.Nil);

            detectionData = new Data.DataTest();
            detectionData.TestId = 235;
            dataWriter.Write(detectionData);

			detectionData = new Data.DataTest();
			detectionData.TestId = 236;
			dataWriter.Write(detectionData);

			detectionData = new Data.DataTest();
			detectionData.TestId = 237;
			dataWriter.Write(detectionData);

            Console.WriteLine("Press enter to read data");
            Console.ReadLine();

            // Read the data
            ICondition[] cond = null;
            waitSet.Wait(ref cond, Duration.Infinite);
            

            SampleInfo[] infos = null;
            Data.DataTest[] dataValues = null;

            result = dataReader.Read(ref dataValues, ref infos);
            Console.WriteLine("dataReader: {0}", dataReader);

            if (dataValues != null)
            {
                Console.WriteLine("Number of samples received: {0}", dataValues.Length);
                for (int index = 0; index < dataValues.Length; index++)
                {
                    Console.WriteLine("TestId: {0}, ProviderId: {1}, Emergency: {2}, TestStr: {3}", dataValues[index].TestId,
                                      dataValues[index].ProviderId, dataValues[index].Emergency, dataValues[index].TestStr);
                    Console.WriteLine("info: ValidData: {0},  InstHandle: {1},  PubHandle:{2},  SourceTS: {3}, ArrivalTS: {4}, sample: {5}, view: {6}, instance: {7}",
                                      infos[index].ValidData,
                                      infos[index].InstanceHandle, infos[index].PublicationHandle,
                                      infos[index].SourceTimestamp, infos[index].ReceptionTimestamp,
                                      infos[index].SampleState, infos[index].ViewState, infos[index].InstanceState);
                }
            }

            Console.WriteLine("Press enter to read data from contentfiltered topic");
            Console.ReadLine();

            result = reader2.Read(ref dataValues, ref infos, DDS.Length.Unlimited);
            if (dataValues != null)
            {
                Console.WriteLine("Number of samples received: {0}", dataValues.Length);
                for (int index = 0; index < dataValues.Length; index++)
                {
                    Console.WriteLine("TestId: {0}, ProviderId: {1}, Emergency: {2}, TestStr: {3}", dataValues[index].TestId,
                                      dataValues[index].ProviderId, dataValues[index].Emergency, dataValues[index].TestStr);
                    Console.WriteLine("info: ValidData: {0},  InstHandle: {1},  PubHandle:{2},  SourceTS: {3}, ArrivalTS: {4}, sample: {5}, view: {6}, instance: {7}",
                                      infos[index].ValidData,
                                      infos[index].InstanceHandle, infos[index].PublicationHandle,
                                      infos[index].SourceTimestamp, infos[index].ReceptionTimestamp,
                                      infos[index].SampleState, infos[index].ViewState, infos[index].InstanceState);
                }
            }

            Console.WriteLine("Press enter to read data from query");
            Console.ReadLine();

            result = dataReader.ReadWithCondition(ref dataValues, ref infos, DDS.Length.Unlimited, queryCondition);
            if (dataValues != null)
            {
                Console.WriteLine("Number of samples received: {0}", dataValues.Length);
                for (int index = 0; index < dataValues.Length; index++)
                {
                    Console.WriteLine("TestId: {0}, ProviderId: {1}, Emergency: {2}, TestStr: {3}", dataValues[index].TestId,
                                      dataValues[index].ProviderId, dataValues[index].Emergency, dataValues[index].TestStr);
                    Console.WriteLine("info: ValidData: {0},  InstHandle: {1},  PubHandle:{2},  SourceTS: {3}, ArrivalTS: {4}, sample: {5}, view: {6}, instance: {7}",
                                      infos[index].ValidData,
                                      infos[index].InstanceHandle, infos[index].PublicationHandle,
                                      infos[index].SourceTimestamp, infos[index].ReceptionTimestamp,
                                      infos[index].SampleState, infos[index].ViewState, infos[index].InstanceState);
                }
            }

            Console.WriteLine("Press enter to cleanup");
            Console.ReadLine();

            Console.WriteLine("DeleteContainedEntities");
            result = dp.DeleteContainedEntities();

            // If you don't use DeleteContainedEntities then you must delete everything that was created
            //result = sub.DeleteDataReader(dataReader);
            //result = publisher.DeleteDataWriter(dataWriter);

            //result = dp.DeleteTopic(topic);
            //result = dp.DeletePublisher(publisher);

            //result = dp.DeleteSubscriber(sub);

            Console.WriteLine("DeleteParticipant");
            result = dpf.DeleteParticipant(dp);

            Console.WriteLine("Press enter to exit");
            Console.ReadLine();

        }

        public override void OnDataAvailable(IDataReader entityInterface)
        {
            Console.WriteLine("Received OnDataAvailable");
        }

        public override void OnInconsistentTopic(ITopic entityInterface, InconsistentTopicStatus status)
        {
            // Test the listener...  This class is an TopicListener -- we can listen for an InconsistenTopic
            // by overriding the OnInconsistentTopic method of the TopicListener.  
            Console.WriteLine("Received Inconsistent Topic");
        }
    }
}
