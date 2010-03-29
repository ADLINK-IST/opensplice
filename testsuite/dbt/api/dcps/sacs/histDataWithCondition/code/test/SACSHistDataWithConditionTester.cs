namespace test
{
    public class SACSHistDataWithConditionTester
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.HistDataWithCondition1());
            suite.AddTest(new test.sacs.HistDataWithCondition2());
            suite.AddTest(new test.sacs.HistDataWithCondition3());
            suite.AddTest(new test.sacs.HistDataWithCondition4());
            suite.AddTest(new test.sacs.HistDataWithCondition5());
            suite.AddTest(new test.sacs.HistDataWithCondition6());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
