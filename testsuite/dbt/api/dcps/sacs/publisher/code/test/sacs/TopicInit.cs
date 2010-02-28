namespace test.sacs
{
    /// <summary>TODO write class description javadoc</summary>
    public class TopicInit : Test.Framework.TestItem
    {
        /// <summary>Initializes a topic, topicQos and typeSupport.</summary>
        /// <remarks>
        /// Initializes a topic, topicQos and typeSupport. This Initialization can
        /// only be called after first calling PublisherItemInit.
        /// </remarks>
        public TopicInit()
            : base("Initialize Topic")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipant participant;
			DDS.TopicQos topQosHolder = null;
            DDS.ITopic topic;
            mod.tstTypeSupport typeSupport = null;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            typeSupport = new mod.tstTypeSupport();
            if (typeSupport == null)
            {
                result.Result = "Creation of tstTypeSupport failed.";
                return result;
            }
            rc = typeSupport.RegisterType(participant, "my_type");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Register type failed.";
                return result;
            }

            if (participant.GetDefaultTopicQos(ref topQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default TopicQos could not be resolved.";
                return result;
            }
            topic = participant.CreateTopic("my_topic", "my_type", topQosHolder);//, null, 0);
            if (topic == null)
            {
                result.Result = "Topic could not be created.";
                return result;
            }
            testCase.RegisterObject("topic", topic);
            testCase.RegisterObject("topicQos", topQosHolder);
            testCase.RegisterObject("typeSupport", typeSupport);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
