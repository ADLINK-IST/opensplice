namespace test.sacs
{
    /// <summary>Deinitialize a Subscriber.</summary>
    /// <remarks>Deinitialize a Subscriber.</remarks>
    public class DomainParticipantItem2Deinit : Test.Framework.TestItem
    {
        /// <summary>Deinitialize a Subscriber.</summary>
        /// <remarks>Deinitialize a Subscriber.</remarks>
        public DomainParticipantItem2Deinit()
            : base("Deinitialize a Subscriber.")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipant participant;
            DDS.ISubscriber subscriber;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            subscriber = (DDS.ISubscriber)testCase.ResolveObject("subscriber");
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            if (participant == null || subscriber == null)
            {
                System.Console.Error.WriteLine("participant or subscriber = null");
                result.Result = "precondition not met";
                return result;
            }
            rc = participant.DeleteSubscriber(subscriber);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a subscriber";
                return result;
            }
            testCase.UnregisterObject("subscriber");
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
