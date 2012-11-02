namespace test
{
    /// <date>May 24, 2005</date>
    public class SACSGetDiscoveredXxx
    {
        /// <exception cref="System.Exception"></exception>
        public static void Main(string[] args)
        {
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");
            suite.AddTest(new test.sacs.GetDiscoveredXxx());
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
