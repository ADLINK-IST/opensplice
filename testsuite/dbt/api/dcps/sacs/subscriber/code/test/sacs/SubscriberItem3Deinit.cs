namespace test.sacs
{
    /// <summary>Remove/Delete the entities created in SubscriberItem3Init.</summary>
    /// <remarks>Remove/Delete the entities created in SubscriberItem3Init.</remarks>
    public class SubscriberItem3Deinit : Test.Framework.TestItem
    {
        /// <summary>Deinitializes the entities created in SubscriberItem3Init.</summary>
        /// <remarks>Deinitializes the entities created in SubscriberItem3Init.</remarks>
        public SubscriberItem3Deinit()
            : base("Deinitialize 3")
        {
        }

        /// <summary>Remove/Delete the entities created in SubscriberItem3Init.</summary>
        /// <remarks>Remove/Delete the entities created in SubscriberItem3Init.</remarks>
        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            topic = (DDS.ITopic)testCase.ResolveObject("otherTopic");
            result = new Test.Framework.TestResult("Deinitialization 3 success", string.Empty
                , Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Fail);
            rc = participant.DeleteTopic(topic);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete a Topic.";
                return result;
            }
            testCase.UnregisterObject("otherTopic");
            result.Result = "Deinitialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
