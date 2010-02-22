namespace test
{
    /// <date>May 24, 2005</date>
    public class SACSTesterTypeSupport
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.TypeSupport1());
            suite.AddTest(new test.sacs.TypeSupport2());
            suite.AddTest(new test.sacs.TypeSupport3());
            suite.AddTest(new test.sacs.TypeSupport4());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
