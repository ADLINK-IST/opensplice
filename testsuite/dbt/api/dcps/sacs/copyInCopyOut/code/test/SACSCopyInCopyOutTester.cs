namespace test
{
    /// <date>Jul 18, 2014</date>
    public class SACSCopyInCopyOutTester
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.CopyInCopyOut1());
            suite.AddTest(new test.sacs.CopyInCopyOut2());
            //suite.AddTest(new test.sacs.CopyInCopyOut3());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
