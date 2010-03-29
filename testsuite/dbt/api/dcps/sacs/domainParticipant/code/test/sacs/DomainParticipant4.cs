namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant4 : Test.Framework.TestCase
    {
        public DomainParticipant4()
            : base("sacs_domainParticipant_tc4", "sacs_domainParticipant"
                , "domainParticipant", "Test if default PublisherQos can be resolved and set.",
                "Test if default PublisherQos can be resolved and set.", null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            string expResult = "Setting/resolving default PublisherQos succeeded.";
            DDS.IDomainParticipant participant;
			DDS.PublisherQos pubQosHolder = null;
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);

            if (participant.GetDefaultPublisherQos(ref pubQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Publisher qos could not be resolved.";
                return result;
            }
            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
