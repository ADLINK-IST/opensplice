namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class DomainParticipantItemDeinit : Test.Framework.TestItem
    {
        public DomainParticipantItemDeinit()
            : base("DomainParticipant deinitialization."
                )
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
            if (factory == null)
            {
                result.Result = "DomainParticipantFactory could not be deinitialized.";
                return result;
            }
            testCase.UnregisterObject("factory");
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            if (participant == null)
            {
                result.Result = "DomainParticipant could not be found.";
                return result;
            }
            rc = factory.DeleteParticipant(participant);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "DomainParticipant could not be deleted.";
                return result;
            }
            testCase.UnregisterObject("participant");
            testCase.UnregisterObject("participantQos");
            result.Result = "Deinitialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
