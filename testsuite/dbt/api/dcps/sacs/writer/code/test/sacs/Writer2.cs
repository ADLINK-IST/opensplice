namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Writer2 : Test.Framework.TestCase
    {
        public Writer2()
            : base("sacs_writer_tc2", "sacs_writer", "sacs_writer", "test untyped datawriter actions"
                , "test untyped datawriter actions", null)
        {
            this.AddPreItem(new test.sacs.WriterInit());
            this.AddPostItem(new test.sacs.WriterDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDataWriter writer;
            DDS.ITopic topic;
            DDS.ITopic topic2;
            DDS.IPublisher publisher;
            DDS.IPublisher publisher2;
            DDS.InstanceHandle[] handles;
			DDS.SubscriptionBuiltinTopicData data = null;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            string expResult = "All Functions supported.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            writer = (DDS.IDataWriter)this.ResolveObject("datawriter");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            topic2 = writer.Topic;
            if (topic != topic2)
            {
                result.Result = "get_topic resolved wrong topic.";
                return result;
            }
            publisher2 = writer.Publisher;
            if (publisher != publisher2)
            {
                result.Result = "get_publisher resolved wrong publisher.";
                return result;
            }
            writer.AssertLiveliness();
            System.Threading.Thread.Sleep(1000); // Give writer some time to discover reader.
            handles = new DDS.InstanceHandle[0];
            rc = writer.GetMatchedSubscriptions(ref handles);
            if (rc != DDS.ReturnCode.Ok || handles.Length != 1)
            {
                result.Result = "get_matched_subscriptions failed.";
                return result;
            } 
            else
            { 
	
	            rc = writer.GetMatchedSubscriptionData(ref data, handles[0]);
	            if (rc != DDS.ReturnCode.Ok)
	            {
	                result.Result = "get_matched_subscription_data failed.";
	                return result;
	            }
	        }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
