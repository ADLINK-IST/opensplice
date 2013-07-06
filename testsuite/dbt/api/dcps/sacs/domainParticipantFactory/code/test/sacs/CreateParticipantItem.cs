namespace test.sacs
{
    /// <summary>
    /// Create a DomainParticipant and register it by the TestCase as
    /// "participant1" with a default qos.
    /// </summary>
    /// <remarks>
    /// Create a DomainParticipant and register it by the TestCase as
    /// "participant1" with a default qos.
    /// </remarks>
    public class CreateParticipantItem : Test.Framework.TestItem
    {
        /// <summary>
        /// Create a DomainParticipant and register it by the TestCase as
        /// "participant1" with a default qos.
        /// </summary>
        /// <remarks>
        /// Create a DomainParticipant and register it by the TestCase as
        /// "participant1" with a default qos.
        /// The DomainParticipantFactory is registered as "theFactory".
        /// </remarks>
        public CreateParticipantItem()
            : base("create a DomainParticipant")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            Test.Framework.TestResult result;
            DDS.DomainParticipantFactory factory;
			DDS.DomainParticipantQos qosHolder = null;
            DDS.IDomainParticipant participant1;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult("newly created DomainParticipant", "newly created DomainParticipant"
                , Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Pass);
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result.Result = "failure creating a DomainParticipantFactory";
                result.Verdict = Test.Framework.TestVerdict.Fail;
                return result;
            }

            rc = factory.GetDefaultParticipantQos(ref qosHolder);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "failure resolving the default participant qos (" + rc + ").";
                result.Verdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            participant1 = factory.CreateParticipant(DDS.DomainId.Default, qosHolder);//, null, 0);
            if (participant1 == null)
            {
                result.Result = "failure creating a DomainParticipant using null as qos parameter";
                result.Verdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            testCase.RegisterObject("theFactory", factory);
            testCase.RegisterObject("participant1", participant1);
            return result;
        }
    }
}
