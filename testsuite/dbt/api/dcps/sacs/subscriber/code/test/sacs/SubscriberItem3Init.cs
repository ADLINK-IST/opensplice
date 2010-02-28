namespace test.sacs
{
    /// <summary>Initialize an other Topic.</summary>
    /// <remarks>Initialize an other Topic.</remarks>
    public class SubscriberItem3Init : Test.Framework.TestItem
    {
        /// <summary>Initialize another Topic.</summary>
        /// <remarks>Initialize another Topic.</remarks>
        public SubscriberItem3Init()
            : base("Initialize another Topic")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipant participant;
            mod.tstTypeSupport typeSupport;
            DDS.TopicQos topicQos;
            DDS.ITopic topic;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            typeSupport = (mod.tstTypeSupport)testCase.ResolveObject("typeSupport");
            topicQos = (DDS.TopicQos)testCase.ResolveObject("topicQos");
            result = new Test.Framework.TestResult("Initialization 3 success", string.Empty,
                Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Fail);
            rc = typeSupport.RegisterType(participant, "my_other_type");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Register type failed.";
                return result;
            }
            topic = participant.CreateTopic("my_other_topic", "my_other_type", topicQos);//, null, 0);
            if (topic == null)
            {
                result.Result = "Topic could not be created.";
                return result;
            }
            testCase.RegisterObject("otherTopic", topic);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
