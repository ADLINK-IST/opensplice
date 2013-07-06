namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant7 : Test.Framework.TestCase
    {
        public DomainParticipant7()
            : base("sacs_domainParticipant_tc7", "sacs_domainParticipant"
                , "domainParticipant", "Topic test.", "Topic test.", null)
        {
            this.AddPreItem(new test.sacs.DomainParticipantItemInit());
            this.AddPostItem(new test.sacs.DomainParticipantItemDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant participant2;
            DDS.DomainParticipantFactory factory;
			DDS.DomainParticipantQos qos = null;
            string expResult = "Topic test succeeded.";
			DDS.TopicQos topQosHolder1 = null;
			DDS.TopicQos topQosHolder2 = null;
            mod.tstTypeSupport typeSupport;
            DDS.ITopic topic;
            DDS.ITopic topic2;
            DDS.ITopic topic3;
            DDS.ITopic topic4;
            DDS.ITopicDescription description;
            DDS.ReturnCode returnCode;
            factory = (DDS.DomainParticipantFactory)this.ResolveObject("factory");
            qos = (DDS.DomainParticipantQos)this.ResolveObject("participantQos");
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            typeSupport = new mod.tstTypeSupport();
            typeSupport.RegisterType(participant, "My_Type");

            if (participant.GetDefaultTopicQos(ref topQosHolder1) != DDS.ReturnCode.Ok)
            {
                result.Result = "Get default TopicQos failed (1).";
                return result;
            }
            if (!test.sacs.QosComparer.TopicQosEquals(topQosHolder1, test.sacs.QosComparer.defaultTopicQos))
            {
                result.Result = "Get default TopicQos did not return the default qos (1).";
                return result;
            }
            topQosHolder1.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
            topQosHolder1.Liveliness.Kind = DDS.LivelinessQosPolicyKind.AutomaticLivelinessQos;
            topQosHolder1.Reliability.Kind = DDS.ReliabilityQosPolicyKind.ReliableReliabilityQos;
            topQosHolder1.DestinationOrder.Kind = DDS.DestinationOrderQosPolicyKind.ByReceptionTimestampDestinationorderQos;
            topQosHolder1.History.Kind = DDS.HistoryQosPolicyKind.KeepAllHistoryQos;
            topQosHolder1.Ownership.Kind = DDS.OwnershipQosPolicyKind.SharedOwnershipQos;
            typeSupport = new mod.tstTypeSupport();
            returnCode = typeSupport.RegisterType(participant, "myTopicType");
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Register type failed.";
                return result;
            }
            topic = participant.CreateTopic("MyDCPSTopic", "myTopicType", topQosHolder1);//, null, 0);
            if (topic == null)
            {
                result.Result = "Create topic failed (3)";
                return result;
            }
            description = participant.LookupTopicDescription("MyDCPSTopic");
            if (topic != description)
            {
                result.Result = "Lookup topic failed (4)";
                return result;
            }
            topic2 = participant.FindTopic("MyDCPSTopic", new DDS.Duration());
            if (topic2 == null)
            {
                result.Result = "Find topic failed (5)";
                return result;
            }
            topic2.GetQos(ref topQosHolder2);
            if (!test.sacs.QosComparer.TopicQosEquals(topQosHolder1, topQosHolder2))
            {
                result.Result = "Find Topic did not find the correct Topic. (5)";
                return result;
            }
            topic3 = participant.CreateTopic("MyDCPSTopic", "myTopicType");
            if (topic3 != null)
            {
                result.Result = "Create Topic with BAD_PARAM succeeded (6).";
                return result;
            }
            topic4 = participant.CreateTopic("MyDCPSTopic", "myTopicType");
            if (topic4 != null)
            {
                result.Result = "Create Topic with BAD_PARAM succeeded (7).";
                return result;
            }
            returnCode = participant.DeleteTopic(topic3);
            if (returnCode != DDS.ReturnCode.BadParameter)
            {
                result.Result = "Expected RETCODE_BAD_PARAMETER but received " + returnCode + " after deleting a non exisiting topic (8).";
                return result;
            }
            returnCode = participant.DeleteTopic(topic4);
            if (returnCode != DDS.ReturnCode.BadParameter)
            {
                result.Result = "Expected RETCODE_BAD_PARAMETER but received " + returnCode + " after deleting a non exisiting topic (9).";
                return result;
            }
            participant2 = factory.CreateParticipant(DDS.DomainId.Default, qos);//, null, 0);
            if (participant2 == null)
            {
                result.Result = "Create Participant failed (10).";
                return result;
            }
            returnCode = participant2.DeleteTopic(topic);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Topic on wrong Participant succeeded (11).";
                return result;
            }
            returnCode = factory.DeleteParticipant(participant2);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Participant failed (12).";
                return result;
            }
            returnCode = participant.DeleteTopic(topic);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Topic failed (13).";
                return result;
            }
            returnCode = participant.DeleteTopic(topic2);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result.Result = "Delete Topic failed (14).";
                return result;
            }
            returnCode = participant.DeleteTopic(topic);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Delete of already deleted Topic succeeded (15).";
                return result;
            }
            returnCode = participant.DeleteTopic(null);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Delete null Topic succeeded (16).";
                return result;
            }
            returnCode = participant.IgnoreParticipant(-10);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Ignore invalid participant succeeded.";
                return result;
            }
            returnCode = participant.IgnorePublication(-10);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Ignore invalid publication succeeded.";
                return result;
            }
            returnCode = participant.IgnoreSubscription(-10);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Ignore invalid subscription succeeded.";
                return result;
            }
            returnCode = participant.IgnoreTopic(-10);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result.Result = "Ignore invalid topic succeeded.";
                return result;
            }
            result.Verdict = Test.Framework.TestVerdict.Pass;
            result.Result = expResult;
            return result;
        }
    }
}
