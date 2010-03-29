namespace test.sacs
{
    /// <summary>Test DataWriter Lookup.</summary>
    /// <remarks>Test DataWriter Lookup.</remarks>
    public class Publisher7 : Test.Framework.TestCase
    {
        /// <summary>Test DataWriter Lookup.</summary>
        /// <remarks>Test DataWriter Lookup.</remarks>
        public Publisher7()
            : base("sacs_publisher_tc7", "sacs_publisher", "publisher", "Check if default DataWriterQos is used when DATAWRITER_QOS_DEFAULT are specified."
                , "Check if default DataWriterQos is used when DATAWRITER_QOS_DEFAULT are specified."
                , null)
        {
            this.AddPreItem(new test.sacs.PublisherItemInit());
            this.AddPreItem(new test.sacs.TopicInit());
            this.AddPostItem(new test.sacs.TopicDeinit());
            this.AddPostItem(new test.sacs.PublisherItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IPublisher publisher;
			DDS.DataWriterQos dataWriterQos = null;
			DDS.DataWriterQos qosHolder1 = null;
			DDS.DataWriterQos qosHolder2 = null;
            DDS.IDataWriter writer = null;
            DDS.ITopic topic;
            string expResult = "Default DataWriterQos is used when DATAWRITER_QOS_DEFAULT is specified.";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            topic = (DDS.ITopic)this.ResolveObject("topic");

            if (publisher.GetDefaultDataWriterQos(ref qosHolder1) != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve default DataWriterQos";
                return result;
            }
            dataWriterQos = qosHolder1;
            dataWriterQos.History.Kind = DDS.HistoryQosPolicyKind.KeepAllHistoryQos;
            dataWriterQos.History.Depth = 150;
            publisher.SetDefaultDataWriterQos(dataWriterQos);

            //TODO: JLS, DDS.DataWriterQos.Default does not exist
            writer = publisher.CreateDataWriter(topic, qosHolder1);
            if (writer == null)
            {
                result.Result = "Could not create a DataWriter.";
                return result;
            }

            if (writer.GetQos(ref qosHolder2) != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve DataWriter qos";
                return result;
            }
            if (!test.sacs.QosComparer.DataWriterQosEquals(qosHolder1, qosHolder2))
            {
                result.Result = "Default DataWriterQos is not used when DATAWRITER_QOS_DEFAULT is specified.";
                return result;
            }
            rc = publisher.DeleteDataWriter(writer);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not delete a DataWriter (RETCODE = " + rc + ").";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
