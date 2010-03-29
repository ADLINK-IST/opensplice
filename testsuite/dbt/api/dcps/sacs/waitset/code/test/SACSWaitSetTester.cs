namespace test
{
    /// <date>May 31, 2005</date>
    public class SACSWaitSetTester
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Waitset1());
            suite.AddTest(new test.sacs.Waitset2());
            suite.AddTest(new test.sacs.Waitset3());
            suite.AddTest(new test.sacs.Waitset4());
            suite.AddTest(new test.sacs.Waitset5());
            suite.AddTest(new test.sacs.Waitset6());
            suite.AddTest(new test.sacs.Waitset7());
            suite.AddTest(new test.sacs.Waitset8());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
