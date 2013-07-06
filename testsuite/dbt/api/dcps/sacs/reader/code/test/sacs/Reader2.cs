namespace test.sacs
{
    /// <date>Jun 20, 2005</date>
    public class Reader2 : Test.Framework.TestCase
    {
        public Reader2()
            : base("sacs_reader_tc2", "sacs_reader", "sacs_reader", "test untyped datareader actions"
                , "test untyped datareader actions", null)
        {
            this.AddPreItem(new test.sacs.ReaderInit());
            this.AddPostItem(new test.sacs.ReaderDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDataReader reader;
            DDS.ITopicDescription topic;
            DDS.ITopicDescription topic2;
            DDS.ISubscriber subscriber;
            DDS.ISubscriber subscriber2;
            Test.Framework.TestResult result;
			DDS.InstanceHandle[] handles = null;
			DDS.PublicationBuiltinTopicData data = null;
            DDS.ReturnCode rc;
            string expResult = "All Functions supported.";
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            reader = (DDS.IDataReader)this.ResolveObject("datareader");
            topic = (DDS.ITopicDescription)this.ResolveObject("topic");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic2 = reader.GetTopicDescription();
            if (topic != topic2)
            {
                result.Result = "get_topicdescription resolved wrong topic.";
                return result;
            }
            subscriber2 = reader.Subscriber;
            if (subscriber != subscriber2)
            {
                result.Result = "get_subscriber resolved wrong subscriber.";
                return result;
            }
            rc = reader.WaitForHistoricalData(new DDS.Duration(3, 0));
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "wait_for_historical_data failed.";
                return result;
            }
            rc = reader.GetMatchedPublications(ref handles);
            if (rc != DDS.ReturnCode.Ok || handles.Length != 1)
            {
                result.Result = "get_matched_publications failed.";
                return result;
            } 
            else 
            {
	            rc = reader.GetMatchedPublicationData(ref data, handles[0]);
	            if (rc != DDS.ReturnCode.Ok)
	            {
	                result.Result = "get_matched_publication_data failed.";
	                return result;
	            }
	        }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
