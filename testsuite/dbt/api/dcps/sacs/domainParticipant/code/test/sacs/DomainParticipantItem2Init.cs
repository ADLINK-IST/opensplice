namespace test.sacs
{
    /// <summary>Initialize a Subscriber.</summary>
    /// <remarks>Initialize a Subscriber.</remarks>
    public class DomainParticipantItem2Init : Test.Framework.TestItem
    {
        /// <summary>Initialize a Subscriber.</summary>
        /// <remarks>Initialize a Subscriber.</remarks>
        public DomainParticipantItem2Init()
            : base("Initialize a Subscriber.")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.IDomainParticipant participant;
			DDS.SubscriberQos subscriberQosHolder = null;
            DDS.ISubscriber subscriber;
            Test.Framework.TestResult result;
            DDS.ITopic topic;
            participant = (DDS.IDomainParticipant)testCase.ResolveObject("participant");
            topic = (DDS.ITopic)testCase.ResolveObject("topic");
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            if (participant == null || topic == null)
            {
                System.Console.Error.WriteLine("participant or topic = null");
                result.Result = "precondition not met";
                return result;
            }

            if (participant.GetDefaultSubscriberQos(ref subscriberQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "could not get default SubscriberQos";
                return result;
            }
            subscriber = participant.CreateSubscriber(subscriberQosHolder);//, null, DDS.StatusKind.Any);
            if (subscriber == null)
            {
                result.Result = "could create a Subscriber";
                return result;
            }
            testCase.RegisterObject("subscriber", subscriber);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
