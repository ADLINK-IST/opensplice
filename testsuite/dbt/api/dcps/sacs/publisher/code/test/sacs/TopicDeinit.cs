namespace test.sacs
{
    /// <summary>TODO write class description javadoc</summary>
    public class TopicDeinit : Test.Framework.TestItem
    {
        public TopicDeinit()
            : base("Deinitialize Topic")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult("Deinitialization success", string.Empty,
                Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            topic = (DDS.ITopic)testCase.ResolveObject("topic");
            rc = participant.DeleteTopic(topic);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete Topic.";
                return result;
            }
            result.Result = "Deinitialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
