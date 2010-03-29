namespace test
{
    /// <date>May 31, 2005</date>
    public class SACSErrorInfoTester
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Errorinfo1());
            suite.AddTest(new test.sacs.Errorinfo2());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
