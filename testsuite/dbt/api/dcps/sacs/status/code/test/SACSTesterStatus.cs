namespace test
{
    /// <date>Jun 13, 2005</date>
    public class SACSTesterStatus
    {
        public static void Main(string[] args)
        {
            Test.Framework.TestCase tc;
            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            Test.Framework.TestItem pre = new test.sacs.StatusInit();
            Test.Framework.TestItem post = new test.sacs.StatusDeinit();
            suite.AddMilestone("Test start");
            tc = new test.sacs.Status1();
            tc.AddPreItem(pre);
            tc.AddPostItem(post);
            suite.AddTest(tc);
            tc = new test.sacs.Status2();
            tc.AddPreItem(pre);
            tc.AddPostItem(post);
            suite.AddTest(tc);
            tc = new test.sacs.Status3();
            tc.AddPreItem(pre);
            tc.AddPostItem(post);
            suite.AddTest(tc);
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
