namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Listener4 : Test.Framework.TestCase
    {
        public Listener4()
            : base("sacs_listener_tc4", "sacs_listener", "listener", "Test if a PublisherListener works."
                , "Test if a PublisherListener works.", null)
        {
            this.AddPreItem(new test.sacs.ListenerInit());
            this.AddPostItem(new test.sacs.ListenerDeinit());
        }

        public override Test.Framework.TestResult Run()
        {

            DDS.IPublisher publisher;
            Test.Framework.TestResult result;
            test.sacs.MyPublisherListener listener;
            string expResult = "PublisherListener test succeeded.";
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            listener = new test.sacs.MyPublisherListener();

            rc = publisher.SetListener(listener, DDS.StatusKind.Any);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "set_listener on Publisher failed.";
                return result;
            }
            rc = publisher.SetListener(null, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Null Listener could not be attached.";
                return result;
            }
            rc = publisher.SetListener(listener, (DDS.StatusKind)1012131412);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Attaching a listener failed.";
                return result;
            }
            rc = publisher.SetListener(listener, 0);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Listener could not be attached (2).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
