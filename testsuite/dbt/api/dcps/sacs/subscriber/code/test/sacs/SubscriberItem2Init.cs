namespace test.sacs
{
    /// <summary>Initializes a new TypeSupport object to create a Topic.</summary>
    /// <remarks>Initializes a new TypeSupport object to create a Topic.</remarks>
    public class SubscriberItem2Init : Test.Framework.TestItem
    {
        /// <summary>Initializes a new TypeSupport object to create a Topic.</summary>
        /// <remarks>Initializes a new TypeSupport object to create a Topic.</remarks>
        public SubscriberItem2Init()
            : base("Initialize a Topic")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipant participant;
            mod.tstTypeSupport typeSupport;
			DDS.TopicQos topQosHolder = null;
            DDS.ITopic topic;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            typeSupport = new mod.tstTypeSupport();
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
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
