namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Status1 : Test.Framework.TestCase
    {
        public Status1()
            : base("sacs_status_tc1", "sacs_status", "status", "Test if Topic statusses work correctly."
                , "Test if Topic statusses work correctly.", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ITopic topic;
            DDS.InconsistentTopicStatus status = new DDS.InconsistentTopicStatus();
            string expResult = "Topic status test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            topic = (DDS.ITopic)this.ResolveObject("topic");

            rc = topic.GetInconsistentTopicStatus(ref status);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "InconsistentTopicStatus could not be resolved.";
                return result;
            }
            if (status.TotalCount != 0)
            {
                result.Result = "InconsistentTopicStatus.TotalCount != 0.";
                return result;
            }
            if (status.TotalCountChange != 0)
            {
                result.Result = "InconsistentTopicStatus.TotalCountChange != 0.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
