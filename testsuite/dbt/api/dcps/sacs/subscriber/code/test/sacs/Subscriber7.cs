namespace test.sacs
{
    /// <summary>Test function get_datareaders.</summary>
    /// <remarks>Test function get_datareaders.</remarks>
    public class Subscriber7 : Test.Framework.TestCase
    {
        public Subscriber7()
            : base("sacs_subscriber_tc7", "sacs_subscriber", "subscriber",
                "Test function copy_from_topic_qos.", "Test function copy_from_topic_qos.", null
                )
        {
            this.AddPreItem(new test.sacs.SubscriberItemInit());
            this.AddPreItem(new test.sacs.SubscriberItem2Init());
            this.AddPostItem(new test.sacs.SubscriberItem2Deinit());
            this.AddPostItem(new test.sacs.SubscriberItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.ISubscriber subscriber;
			DDS.DataReaderQos qosHolder = null;
			DDS.TopicQos topicQos = null;
            string expResult = "copy_from_topic_qos test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
            topicQos = (DDS.TopicQos)this.ResolveObject("topicQos");

            rc = subscriber.CopyFromTopicQos(ref qosHolder, topicQos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "copy_from_topic_qos failed.";
                return result;
            }

            if (!test.sacs.QosComparer.DeadlineQosPolicyEquals(qosHolder.Deadline, topicQos.Deadline))
            {
                result.Result = "deadLineQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.DestinationOrderQosPolicyEquals(qosHolder.DestinationOrder, topicQos.DestinationOrder))
            {
                result.Result = "destinationQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.DurabilityQosPolicyEquals(qosHolder.Durability, topicQos.Durability))
            {
                result.Result = "durabilityQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.HistoryQosPolicyEquals(qosHolder.History, topicQos.History))
            {
                result.Result = "historyQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.LatencyBudgetQosPolicyEquals(qosHolder.LatencyBudget, topicQos.LatencyBudget))
            {
                result.Result = "latencyBudgetQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.LivelinessQosPolicyEquals(qosHolder.Liveliness, topicQos.Liveliness))
            {
                result.Result = "livelinessQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.ReliabilityQosPolicyEquals(qosHolder.Reliability, topicQos.Reliability))
            {
                result.Result = "reliabilityQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.ResourceLimitsQosPolicyEquals(qosHolder.ResourceLimits, topicQos.ResourceLimits))
            {
                result.Result = "resourceLimitsQosPolicy not valid.";
                return result;
            }
            byte[] bytes = new byte[] { 1, 2, 3 };
            DDS.Duration duration = new DDS.Duration(10, 9);
            DDS.UserDataQosPolicy udp = new DDS.UserDataQosPolicy();
            udp.Value = bytes;
            qosHolder.UserData.Value = bytes;
            DDS.TimeBasedFilterQosPolicy tfp = new DDS.TimeBasedFilterQosPolicy();
            tfp.MinimumSeparation = duration;
            qosHolder.TimeBasedFilter.MinimumSeparation = duration;
            DDS.ReaderDataLifecycleQosPolicy rlp = new DDS.ReaderDataLifecycleQosPolicy();
            rlp.AutopurgeDisposedSamplesDelay = duration;
            rlp.AutopurgeNowriterSamplesDelay = duration;
            rlp.EnableInvalidSamples = true;
            qosHolder.ReaderDataLifecycle.AutopurgeDisposedSamplesDelay = duration;
            qosHolder.ReaderDataLifecycle.AutopurgeNowriterSamplesDelay = duration;
            qosHolder.ReaderDataLifecycle.EnableInvalidSamples = true;

            rc = subscriber.CopyFromTopicQos(ref qosHolder, topicQos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "copy_from_topic_qos failed.";
                return result;
            }

            if (!test.sacs.QosComparer.UserDataQosPolicyEquals(qosHolder.UserData, udp))
            {
                result.Result = "userDataQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.TimeBasedFilterQosPolicyEquals(qosHolder.TimeBasedFilter, tfp))
            {
                result.Result = "timeBasedFilterQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.ReaderDataLifecycleQosPolicyEquals(qosHolder.ReaderDataLifecycle, rlp))
            {
                result.Result = "readerDataLifecycleQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.DeadlineQosPolicyEquals(qosHolder.Deadline, topicQos.Deadline))
            {
                result.Result = "deadLineQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.DestinationOrderQosPolicyEquals(qosHolder.DestinationOrder, topicQos.DestinationOrder))
            {
                result.Result = "destinationQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.DurabilityQosPolicyEquals(qosHolder.Durability, topicQos.Durability))
            {
                result.Result = "durabilityQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.HistoryQosPolicyEquals(qosHolder.History, topicQos.History))
            {
                result.Result = "historyQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.LatencyBudgetQosPolicyEquals(qosHolder.LatencyBudget, topicQos.LatencyBudget))
            {
                result.Result = "latencyBudgetQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.LivelinessQosPolicyEquals(qosHolder.Liveliness, topicQos.Liveliness))
            {
                result.Result = "livelinessQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.ReliabilityQosPolicyEquals(qosHolder.Reliability, topicQos.Reliability))
            {
                result.Result = "reliabilityQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.ResourceLimitsQosPolicyEquals(qosHolder.ResourceLimits, topicQos.ResourceLimits))
            {
                result.Result = "resourceLimitsQosPolicy not valid.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
