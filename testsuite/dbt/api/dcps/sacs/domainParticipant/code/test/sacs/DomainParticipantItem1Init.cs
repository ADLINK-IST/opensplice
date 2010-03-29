namespace test.sacs
{
    /// <summary>Initialize a Topic.</summary>
    /// <remarks>Initialize a Topic.</remarks>
    public class DomainParticipantItem1Init : Test.Framework.TestItem
    {
        /// <summary>Initialize a Topic.</summary>
        /// <remarks>Initialize a Topic.</remarks>
        public DomainParticipantItem1Init()
            : base("Initialize a Topic.")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipant participant;
            mod.tstTypeSupport typeSupport;
            DDS.ITopic topic;
			DDS.TopicQos topicQosHolder = null;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
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

            if (participant.GetDefaultTopicQos(ref topicQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "participant.get_default_topic_qos failed.";
                return result;
            }
            topic = participant.CreateTopic("my_topic", "my_type", topicQosHolder);//, null, DDS.StatusKind.Any);
            if (topic == null)
            {
                result.Result = "participant.create_topic failed.";
                return result;
            }
            testCase.RegisterObject("topic", topic);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
