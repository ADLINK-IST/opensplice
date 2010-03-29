namespace test
{
    /// <date>Jun 2, 2005</date>
    public class SACSTesterPublisher
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Publisher1());
            suite.AddTest(new test.sacs.Publisher2());
            suite.AddTest(new test.sacs.Publisher3());
            suite.AddTest(new test.sacs.Publisher4());
            suite.AddTest(new test.sacs.Publisher5());
            suite.AddTest(new test.sacs.Publisher6());
            suite.AddTest(new test.sacs.Publisher7());
            suite.AddTest(new test.sacs.Publisher8());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
