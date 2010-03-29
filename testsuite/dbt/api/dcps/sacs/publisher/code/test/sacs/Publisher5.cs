namespace test.sacs
{
    /// <summary>
    /// Tests the initial default DataWriterQos and the getting and setting
    /// of the default DataWriterQos in the Publisher.
    /// </summary>
    /// <remarks>
    /// Tests the initial default DataWriterQos and the getting and setting
    /// of the default DataWriterQos in the Publisher.
    /// </remarks>
    public class Publisher5 : Test.Framework.TestCase
    {
        /// <summary>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </summary>
        /// <remarks>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </remarks>
        public Publisher5()
            : base("sacs_publisher_tc5", "sacs_publisher", "publisher", "Test if copy_from_topic_qos."
                , "Test if copy_from_topic_qos.", null)
        {
            this.AddPreItem(new test.sacs.PublisherItemInit());
            this.AddPreItem(new test.sacs.TopicInit());
            this.AddPostItem(new test.sacs.TopicDeinit());
            this.AddPostItem(new test.sacs.PublisherItemDeinit());
        }

        /// <summary>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </summary>
        /// <remarks>
        /// Tests the initial default DataWriterQos and the getting and setting
        /// of the default DataWriterQos in the Publisher.
        /// </remarks>
        public override Test.Framework.TestResult Run()
        {
            DDS.IPublisher publisher;
			DDS.DataWriterQos qosHolder = null;
			DDS.TopicQos topicQos = null;
            string expResult = "copy_from_topic_qos test succeeded";
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            topicQos = (DDS.TopicQos)this.ResolveObject("topicQos");

            rc = publisher.CopyFromTopicQos(ref qosHolder, topicQos);
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
            if (!test.sacs.QosComparer.LifespanQosPolicyEquals(qosHolder.Lifespan, topicQos.Lifespan))
            {
                result.Result = "lifespanQosPolicy not valid.";
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
            if (!test.sacs.QosComparer.TransportPriorityQosPolicyEquals(qosHolder.TransportPriority, topicQos.TransportPriority))
            {
                result.Result = "transportPriorityQosPolicy not valid.";
                return result;
            }
            byte[] bytes = new byte[] { 1, 2, 3 };
            DDS.UserDataQosPolicy udp = new DDS.UserDataQosPolicy();
            udp.Value = bytes;
            qosHolder.UserData.Value = bytes;
            DDS.OwnershipStrengthQosPolicy osp = new DDS.OwnershipStrengthQosPolicy();
            osp.Value = 11;
            qosHolder.OwnershipStrength.Value = 11;
            DDS.WriterDataLifecycleQosPolicy wlp = new DDS.WriterDataLifecycleQosPolicy();
            wlp.AutodisposeUnregisteredInstances = true;
            qosHolder.WriterDataLifecycle.AutodisposeUnregisteredInstances = true;

            DDS.DataWriterQos wqos = qosHolder;
            rc = publisher.CopyFromTopicQos(ref qosHolder, topicQos);
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
            if (!test.sacs.QosComparer.OwnershipStrengthQosPolicyEquals(qosHolder.OwnershipStrength, osp))
            {
                result.Result = "ownershipStrengthQosPolicy not valid.";
                return result;
            }
            if (!test.sacs.QosComparer.WriterDataLifecycleQosPolicyEquals(qosHolder.WriterDataLifecycle, wlp))
            {
                result.Result = "writerDataLifecycleQosPolicy not valid.";
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
            if (!test.sacs.QosComparer.LifespanQosPolicyEquals(qosHolder.Lifespan, topicQos.Lifespan))
            {
                result.Result = "lifespanQosPolicy not valid.";
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
            if (!test.sacs.QosComparer.TransportPriorityQosPolicyEquals(qosHolder.TransportPriority, topicQos.TransportPriority))
            {
                result.Result = "transportPriorityQosPolicy not valid.";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
