using System;

namespace test
{
    /// <date>Dec 22, 2013</date>
    public class SACSQosProviderTester
    {
        public static void Main (string[] args)
        {
            string OSPL_HOME = System.Environment.GetEnvironmentVariable ("OSPL_HOME");
            string base_uri = "file://" + OSPL_HOME + "/testsuite/dbt/api/dcps/sacs/qosProvider/code";

            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.QosProviderEmpty(base_uri));
            suite.AddTest(new test.sacs.QosProviderDefaults(base_uri));
            suite.AddMilestone("Test stop");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
