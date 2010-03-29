namespace test.sacs
{
    /// <date>Sep 5, 2009</date>
    public class BuiltinDeinit : Test.Framework.TestItem
    {
        public BuiltinDeinit()
            : base("Deinitialize builtin test")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult("Deinitialization success", string.Empty,
                Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Fail);
            factory = (DDS.IDomainParticipantFactory)testCase.ResolveObject("factory");
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
            testCase.UnregisterObject("factory");
            result.Result = "Deinitialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
