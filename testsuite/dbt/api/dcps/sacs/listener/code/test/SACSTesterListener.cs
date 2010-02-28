using System;

namespace test
{
    /// <date>Jun 8, 2005</date>
    public class SACSTesterListener
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Listener1());
            suite.AddTest(new test.sacs.Listener2());
            suite.AddTest(new test.sacs.Listener3());
            suite.AddTest(new test.sacs.Listener4());
            suite.AddTest(new test.sacs.Listener5());
            suite.AddTest(new test.sacs.Listener6());
            suite.AddTest(new test.sacs.Listener7());
            suite.AddTest(new test.sacs.Listener8());
            suite.AddTest(new test.sacs.Listener9());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
