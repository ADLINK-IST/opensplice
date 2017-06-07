namespace test.sacs
{
    /// <summary>Initialize a DomainParticipant.</summary>
    /// <remarks>Initialize a DomainParticipant.</remarks>
    public class CopyInCopyOutInit : Test.Framework.TestItem
    {
        /// <summary>Initialize a DomainParticipant.</summary>
        /// <remarks>Initialize a DomainParticipant.</remarks>
        public CopyInCopyOutInit()
            : base("Initialize DomainParticipant")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
            Foo.TestTypeSupport typeSupport;
            DDS.TopicQos topicQos = null;
            DDS.ITopic topic;
            DDS.DomainParticipantQos pqos = null;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result.Result = "DomainParticipantFactory could not be initialized.";
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref pqos) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DomainParticipantQos could not be resolved.";
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqos);//, null, 0);
            if (participant == null)
            {
                result.Result = "Creation of DomainParticipant failed.";
                return result;
            }
            typeSupport = new Foo.TestTypeSupport();
            if (typeSupport == null)
            {
                result.Result = "Creation of Foo.TestTypeSupport failed.";
                return result;
            }
            rc = typeSupport.RegisterType(participant, "Foo::Test");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "register_type failed.";
                return result;
            }

            if (participant.GetDefaultTopicQos(ref topicQos) != DDS.ReturnCode.Ok)
            {
                result.Result = "participant.get_default_topic_qos failed.";
                return result;
            }
            topicQos.Reliability.Kind = DDS.ReliabilityQosPolicyKind.ReliableReliabilityQos;
            topicQos.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
            topic = participant.CreateTopic("my_topic", "Foo::Test", topicQos);//, null, 0);
            if (topic == null)
            {
                result.Result = "participant.create_topic failed.";
                return result;
            }
            testCase.RegisterObject("factory", factory);
            testCase.RegisterObject("participant", participant);
            testCase.RegisterObject("topic", topic);
            testCase.RegisterObject("topicQos", topicQos);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
