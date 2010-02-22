namespace test.sacs
{
    /// <summary>Test get_instance function of the DomainParticipantFactory compared to constant TheParticipantFactory.
    /// 	</summary>
    /// <remarks>Test get_instance function of the DomainParticipantFactory compared to constant TheParticipantFactory.
    /// 	</remarks>
    public class DomainParticipantFactory7 : Test.Framework.TestCase
    {
        public DomainParticipantFactory7()
            : base("sacs_domainParticipantFactory_tc7", "sacs_domainParticipantFactory"
                , "domainParticipantFactory", "domainParticipantFactory singleton constant/query"
                , "Verify regular query, compare with constant TheParticipantFactory", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.DomainParticipantFactory factory1;
            DDS.DomainParticipantFactory factory2;
            Test.Framework.TestResult result = new Test.Framework.TestResult("factory1 and factory2 equal"
                , "OK", Test.Framework.TestVerdict.Pass, Test.Framework.TestVerdict.Pass);
            factory1 = DDS.DomainParticipantFactory.Instance;
            factory2 = DDS.DomainParticipantFactory.Instance;
            if (factory1 != factory2)
            {
                result.Result = "factory1 and factory2 differ";
                result.Verdict = Test.Framework.TestVerdict.Fail;
            }
            return result;
        }
    }
}
