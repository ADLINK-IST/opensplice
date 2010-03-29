namespace test.sacs
{
    /// <summary>Test DataReader Lookup.</summary>
    /// <remarks>Test DataReader Lookup.</remarks>
    public class Subscriber4 : Test.Framework.TestCase
    {
        /// <summary>Test DataReader Lookup.</summary>
        /// <remarks>Test DataReader Lookup.</remarks>
        public Subscriber4()
            : base("sacs_subscriber_tc4", "sacs_subscriber", "subscriber",
                "Test DataReader Lookup.", "Test DataReader Lookup.", null)
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPreItem(new test.sacs.SubscriberItem2Init());
            this.AddPreItem(new test.sacs.SubscriberItem3Init());
            this.AddPostItem(new test.sacs.SubscriberItem3Deinit());
            this.AddPostItem(new test.sacs.SubscriberItem2Deinit());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ISubscriber subscriber;
			DDS.DataReaderQos dataReaderQos = null;
			DDS.DataReaderQos qosHolder = null;
            DDS.IDataReader reader1;
            DDS.IDataReader reader2;
            DDS.IDataReader reader3;
            DDS.IDataReader lookedUpReader;
            DDS.ITopic topic1;
            DDS.ITopic topic2;
            bool isReader1LookedUp;
            string expResult = "Lookup DataReader test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic1 = (DDS.ITopic)this.ResolveObject("topic");
            topic2 = (DDS.ITopic)this.ResolveObject("otherTopic");

            rc = subscriber.GetDefaultDataReaderQos(ref qosHolder);
            dataReaderQos = qosHolder;
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve default DataReaderQos (1).";
                return result;
            }
            reader1 = subscriber.CreateDataReader(topic1, dataReaderQos);//, null, 0);
            if (reader1 == null)
            {
                result.Result = "Could not create a DataReader (2).";
                return result;
            }
            reader2 = subscriber.CreateDataReader(topic2, dataReaderQos);//, null, 0);
            if (reader2 == null)
            {
                result.Result = "Could not create a DataReader (3).";
                return result;
            }
            reader3 = subscriber.CreateDataReader(topic1, dataReaderQos);//, null, 0);
            if (reader3 == null)
            {
                result.Result = "Could not create a DataReader (4).";
                return result;
            }
            lookedUpReader = subscriber.LookupDataReader("my_topic");
            if (lookedUpReader != reader1 && lookedUpReader != reader3)
            {
                result.Result = "Failed to lookup a DataReader (5).";
                return result;
            }
            isReader1LookedUp = lookedUpReader == reader1 ? true : false;
            lookedUpReader = subscriber.LookupDataReader("my_other_topic");
            if (lookedUpReader != reader2)
            {
                result.Result = "Failed to lookup a DataReader (6).";
                return result;
            }
            lookedUpReader = subscriber.LookupDataReader("non_existing_name");
            if (lookedUpReader != null)
            {
                result.Result = "Looked up a DataReader for a non existing Topic name (7).";
                return result;
            }
            if (isReader1LookedUp)
            {
                rc = subscriber.DeleteDataReader(reader1);
            }
            else
            {
                rc = subscriber.DeleteDataReader(reader3);
            }
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete a DataReader (RETCODE = " + rc + ")(8).";
                return result;
            }
            lookedUpReader = subscriber.LookupDataReader("my_topic");
            if (isReader1LookedUp)
            {
                if (lookedUpReader != reader3)
                {
                    result.Result = "Failed to lookup a DataReader (reader3) (9).";
                    return result;
                }
            }
            else
            {
                if (lookedUpReader != reader1)
                {
                    result.Result = "Failed to lookup a DataReader (reader1) (9).";
                    return result;
                }
            }
            if (isReader1LookedUp)
            {
                rc = subscriber.DeleteDataReader(reader3);
            }
            else
            {
                rc = subscriber.DeleteDataReader(reader1);
            }
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete a DataReader (RETCODE = " + rc + ")(10).";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete a DataReader (RETCODE = " + rc + ")(11).";
                return result;
            }
            lookedUpReader = subscriber.LookupDataReader(null);
            if (lookedUpReader != null)
            {
                result.Result = "Looked up a DataReader for an empty Topic name (12).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
