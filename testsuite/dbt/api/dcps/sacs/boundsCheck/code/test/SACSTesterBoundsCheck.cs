namespace test
{
	public class SAJTesterBoundsCheck
	{
		public static void Main(string[] args)
		{
			Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
			suite.AddMilestone("Test start");
			suite.AddTest(new test.sacs.BoundsCheck1());
			suite.AddMilestone("Test end");
			suite.RunTests();
			suite.PrintReport();
		}
	}
}
