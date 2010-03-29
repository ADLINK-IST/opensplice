namespace test
{
    /// <date>Jun 23, 2005</date>
    public class SACSTesterReader
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Reader1());
            suite.AddTest(new test.sacs.Reader2());
            suite.AddTest(new test.sacs.Reader3());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
