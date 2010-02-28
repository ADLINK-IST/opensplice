namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant9 : Test.Framework.TestCase
    {
        public DomainParticipant9()
            : base("sacs_domainParticipant_tc9", "sacs_domainParticipant"
                , "domainParticipant", "set/get default_qos test.", "set/get default_qos test.",
                null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            string expResult = "set/get default_qos test succeeded.";
            DDS.ReturnCode rc;
            DDS.IDomainParticipant participant;
            DDS.PublisherQos pholder;
            DDS.SubscriberQos sholder;
            DDS.TopicQos tholder;
            DDS.PublisherQos pqos;
            DDS.SubscriberQos sqos;
            DDS.TopicQos tqos;
            DDS.PartitionQosPolicy pqp;
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            pholder = new DDS.PublisherQos();
            sholder = new DDS.SubscriberQos();
            tholder = new DDS.TopicQos();

            if (participant.GetDefaultPublisherQos(ref pholder) != DDS.ReturnCode.Ok)
            {
                result.Result = "get_default_publisher_qos failed.";
                return result;
            }
            pqos = pholder;
            string[] partitions = new string[2];
            partitions[0] = "partition";
            partitions[1] = "partition2";
            pqp = new DDS.PartitionQosPolicy();
            pqp.Name = partitions;

            pqos.Partition = pqp;
            rc = participant.SetDefaultPublisherQos(pqos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "set_default_publisher_qos failed (1).";
                return result;
            }

            if (participant.GetDefaultPublisherQos(ref pholder) != DDS.ReturnCode.Ok)
            {
                result.Result = "get_default_publisher_qos failed (2).";
                return result;
            }
            if (!test.sacs.QosComparer.PublisherQosEquals(pqos, pholder))
            {
                result.Result = "resolved qos does not match the applied one.";
                return result;
            }

            if (participant.GetDefaultSubscriberQos(ref sholder) != DDS.ReturnCode.Ok)
            {
                result.Result = "get_default_subscriber_qos failed.";
                return result;
            }
            sqos = sholder;
            sqos.Partition = pqp;
            rc = participant.SetDefaultSubscriberQos(sqos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "set_default_subscriber_qos failed (1).";
                return result;
            }

            if (participant.GetDefaultSubscriberQos(ref sholder) != DDS.ReturnCode.Ok)
            {
                result.Result = "get_default_subscriber_qos failed (2).";
                return result;
            }
            if (!test.sacs.QosComparer.SubscriberQosEquals(sqos, sholder))
            {
                result.Result = "resolved qos does not match the applied one (2).";
                return result;
            }

            if (participant.GetDefaultTopicQos(ref tholder) != DDS.ReturnCode.Ok)
            {
                result.Result = "get_default_topic_qos failed.";
                return result;
            }
            tqos = tholder;
            tqos.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;

            rc = participant.SetDefaultTopicQos(tqos);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "set_default_topic_qos failed (1).";
                return result;
            }

            if (participant.GetDefaultTopicQos(ref tholder) != DDS.ReturnCode.Ok)
            {
                result.Result = "get_default_topic_qos failed (2).";
                return result;
            }
            if (!test.sacs.QosComparer.TopicQosEquals(tqos, tholder))
            {
                result.Result = "resolved qos does not match the applied one (3).";
                return result;
            }
            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
