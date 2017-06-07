/// <date>May 24, 2005</date>
public class defaultConstructor
{
    /// <exception cref="System.Exception"></exception>
    public static void Main(string[] args)
    {
        Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
        suite.AddMilestone("Test start");
        suite.AddTest(new saj.testA());
#if OSPL_UNION_SUPPORTED
        suite.AddTest(new saj.testTestUnionStarts());
        suite.AddTest(new saj.testStructBigTest());
#endif
        suite.AddMilestone("Test end");
        suite.RunTests();
        suite.PrintReport();
    }
}
