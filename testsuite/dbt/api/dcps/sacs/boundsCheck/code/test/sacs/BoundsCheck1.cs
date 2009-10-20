namespace test.sacs
{
	/// <date>Jun 20, 2005</date>
	public class BoundsCheck1 : Test.Framework.TestCase
	{
		public BoundsCheck1() : base("sacs_boundsCheck_tc1", "sacs_boundsCheck", "sacs_boundsCheck"
			, "test whether bounds are taken into account", "test whether bounds are taken into account"
			, null)
		{
		}

		public override Test.Framework.TestResult Run()
		{
			DDS.DomainParticipantFactory factory;
			DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder;
			DDS.PublisherQos pubQosHolder;
			DDS.IPublisher publisher;
			mod.boundsTypeTypeSupport typeSupport;
			DDS.TopicQos tQosHolder;
			DDS.ITopic topic;
			DDS.DataWriterQos dwQosHolder;
			mod.boundsTypeDataWriter datawriter;
			mod.boundsType message;
			Test.Framework.TestResult result;
			DDS.ReturnCode rc;
			string expResult = "Test succeeded.";
			result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict
				.Pass, Test.Framework.TestVerdict.Fail);
			factory = DDS.DomainParticipantFactory.GetInstance();
			if (factory == null)
			{
				result.Result = "DomainParticipantFactory could not be initialized.";
				return result;
			}
			pqosHolder = new DDS.DomainParticipantQos();
			rc = factory.GetDefaultParticipantQos(out pqosHolder);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default DomainParticipantQos could not be resolved.";
				return result;
			}
			participant = factory.CreateParticipant(string.Empty, ref pqosHolder);
			if (participant == null)
			{
				result.Result = "Creation of DomainParticipant failed.";
				return result;
			}
			typeSupport = new mod.boundsTypeTypeSupport();
			rc = typeSupport.RegisterType(participant, "boundsType");
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Typesupport could not be registered.";
				return result;
			}
			tQosHolder = new DDS.TopicQos();
			rc = participant.GetDefaultTopicQos(out tQosHolder);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default TopicQos could not be resolved.";
				return result;
			}
			topic = participant.CreateTopic("bounds", "boundsType", ref tQosHolder);
			if (topic == null)
			{
				result.Result = "Topic could not be created.";
				return result;
			}
			pubQosHolder = new DDS.PublisherQos();
			rc = participant.GetDefaultPublisherQos(out pubQosHolder);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default PublisherQos could not be resolved.";
				return result;
			}
			publisher = participant.CreatePublisher(ref pubQosHolder);
			if (publisher == null)
			{
				result.Result = "Publisher could not be created.";
				return result;
			}
			dwQosHolder = new DDS.DataWriterQos();
			rc = publisher.GetDefaultDataWriterQos(out dwQosHolder);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "Default DataWriterQos could not be resolved.";
				return result;
			}
			datawriter = (mod.boundsTypeDataWriter)publisher.CreateDataWriter(topic, ref dwQosHolder);
			message = new mod.boundsType();
			message.name = null;
			rc = datawriter.Write(message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "write of bad data did not return BAD_PARAMETER (1).";
				return result;
			}
			message = new mod.boundsType();
			message.name5 = "more_then_5_characters";
			rc = datawriter.Write(message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "write of bad data did not return BAD_PARAMETER (2).";
				return result;
			}
			message = new mod.boundsType();
			message.strSeq2 = new string[3];
			message.strSeq2[0] = "0";
			message.strSeq2[1] = "1";
			message.strSeq2[2] = "2";
			rc = datawriter.Write(message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "write of bad data did not return BAD_PARAMETER (2).";
				return result;
			}
			message = new mod.boundsType();
			message.name5 = null;
			rc = datawriter.Write(message, 0);
			if (rc != DDS.ReturnCode.BadParameter)
			{
				result.Result = "write of bad data did not return BAD_PARAMETER (2).";
				return result;
			}
			rc = participant.DeleteContainedEntities();
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "delete_contained_entities failed.";
				return result;
			}
			rc = factory.DeleteParticipant(participant);
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "delete_contained_entities failed.";
				return result;
			}
			result.Result = expResult;
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
