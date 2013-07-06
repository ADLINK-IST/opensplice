namespace test.sacs
{
    /// <summary>Test lookup function of the DomainParticipantFactory.</summary>
    /// <remarks>Test lookup function of the DomainParticipantFactory.</remarks>
    public class DomainParticipantFactory6 : Test.Framework.TestCase
    {
        public DomainParticipantFactory6()
            : base("sacs_domainParticipantFactory_tc6", "sacs_domainParticipantFactory"
                , "domainParticipantFactory", "Lookup; lookup DomainParticipants", "Verify regular lookup and when looking for non existing Participants or using bad parameters"
                , null)
        {
            AddPreItem(new test.sacs.CreateParticipantItem());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant1;
            DDS.IDomainParticipant lookedUpParticipant1;
            DDS.DomainParticipantFactory factory;
            Test.Framework.TestResult result = new Test.Framework.TestResult("lookup returns the same objects as where originally created"
                , "OK", Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Pass);
            factory = (DDS.DomainParticipantFactory)ResolveObject("theFactory");
            participant1 = (DDS.IDomainParticipant)ResolveObject("participant1");
            lookedUpParticipant1 = factory.LookupParticipant(DDS.DomainId.Default);
            if (participant1 != lookedUpParticipant1)
            {
                result.Result = "participant1 and lookedUpParticipant1 differ";
                result.Verdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            lookedUpParticipant1 = null;
            lookedUpParticipant1 = factory.LookupParticipant(12345);
            if (lookedUpParticipant1 != null)
            {
                result.Result = "lookup returned a participant after looking for a non existing participant";
                result.Verdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            lookedUpParticipant1 = null;
            lookedUpParticipant1 = factory.LookupParticipant(DDS.DomainId.Default);
            if (lookedUpParticipant1 != participant1)
            {
                result.Result = "lookup didn't return the expected participant";
                result.Verdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            return result;
        }
    }
}
