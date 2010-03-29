/// <date>May 24, 2005</date>
public class defaultConstructor
{
    /// <exception cref="System.Exception"></exception>
    public static void Main(string[] args)
    {
        Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
        suite.AddMilestone("Test start");
        suite.AddTest(new saj.testA());
        suite.AddTest(new saj.testTestUnionStarts());
        suite.AddTest(new saj.testStructBigTest());
        suite.AddMilestone("Test end");
        suite.RunTests();
        suite.PrintReport();
    }
}
