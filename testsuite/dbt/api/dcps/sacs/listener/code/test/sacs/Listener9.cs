namespace test.sacs
{
	/// <date>Jun 2, 2005</date>
	public class Listener9 : Test.Framework.TestCase
	{
		public Listener9() : base("sacs_listener_tc9", "sacs_listener", "listener", "Test if the initial events of a DataReaderListener are processed."
			, "Test if the initial events of a DataReaderListener are processed.", null)
		{
		}

		public override Test.Framework.TestResult Run()
		{
			DDS.DomainParticipantFactory factory;
			DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder;
			DDS.TopicQos topQosHolder;
			DDS.ITopic topic;
			mod.tstTypeSupport typeSupport = null;
			mod.tstDataReader datareader;
			test.sacs.MyDataReaderListener listener;
			DDS.ISubscriber subscriber;
			DDS.SubscriberQos sqosHolder;
			DDS.DataReaderQos dqosHolder;
			DDS.IPublisher publisher;
			DDS.PublisherQos pubQosHolder;
			mod.tstDataWriter datawriter;
			DDS.DataWriterQos wqosHolder;
			Test.Framework.TestResult result;
			DDS.ReturnCode rc;
			string expResult = "DataReaderListener test succeeded.";
			result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
				.Pass, Test.Framework.TestVerdict.Fail);
			factory = DDS.DomainParticipantFactory.GetInstance();
			if (factory == null)
			{
				result.Result = "DomainParticipantFactory could not be initialized.");
				return result;
			}
			pqosHolder = new DDS.DomainParticipantQos();
			factory.GetDefaultParticipantQos(pqosHolder);
			if (pqosHolder.Value == null)
			{
				result.Result = "Default DomainParticipantQos could not be resolved.");
				return result;
			}
			participant = factory.CreateParticipant(string.Empty, pqosHolder.Value, null, 0);
			if (participant == null)
			{
				result.Result = "Creation of DomainParticipant failed.");
				return result;
			}
			typeSupport = new mod.tstTypeSupport();
			if (typeSupport == null)
			{
				result.Result = "Creation of tstTypeSupport failed.");
				this.Cleanup(factory, participant);
				return result;
			}
			rc = typeSupport.RegisterType(participant, "my_type");
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Register type failed.");
				this.Cleanup(factory, participant);
				return result;
			}
			topQosHolder = new DDS.TopicQos();
			participant.GetDefaultTopicQos(topQosHolder);
			if (topQosHolder.Value == null)
			{
				result.Result = "Default TopicQos could not be resolved.");
				this.Cleanup(factory, participant);
				return result;
			}
			topQosHolder.Value.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
			topic = participant.CreateTopic("my_topic_t", "my_type", topQosHolder.Value, null
				, 0);
			if (topic == null)
			{
				result.Result = "Topic could not be created.");
				this.Cleanup(factory, participant);
				return result;
			}
			pubQosHolder = new DDS.PublisherQos();
			participant.GetDefaultPublisherQos(pubQosHolder);
			if (pubQosHolder.Value == null)
			{
				result.Result = "Default PublisherQos could not be resolved.");
				this.Cleanup(factory, participant);
				return result;
			}
			publisher = participant.CreatePublisher(pubQosHolder.Value, null, 0);
			if (publisher == null)
			{
				result.Result = "Publisher could not be created.");
				this.Cleanup(factory, participant);
				return result;
			}
			wqosHolder = new DDS.DataWriterQos();
			publisher.GetDefaultDataWriterQos(wqosHolder);
			if (wqosHolder.Value == null)
			{
				result.Result = "Default DataWriterQos could not be resolved.");
				this.Cleanup(factory, participant);
				return result;
			}
			wqosHolder.Value.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
			datawriter = (mod.tstDataWriter)publisher.CreateDataWriter(topic, wqosHolder.Value
				, null, 0);
			if (datawriter == null)
			{
				result.Result = "DataWriter could not be created.");
				this.Cleanup(factory, participant);
				return result;
			}
			mod.tst t = new mod.tst(1, 2, 3);
			rc = datawriter.Write(t, DDS.InstanceHandle.Nil);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Data could not be written.");
				this.Cleanup(factory, participant);
				return result;
			}
			sqosHolder = new DDS.SubscriberQos();
			participant.GetDefaultSubscriberQos(sqosHolder);
			if (sqosHolder.Value == null)
			{
				result.Result = "Default SubscriberQos could not be resolved.");
				this.Cleanup(factory, participant);
				return result;
			}
			subscriber = participant.CreateSubscriber(sqosHolder.Value, null, 0);
			if (subscriber == null)
			{
				result.Result = "Subscriber could not be created.");
				this.Cleanup(factory, participant);
				return result;
			}
			dqosHolder = new DDS.DataReaderQos();
			subscriber.GetDefaultDataReaderQos(dqosHolder);
			if (dqosHolder.Value == null)
			{
				result.Result = "Default DataReaderQos could not be resolved.");
				this.Cleanup(factory, participant);
				return result;
			}
			dqosHolder.Value.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
			listener = new test.sacs.MyDataReaderListener();
			datareader = (mod.tstDataReader)subscriber.CreateDataReader(topic, dqosHolder.Value
				, listener, DDS.ANY_STATUS.Value);
			if (datareader == null)
			{
				result.Result = "DataReader could not be created.");
				this.Cleanup(factory, participant);
				return result;
			}
			try
			{
				java.lang.Thread.Sleep(3000);
			}
			catch (System.Exception e)
			{
				Sharpen.Runtime.PrintStackTrace(e);
			}
			if (!listener.onLivelinessChangedCalled)
			{
				result.Result = "on_liveliness_changed does not work properly.");
				this.Cleanup(factory, participant);
				return result;
			}
			datareader.GetLivelinessChangedStatus(new DDS.LivelinessChangedStatusHolder());
			if (!listener.onDataAvailableCalled)
			{
				result.Result = "on_data_available does not work properly.");
				this.Cleanup(factory, participant);
				return result;
			}
			rc = participant.DeleteContainedEntities();
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Delete contained entities failed.");
				this.Cleanup(factory, participant);
				return result;
			}
			rc = factory.DeleteParticipant(participant);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Delete DomainParticipant failed.");
				return result;
			}
			result.Result = expResult);
			result.Verdict = Test.Framework.TestVerdict.Pass);
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
