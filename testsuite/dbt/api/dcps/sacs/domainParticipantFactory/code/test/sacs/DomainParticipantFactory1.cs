namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipantFactory1 : Test.Framework.TestCase
    {
        public DomainParticipantFactory1()
            : base("sacs_domainParticipantFactory_tc1", "sacs_domainParticipantFactory"
                , "domainParticipantFactory", "Test if a DomainParticipantFactory can be resolved."
                , "Test if a DomainParticipantFactory can be resolved", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Resolving DomainParticipantFactory succeeded.";
            DDS.DomainParticipantFactory factory;
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipantFactory could not be initialised."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
                );
            return result;
        }
    }
}
