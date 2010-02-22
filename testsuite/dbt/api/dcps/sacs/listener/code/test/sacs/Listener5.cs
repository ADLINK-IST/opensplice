namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Listener5 : Test.Framework.TestCase
    {
        public Listener5()
            : base("sacs_listener_tc5", "sacs_listener", "listener", "Test if a TopicListener works."
                , "Test if a TopicListener works.", null)
        {
            this.AddPreItem(new test.sacs.ListenerInit());
            this.AddPostItem(new test.sacs.ListenerDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ITopic topic;
            Test.Framework.TestResult result;
            test.sacs.MyTopicListener listener;
            string expResult = "TopicListener test succeeded.";
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            topic = (DDS.ITopic)this.ResolveObject("topic");
            listener = new test.sacs.MyTopicListener();
            rc = topic.SetListener(listener, DDS.StatusKind.InconsistentTopic);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "set_listener on Topic failed.";
                return result;
            }
            rc = topic.SetListener(null, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Null Listener could not be attached.";
                return result;
            }
            rc = topic.SetListener(listener, DDS.StatusKind.InconsistentTopic);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (2).";
                return result;
            }
            rc = topic.SetListener(listener, DDS.StatusKind.InconsistentTopic);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (3).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
