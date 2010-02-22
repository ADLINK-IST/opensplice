namespace test
{
    /// <date>Sep 5, 2009</date>
    public class SACSTesterBuiltin
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.Builtin1());
            suite.AddTest(new test.sacs.Builtin2());
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
