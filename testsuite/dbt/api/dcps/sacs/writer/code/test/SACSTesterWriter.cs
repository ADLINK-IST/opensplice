namespace test
{
    /// <date>Jun 13, 2005</date>
    public class SACSTesterWriter
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Writer1());
            suite.AddTest(new test.sacs.Writer2());
            suite.AddTest(new test.sacs.Writer3());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
