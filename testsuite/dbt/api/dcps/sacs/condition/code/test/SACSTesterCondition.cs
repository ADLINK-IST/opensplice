namespace test
{
    /// <date>Jun 23, 2005</date>
    public class SACSTesterCondition
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Condition1());
            suite.AddTest(new test.sacs.Condition2());
            suite.AddTest(new test.sacs.Condition3());
            suite.AddTest(new test.sacs.Condition4());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
