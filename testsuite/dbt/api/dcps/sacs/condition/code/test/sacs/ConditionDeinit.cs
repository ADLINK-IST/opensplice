namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class ConditionDeinit : Test.Framework.TestItem
    {
        public ConditionDeinit()
            : base("Deinitialize condition")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult("Deinitialization success", string.Empty,
                Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Fail);
            factory = (DDS.DomainParticipantFactory)testCase.ResolveObject("factory");
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            if (participant == null)
            {
                result.Result = "DomainParticipant could not be found.";
                return result;
            }
            rc = participant.DeleteContainedEntities();
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete contained entities of DomainParticipant.";
                return result;
            }
            rc = factory.DeleteParticipant(participant);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete DomainParticipant.";
                return result;
            }
            testCase.UnregisterObject("participant");
            testCase.UnregisterObject("participantQos");
            testCase.UnregisterObject("topic");
            testCase.UnregisterObject("topicQos");
            testCase.UnregisterObject("publisher");
            testCase.UnregisterObject("publisherQos");
            testCase.UnregisterObject("subscriber");
            testCase.UnregisterObject("subscriberQos");
            testCase.UnregisterObject("datareader");
            testCase.UnregisterObject("datareaderQos");
            testCase.UnregisterObject("datawriter");
            testCase.UnregisterObject("datawriterQos");
            testCase.UnregisterObject("factory");
            result.Result = "Deinitialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
