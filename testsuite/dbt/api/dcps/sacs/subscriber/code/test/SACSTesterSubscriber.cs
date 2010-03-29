namespace test
{
    /// <date>Jun 2, 2005</date>
    public class SACSTesterSubscriber
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Subscriber1());
            suite.AddTest(new test.sacs.Subscriber2());
            suite.AddTest(new test.sacs.Subscriber3());
            suite.AddTest(new test.sacs.Subscriber4());
            suite.AddTest(new test.sacs.Subscriber5());
            suite.AddTest(new test.sacs.Subscriber6());
            suite.AddTest(new test.sacs.Subscriber7());
            suite.AddTest(new test.sacs.Subscriber8());
            suite.AddTest(new test.sacs.Subscriber9());
            suite.AddTest(new test.sacs.Subscriber10());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
