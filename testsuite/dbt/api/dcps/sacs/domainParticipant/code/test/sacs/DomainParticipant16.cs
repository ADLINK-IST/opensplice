namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant16 : Test.Framework.TestCase
    {
        public DomainParticipant16()
            : base("sacs_domainParticipant_tc16", "sacs_domainParticipant"
                , "domainParticipant", "Test if default topicQos is used when TOPIC_QOS_DEFAULT is specified."
                , "Test if default topicQos is used when TOPIC_QOS_DEFAULT is specified.", null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            DDS.IDomainParticipant participant;
            DDS.DomainParticipantFactory factory;
            DDS.DomainParticipantQos qos;
            string expResult = "Default topicQos is used when TOPIC_QOS_DEFAULT is specified.";
            DDS.TopicQos topQosHolder1;
            DDS.TopicQos topQosHolder2;
            mod.tstTypeSupport typeSupport;
            DDS.ITopic topic;
            DDS.ReturnCode returnCode;
            factory = (DDS.DomainParticipantFactory)this.ResolveObject("factory");
            qos = (DDS.DomainParticipantQos)this.ResolveObject("participantQos");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            typeSupport = new mod.tstTypeSupport();
            topQosHolder1 = new DDS.TopicQos();
            topQosHolder2 = new DDS.TopicQos();
            typeSupport.RegisterType(participant, "My_Type");

            if (participant.GetDefaultTopicQos(ref topQosHolder1) != DDS.ReturnCode.Ok)
            {
                result.Result = "Get default TopicQos failed (1).";
                return result;
            }

            topQosHolder1.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
            topQosHolder1.Liveliness.Kind = DDS.LivelinessQosPolicyKind.AutomaticLivelinessQos;
            topQosHolder1.Reliability.Kind = DDS.ReliabilityQosPolicyKind.ReliableReliabilityQos;
            topQosHolder1.DestinationOrder.Kind = DDS.DestinationOrderQosPolicyKind.ByReceptionTimestampDestinationorderQos;
            topQosHolder1.History.Kind = DDS.HistoryQosPolicyKind.KeepAllHistoryQos;
            topQosHolder1.Ownership.Kind = DDS.OwnershipQosPolicyKind.SharedOwnershipQos;
            participant.SetDefaultTopicQos(topQosHolder1);
            typeSupport = new mod.tstTypeSupport();
            returnCode = typeSupport.RegisterType(participant, "myTopicType");
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Register type failed.";
                return result;
            }
            topic = participant.CreateTopic("MyDCPSTopic", "myTopicType");
            if (topic == null)
            {
                result.Result = "Create topic failed.";
                return result;
            }
            topic.GetQos(ref topQosHolder2);
            if (!test.sacs.QosComparer.TopicQosEquals(topQosHolder1, topQosHolder2))
            {
                result.Result = "Default topicQos is not used when TOPIC_QOS_DEFAULT is specified.";
                return result;
            }
            returnCode = participant.DeleteTopic(topic);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Topic failed.";
                return result;
            }
            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
