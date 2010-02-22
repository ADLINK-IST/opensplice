namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Waitset4 : Test.Framework.TestCase
    {
        public Waitset4()
            : base("sacs_waitset_tc4", "waitset", "waitset", "test topic status condition in waitset"
                , "test topic status condition in waitset", null)
        {
            this.AddPreItem(new test.sacs.WaitsetInit());
            this.AddPostItem(new test.sacs.WaitsetDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ITopic topic;
            DDS.IStatusCondition condition;
            DDS.WaitSet waitset;
            DDS.ICondition[] conditionHolder;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            string expResult = "StatusCondition test succeeded.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            topic = (DDS.ITopic)this.ResolveObject("topic");
            condition = topic.StatusCondition;
            if (condition == null)
            {
                result.Result = "Could not resolve participant condition.";
                return result;
            }
            waitset = new DDS.WaitSet();
            rc = waitset.AttachCondition(condition);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not attach condition.";
                return result;
            }
            conditionHolder = new DDS.ICondition[0];
            rc = waitset.Wait(ref conditionHolder, new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Timeout)
            {
                result.Result = "WaitSet.Wait failed.";
                return result;
            }
            if (conditionHolder.Length > 0)
            {
                result.Result = "WaitSet.Wait returned conditions where it shouldn't.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
