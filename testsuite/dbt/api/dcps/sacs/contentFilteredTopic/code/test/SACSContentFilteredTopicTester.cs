namespace test
{
    /// <date>Jun 2, 2005</date>
    public class SACSContentFilteredTopicTester
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.CFTopic1());
            suite.AddTest(new test.sacs.CFTopic2());
            suite.AddTest(new test.sacs.CFTopic3());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
