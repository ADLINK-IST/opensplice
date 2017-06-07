using System.Threading;
using System.Collections.Generic;
namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Listener8 : Test.Framework.TestCase
    {
        Dictionary<DDS.StatusKind, Semaphore> semaphores = null;
        public Listener8()
            : base("sacs_listener_tc8", "sacs_listener", "listener", "Test if a DataReaderListener works."
                , "Test if a DataReaderListener works.", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.TopicQos topQosHolder = null;
            DDS.ITopic topic;
            mod.tstTypeSupport typeSupport = null;
            mod.tstDataReader datareader;
            test.sacs.MyDataReaderListener listener;
            DDS.ISubscriber subscriber;
			DDS.SubscriberQos sqosHolder = null;
			DDS.DataReaderQos dqosHolder = null;
            DDS.IPublisher publisher;
			DDS.PublisherQos pubQosHolder = null;
            mod.tstDataWriter datawriter;
			DDS.DataWriterQos wqosHolder = null;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            string expResult = "DataReaderListener test succeeded.";
            semaphores = new Dictionary<DDS.StatusKind, Semaphore>();
            semaphores.Add(DDS.StatusKind.DataAvailable, new Semaphore(0, 1));

            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result.Result = "DomainParticipantFactory could not be initialized.";
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref pqosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DomainParticipantQos could not be resolved.";
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant == null)
            {
                result.Result = "Creation of DomainParticipant failed.";
                return result;
            }
            typeSupport = new mod.tstTypeSupport();
            if (typeSupport == null)
            {
                result.Result = "Creation of tstTypeSupport failed.";
                this.Cleanup(factory, participant);
                return result;
            }
            rc = typeSupport.RegisterType(participant, "my_type");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Register type failed.";
                this.Cleanup(factory, participant);
                return result;
            }

            if (participant.GetDefaultTopicQos(ref topQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default TopicQos could not be resolved.";
                this.Cleanup(factory, participant);
                return result;
            }
            topic = participant.CreateTopic("my_topic", "my_type", topQosHolder);//, null,
                //0);
            if (topic == null)
            {
                result.Result = "Topic could not be created.";
                this.Cleanup(factory, participant);
                return result;
            }

            if (participant.GetDefaultSubscriberQos(ref sqosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default SubscriberQos could not be resolved.";
                this.Cleanup(factory, participant);
                return result;
            }
            subscriber = participant.CreateSubscriber(sqosHolder);//, null, 0);
            if (subscriber == null)
            {
                result.Result = "Subscriber could not be created.";
                this.Cleanup(factory, participant);
                return result;
            }

            if (subscriber.GetDefaultDataReaderQos(ref dqosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DataReaderQos could not be resolved.";
                this.Cleanup(factory, participant);
                return result;
            }
            datareader = (mod.tstDataReader)subscriber.CreateDataReader(topic, dqosHolder);//, null, 0);
            if (datareader == null)
            {
                result.Result = "DataReader could not be created.";
                this.Cleanup(factory, participant);
                return result;
            }

            if (participant.GetDefaultPublisherQos(ref pubQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default PublisherQos could not be resolved.";
                this.Cleanup(factory, participant);
                return result;
            }
            publisher = participant.CreatePublisher(pubQosHolder);//, null, 0);
            if (publisher == null)
            {
                result.Result = "Publisher could not be created.";
                this.Cleanup(factory, participant);
                return result;
            }

            if (publisher.GetDefaultDataWriterQos(ref wqosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DataWriterQos could not be resolved.";
                this.Cleanup(factory, participant);
                return result;
            }
            datawriter = (mod.tstDataWriter)publisher.CreateDataWriter(topic, wqosHolder);//, null, 0);
            if (datawriter == null)
            {
                result.Result = "DataWriter could not be created.";
                this.Cleanup(factory, participant);
                return result;
            }
            listener = new test.sacs.MyDataReaderListener(semaphores);
            rc = datareader.SetListener(listener, DDS.StatusKind.LivelinessChanged | DDS.StatusKind.DataAvailable);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Set listener failed.";
                return result;
            }
            mod.tst t = new mod.tst();
            t.long_1 = 1;
            t.long_2 = 2;
            t.long_3 = 3;

            rc = datawriter.Write(t, DDS.InstanceHandle.Nil);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Data could not be written.";
                this.Cleanup(factory, participant);
                return result;
            }
            if(semaphores[DDS.StatusKind.DataAvailable].WaitOne(10000)) {
	            if (!listener.onDataAvailableCalled)
	            {
	                result.Result = "on_data_available does not work properly.";
	                this.Cleanup(factory, participant);
	                return result;
	            }
            } else {
                result.Result = "on_data_available did not trigger";
                this.Cleanup(factory, participant);
                return result;
            }
            listener.Reset();
            rc = publisher.DeleteDataWriter(datawriter);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "DataWriter could not be deleted.";
                this.Cleanup(factory, participant);
                return result;
            }
            rc = participant.DeleteContainedEntities();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete contained entities failed.";
                this.Cleanup(factory, participant);
                return result;
            }
            rc = factory.DeleteParticipant(participant);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete DomainParticipant failed.";
                this.Cleanup(factory, participant);
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }

        private void Cleanup(DDS.DomainParticipantFactory f, DDS.IDomainParticipant p)
        {
            if (p != null)
            {
                p.DeleteContainedEntities();
            }
            if (f != null)
            {
                f.DeleteParticipant(p);
            }
        }
    }
}
