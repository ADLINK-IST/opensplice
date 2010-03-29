namespace test.sacs
{
    /// <summary>Test function get_datareaders.</summary>
    /// <remarks>Test function get_datareaders.</remarks>
    public class Subscriber6 : Test.Framework.TestCase
    {
        /// <summary>Test function get_datareaders.</summary>
        /// <remarks>Test function get_datareaders.</remarks>
        public Subscriber6()
            : base("sacs_subscriber_tc6", "sacs_subscriber", "subscriber",
                "Test function get_datareaders.", "Test function get_datareaders.", null)
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPreItem(new test.sacs.SubscriberItem2Init());
            this.AddPostItem(new test.sacs.SubscriberItem2Deinit());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ISubscriber subscriber;
			DDS.DataReaderQos dataReaderQos = null;
			DDS.IDataReader[] dataReaderSeqHolder = null;
            DDS.IDataReader reader1;
            DDS.IDataReader reader2;
            DDS.IDataReader reader3;
            DDS.ITopic topic;
            string expResult = "get_datareaders test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic = (DDS.ITopic)this.ResolveObject("topic");

            if (subscriber.GetDefaultDataReaderQos(ref dataReaderQos) != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve default DataReaderQos (1).";
                return result;
            }
            reader1 = subscriber.CreateDataReader(topic, dataReaderQos);//, null, 0);
            if (reader1 == null)
            {
                result.Result = "Could not create a DataReader (2).";
                return result;
            }
            reader2 = subscriber.CreateDataReader(topic, dataReaderQos);//, null, 0);
            if (reader2 == null)
            {
                result.Result = "Could not create a DataReader (3).";
                return result;
            }
            reader3 = subscriber.CreateDataReader(topic, dataReaderQos);//, null, 0);
            if (reader3 == null)
            {
                result.Result = "Could not create a DataReader (4).";
                return result;
            }

            rc = subscriber.GetDataReaders(ref dataReaderSeqHolder, DDS.SampleStateKind.Any,
                DDS.ViewStateKind.Any, DDS.InstanceStateKind.Alive);
            if (rc != DDS.ReturnCode.Unsupported)
            {
                result.Result = "Expected RETCODE_UNSUPPORTED but received " + rc + " after calling function get_datareaders (5).";
                result.ExpectedVerdict = Test.Framework.TestVerdict.Fail;
                return result;
            }
            //@todo add extra tests for get_datareaders
            rc = subscriber.DeleteDataReader(reader1);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but received " + rc + " after deleting a datareader (6).";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader2);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but received " + rc + " after deleting a datareader (7).";
                return result;
            }
            rc = subscriber.DeleteDataReader(reader3);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Expected RETCODE_OK but received " + rc + " after deleting a datareader (8).";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
