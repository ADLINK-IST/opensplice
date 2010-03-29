namespace test.sacs
{
    /// <summary>Test function get_datareaders.</summary>
    /// <remarks>Test function get_datareaders.</remarks>
    public class Subscriber8 : Test.Framework.TestCase
    {
        public Subscriber8()
            : base("sacs_subscriber_tc8", "sacs_subscriber", "subscriber",
                "Additional subscriber test.", "Additional subscriber test.", null)
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPreItem(new test.sacs.SubscriberItem2Init());
            this.AddPostItem(new test.sacs.SubscriberItem2Deinit());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ISubscriber subscriber;
            string expResult = "Functions not supported yet.";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            subscriber.NotifyDataReaders();
            rc = subscriber.BeginAccess();
            if (rc != DDS.ReturnCode.Unsupported)
            {
                result.Result = "subscriber.begin_access has been implemented.";
                return result;
            }
            rc = subscriber.EndAccess();
            if (rc != DDS.ReturnCode.Unsupported)
            {
                result.Result = "subscriber.end_access has been implemented.";
                return result;
            }
            result.Result = "Functions not supported yet.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
