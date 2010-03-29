namespace test.sacs
{
    /// <summary>Deinitialize the Topic.</summary>
    /// <remarks>Deinitialize the Topic.</remarks>
    public class SubscriberItem2Deinit : Test.Framework.TestItem
    {
        /// <summary>Deinitialize the Topic.</summary>
        /// <remarks>Deinitialize the Topic.</remarks>
        public SubscriberItem2Deinit()
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
                if (rc == DDS.ReturnCode.PreconditionNotMet)
                {
                    rc = participant.DeleteContainedEntities();
                }
                if (rc != DDS.ReturnCode.Ok)
                {
                    result.Result = "Could not delete Topic.";
                    return result;
                }
            }
            testCase.UnregisterObject("topic");
            result.Result = "Deinitialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
