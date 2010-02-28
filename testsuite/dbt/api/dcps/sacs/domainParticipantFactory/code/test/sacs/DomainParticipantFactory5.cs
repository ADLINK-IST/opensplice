namespace test.sacs
{
    /// <summary>Tests if multiple calls to getInstance return the same object.</summary>
    /// <remarks>Tests if multiple calls to getInstance return the same object.</remarks>
    public class DomainParticipantFactory5 : Test.Framework.TestCase
    {
        /// <summary>Tests if multiple calls to getInstance return the same object.</summary>
        /// <remarks>Tests if multiple calls to getInstance return the same object.</remarks>
        public DomainParticipantFactory5()
            : base("sacs_domainParticipantFactory_tc5", "sacs_domainParticipantFactory"
                , "domainParticipantFactory", "Initialisation; get another DomainParticipantFactory instance"
                , "Verify all instances of DomainParticipantFactory are the same", null)
        {
        }

        /// <summary>Execute the testcase</summary>
        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "All instances of DomainParticipantFactory are the same";
            DDS.DomainParticipantFactory[] factory = new DDS.DomainParticipantFactory[3];
            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
                );
            factory[0] = DDS.DomainParticipantFactory.Instance;
            factory[1] = DDS.DomainParticipantFactory.Instance;
            factory[2] = DDS.DomainParticipantFactory.Instance;
            if (factory[0] != factory[1] || factory[1] != factory[2])
            {
                result = new Test.Framework.TestResult(expResult, "Not all instances are the same"
                    , expVerdict, expVerdict);
            }
            return result;
        }
    }
}
