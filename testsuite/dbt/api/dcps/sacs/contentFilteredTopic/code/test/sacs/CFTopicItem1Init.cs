namespace test.sacs
{
	/// <summary>Initialize a DomainParticipant.</summary>
	/// <remarks>Initialize a DomainParticipant.</remarks>
	public class CFTopicItem1Init : Test.Framework.TestItem
	{
		/// <summary>Initialize a DomainParticipant.</summary>
		/// <remarks>Initialize a DomainParticipant.</remarks>
		public CFTopicItem1Init() : base("Initialize DomainParticipant")
		{
		}

		public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
		{
			DDS.DomainParticipantFactory factory;
			DDS.IDomainParticipant participant;
			mod.tstTypeSupport typeSupport;
			DDS.TopicQos topicQosHolder;
			DDS.ITopic topic;
			DDS.DomainParticipantQos pqosHolder;
			DDS.ReturnCode rc;
			Test.Framework.TestResult result;
			result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
				Test.Framework.TestVerdict.Fail);
			factory = DDS.DomainParticipantFactory.GetInstance();
			if (factory == null)
			{
				result.Result = "DomainParticipantFactory could not be initialized.";
				return result;
			}

            if (factory.GetDefaultParticipantQos(out pqosHolder) != DDS.ReturnCode.Ok)
			{
				result.Result = "Default DomainParticipantQos could not be resolved.";
				return result;
			}
			participant = factory.CreateParticipant(string.Empty, ref pqosHolder, null, 0);
			if (participant == null)
			{
				result.Result = "Creation of DomainParticipant failed.";
				return result;
			}
			typeSupport = new mod.tstTypeSupport();
			if (typeSupport == null)
			{
				result.Result = "Creation of tstTypeSupport failed.";
				return result;
			}
			rc = typeSupport.RegisterType(participant, "my_type");
			if (rc != DDS.ReturnCode.Ok)
			{
				result.Result = "register_type failed.";
				return result;
			}

            if (participant.GetDefaultTopicQos(out topicQosHolder) != DDS.ReturnCode.Ok)
			{
				result.Result = "participant.get_default_topic_qos failed.";
				return result;
			}
			topic = participant.CreateTopic("my_topic", "my_type", ref topicQosHolder, null, 0);
			if (topic == null)
			{
				result.Result = "participant.create_topic failed.";
				return result;
			}
			testCase.RegisterObject("factory", factory);
			testCase.RegisterObject("participantQos", pqosHolder);
			testCase.RegisterObject("participant", participant);
			testCase.RegisterObject("topic", topic);
			result.Result = "Initialization success.";
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
