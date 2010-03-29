namespace test.sacs
{
    /// <summary>Deinitialize a Topic.</summary>
    /// <remarks>Deinitialize a Topic.</remarks>
    public class DomainParticipantItem1Deinit : Test.Framework.TestItem
    {
        /// <summary>Deinitialize a Topic.</summary>
        /// <remarks>Deinitialize a Topic.</remarks>
        public DomainParticipantItem1Deinit()
            : base("Deinitialize a Topic.")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            string expectedResult = "Topic is deinitialized";
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult(expectedResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            topic = (DDS.ITopic)testCase.ResolveObject("topic");
            rc = participant.DeleteTopic(topic);
            if (rc == DDS.ReturnCode.PreconditionNotMet)
            {
                rc = participant.DeleteContainedEntities();
            }
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Recieved return code " + rc + " after calling participant.delete_topic";
                return result;
            }
            testCase.UnregisterObject("topic");
            result.Result = expectedResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
