namespace test.sacs
{
    /// <summary>Test DataReader Lookup.</summary>
    /// <remarks>Test DataReader Lookup.</remarks>
    public class Subscriber9 : Test.Framework.TestCase
    {
        /// <summary>Test DataReader Lookup.</summary>
        /// <remarks>Test DataReader Lookup.</remarks>
        public Subscriber9()
            : base("sacs_subscriber_tc9", "sacs_subscriber", "subscriber",
                "Check if default DataReadearQos is used when DATAREADER_QOS_DEFAULT are specified."
                , "Check if default DataReadearQos is used when DATAREADER_QOS_DEFAULT are specified."
                , null)
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPreItem(new test.sacs.SubscriberItem2Init());
            this.AddPostItem(new test.sacs.SubscriberItem2Deinit());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ISubscriber subscriber;
            DDS.DataReaderQos dataReaderQos;
			DDS.DataReaderQos qosHolder1 = null;
			DDS.DataReaderQos qosHolder2 = null;
            DDS.IDataReader reader;
            DDS.ITopic topic;
            string expResult = "Default DataReadearQos is used when DATAREADER_QOS_DEFAULT is specified.";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topic = (DDS.ITopic)this.ResolveObject("topic");

            if (subscriber.GetDefaultDataReaderQos(ref qosHolder1) != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve default DataReaderQos";
                return result;
            }

            dataReaderQos = qosHolder1;
            dataReaderQos.History.Kind = DDS.HistoryQosPolicyKind.KeepAllHistoryQos;
            dataReaderQos.History.Depth = 150;
            subscriber.SetDefaultDataReaderQos(dataReaderQos);

            reader = subscriber.CreateDataReader(topic);
            if (reader == null)
            {
                result.Result = "Could not create a DataReader.";
                return result;
            }

            if (reader.GetQos(ref qosHolder2) != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve DataReader qos";
                return result;
            }
            
            if (!test.sacs.QosComparer.DataReaderQosEquals(qosHolder1, qosHolder2))
            {
                result.Result = "Default DataReadearQos is not used when DATAREADER_QOS_DEFAULT is specified." + qosHolder2.History.Kind + "  ::  " + qosHolder1.History.Kind;
                return result;
            }
            rc = subscriber.DeleteDataReader(reader);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete a DataReader (RETCODE = " + rc + ").";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
