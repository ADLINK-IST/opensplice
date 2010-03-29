namespace test
{
    /// <date>May 24, 2005</date>
    public class SACSTesterParticipant
    {
        /// <exception cref="System.Exception"></exception>
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.DomainParticipant1());
            suite.AddTest(new test.sacs.DomainParticipant2());
            suite.AddTest(new test.sacs.DomainParticipant3());
            suite.AddTest(new test.sacs.DomainParticipant4());
            suite.AddTest(new test.sacs.DomainParticipant5());
            suite.AddTest(new test.sacs.DomainParticipant6());
            suite.AddTest(new test.sacs.DomainParticipant7());
            suite.AddTest(new test.sacs.DomainParticipant8());
            suite.AddTest(new test.sacs.DomainParticipant9());
            suite.AddTest(new test.sacs.DomainParticipant10());
            suite.AddTest(new test.sacs.DomainParticipant11());
            suite.AddTest(new test.sacs.DomainParticipant12());
            suite.AddTest(new test.sacs.DomainParticipant13());
            suite.AddTest(new test.sacs.DomainParticipant14());
            suite.AddTest(new test.sacs.DomainParticipant15());
            suite.AddTest(new test.sacs.DomainParticipant16());
            suite.AddTest(new test.sacs.DomainParticipant17());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }

    internal class Utils
    {
        internal static void FillStringArray(ref string[] arr, string val)
        {
            for (int i = 0; i < arr.Length; i++)
            {
                arr[i] = val;
            }
        }
    }
}
