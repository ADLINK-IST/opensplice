namespace test.sacs
{
    /// <summary>Test DataReader Lookup.</summary>
    /// <remarks>Test DataReader Lookup.</remarks>
    public class Subscriber10 : Test.Framework.TestCase
    {
        /// <summary>Test DataReader Lookup.</summary>
        /// <remarks>Test DataReader Lookup.</remarks>
        public Subscriber10()
            : base("sacs_subscriber_tc10", "sacs_subscriber", "subscriber"
                , "Check if copy_from_topic_qos rejects TOPIC_QOS_DEFAULT with correct code.", "Check if copy_from_topic_qos rejects TOPIC_QOS_DEFAULT with correct code."
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
			DDS.DataReaderQos dataReaderQos = null;
            DDS.IDomainParticipant participant;
            DDS.DataReaderQos qosHolder1 = null;
            DDS.ITopic topic;
            string expResult = "copy_from_topic_qos rejects TOPIC_QOS_DEFAULT with correct code.";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc = DDS.ReturnCode.Error;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            topic = (DDS.ITopic)this.ResolveObject("topic");

            if (subscriber.GetDefaultDataReaderQos(ref dataReaderQos) != DDS.ReturnCode.Ok)
            {
                result.Result = "Could not retrieve default DataReaderQos";
                return result;
            }

            dataReaderQos.History.Kind = DDS.HistoryQosPolicyKind.KeepAllHistoryQos;
            dataReaderQos.History.Depth = 150;

            // TODO: JLS, Verify the intent of this BadParameter test.
            DDS.TopicQos topicQosHolder = null;
            rc = subscriber.CopyFromTopicQos(ref qosHolder1, topicQosHolder);
            if (rc != DDS.ReturnCode.BadParameter)
            {
                result.Result = "copy_from_topic_qos returns wrong code (RETCODE = " + rc + ").";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
