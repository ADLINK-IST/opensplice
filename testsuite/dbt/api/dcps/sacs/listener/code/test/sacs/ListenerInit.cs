namespace test.sacs
{
	/// <date>Jun 2, 2005</date>
	public class ListenerInit : Test.Framework.TestItem
	{
		public ListenerInit() : base("Initialize listener")
		{
		}

		public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
		{
			DDS.DomainParticipantFactory factory;
			DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder;
			DDS.SubscriberQos subQosHolder;
			DDS.ISubscriber subscriber;
			DDS.PublisherQos pubQosHolder;
			DDS.IPublisher publisher;
			mod.tstTypeSupport typeSupport;
			DDS.TopicQos tQosHolder;
			DDS.ITopic topic;
			DDS.DataReaderQos drQosHolder;
			mod.tstDataReader datareader;
			DDS.DataWriterQos dwQosHolder;
			mod.tstDataWriter datawriter;
			Test.Framework.TestResult result;
			DDS.ReturnCode rc;
			result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict
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
			subQosHolder = new DDS.SubscriberQos();
			participant.GetDefaultSubscriberQos(subQosHolder);
			if (subQosHolder.Value == null)
			{
				result.Result = "Default SubscriberQos could not be resolved.");
				return result;
			}
			subscriber = participant.CreateSubscriber(subQosHolder.Value, null, 0);
			if (subscriber == null)
			{
				result.Result = "Subscriber could not be created.");
				return result;
			}
			typeSupport = new mod.tstTypeSupport();
			rc = typeSupport.RegisterType(participant, "tstType");
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Typesupport could not be registered.");
				return result;
			}
			tQosHolder = new DDS.TopicQos();
			participant.GetDefaultTopicQos(tQosHolder);
			if (tQosHolder.Value == null)
			{
				result.Result = "Default TopicQos could not be resolved.");
				return result;
			}
			topic = participant.CreateTopic("tst", "tstType", tQosHolder.Value, null, 0);
			if (topic == null)
			{
				result.Result = "Topic could not be created.");
				return result;
			}
			drQosHolder = new DDS.DataReaderQos();
			subscriber.GetDefaultDataReaderQos(drQosHolder);
			if (drQosHolder.Value == null)
			{
				result.Result = "Default DataReaderQos could not be resolved.");
				return result;
			}
			datareader = (mod.tstDataReader)subscriber.CreateDataReader(topic, drQosHolder.Value
				, null, 0);
			if (datareader == null)
			{
				result.Result = "DataReader could not be created.");
				return result;
			}
			pubQosHolder = new DDS.PublisherQos();
			participant.GetDefaultPublisherQos(pubQosHolder);
			if (pubQosHolder.Value == null)
			{
				result.Result = "Default PublisherQos could not be resolved.");
				return result;
			}
			publisher = participant.CreatePublisher(pubQosHolder.Value, null, 0);
			if (publisher == null)
			{
				result.Result = "Publisher could not be created.");
				return result;
			}
			dwQosHolder = new DDS.DataWriterQos();
			publisher.GetDefaultDataWriterQos(dwQosHolder);
			if (dwQosHolder.Value == null)
			{
				result.Result = "Default DataWriterQos could not be resolved.");
				return result;
			}
			datawriter = (mod.tstDataWriter)publisher.CreateDataWriter(topic, dwQosHolder.Value
				, null, 0);
			testCase.RegisterObject("factory", factory);
			testCase.RegisterObject("participantQos", pqosHolder.Value);
			testCase.RegisterObject("participant", participant);
			testCase.RegisterObject("topic", topic);
			testCase.RegisterObject("topicQos", tQosHolder.Value);
			testCase.RegisterObject("subscriber", subscriber);
			testCase.RegisterObject("subscriberQos", subQosHolder.Value);
			testCase.RegisterObject("datareader", datareader);
			testCase.RegisterObject("datareaderQos", drQosHolder.Value);
			testCase.RegisterObject("publisher", publisher);
			testCase.RegisterObject("publisherQos", pubQosHolder.Value);
			testCase.RegisterObject("datawriter", datawriter);
			testCase.RegisterObject("datawriterQos", dwQosHolder.Value);
			result.Result = "Initialization success.");
			result.Verdict = Test.Framework.TestVerdict.Pass);
			return result;
		}
	}
}
