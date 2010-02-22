namespace test
{
    /// <date>May 23, 2005</date>
    public class SACSTester
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.DomainParticipantFactory1());
            suite.AddTest(new test.sacs.DomainParticipantFactory2());
            suite.AddTest(new test.sacs.DomainParticipantFactory3());
            suite.AddTest(new test.sacs.DomainParticipantFactory4());
            suite.AddTest(new test.sacs.DomainParticipantFactory5());
            suite.AddTest(new test.sacs.DomainParticipantFactory6());
            suite.AddTest(new test.sacs.DomainParticipantFactory7());
            suite.AddTest(new test.sacs.DomainParticipantFactory8());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
