namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class DomainParticipantItemInit : Test.Framework.TestItem
    {
        public DomainParticipantItemInit()
            : base("DomainParticipant initialization.")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result.Result = "DomainParticipantFactory could not be initialized.";
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref pqosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DomainParticipantQos could not be resolved.";
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant == null)
            {
                result.Result = "Creation of DomainParticipant failed.";
                return result;
            }
            testCase.RegisterObject("participantQos", pqosHolder);
            testCase.RegisterObject("factory", factory);
            testCase.RegisterObject("participant", participant);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
