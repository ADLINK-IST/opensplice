namespace test.sacs
{
    /// <date>Sep 5, 2009</date>
    public class BuiltinInit : Test.Framework.TestItem
	{
		public BuiltinInit() : base("Initialize builtin test")
		{
		}

		public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
		{
			DDS.IDomainParticipantFactory factory;
			DDS.IDomainParticipant participant;
            DDS.DomainParticipantQos participantQos;
			Test.Framework.TestResult result;
			result = new Test.Framework.TestResult("Initialization success", string.Empty, 
                Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Fail);
			factory = DDS.DomainParticipantFactory.GetInstance();
			if (factory == null)
			{
				result.Result = "DomainParticipantFactory could not be initialized.";
				return result;
			}

            if (factory.GetDefaultParticipantQos(out participantQos) != DDS.ReturnCode.Ok)
			{
				result.Result = "Default DomainParticipantQos could not be resolved.";
				return result;
			}
			participant = factory.CreateParticipant(string.Empty, ref participantQos, null, 0);
			if (participant == null)
			{
				result.Result = "Creation of DomainParticipant failed.";
				return result;
			}
			testCase.RegisterObject("factory", factory);
			testCase.RegisterObject("participantQos", participantQos);
			testCase.RegisterObject("participant", participant);
			result.Result = "Initialization success.";
			result.Verdict = Test.Framework.TestVerdict.Pass;
			return result;
		}
	}
}
