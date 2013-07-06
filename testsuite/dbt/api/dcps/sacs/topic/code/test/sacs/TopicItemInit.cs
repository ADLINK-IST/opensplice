namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class TopicItemInit : Test.Framework.TestItem
    {
        public TopicItemInit()
            : base("Initialize Topic")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.TopicQos topQosHolder = null;
            DDS.ITopic topic;
            mod.tstTypeSupport typeSupport = null;
            Test.Framework.TestResult result;
            string name;
            string typeName;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result.Result = "DomainParticipantFactory could not be initialized.";
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref pqosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DomainParticipantQos could not be resolved.";
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant == null)
            {
                result.Result = "Creation of DomainParticipant failed.";
                return result;
            }
            typeSupport = new mod.tstTypeSupport();
            if (typeSupport == null)
            {
                result.Result = "Creation of tstTypeSupport failed.";
                return result;
            }
            name = "my_topic";
            typeName = "my_type";
            rc = typeSupport.RegisterType(participant, typeName);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Register type failed.";
                return result;
            }

            if (participant.GetDefaultTopicQos(ref topQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default TopicQos could not be resolved.";
                return result;
            }
            topic = participant.CreateTopic(name, typeName, topQosHolder);//, null, 0);
            if (topic == null)
            {
                result.Result = "Topic could not be created.";
                return result;
            }
            testCase.RegisterObject("factory", factory);
            testCase.RegisterObject("participantQos", pqosHolder);
            testCase.RegisterObject("participant", participant);
            testCase.RegisterObject("topic", topic);
            testCase.RegisterObject("topicQos", topQosHolder);
            testCase.RegisterObject("typeSupport", typeSupport);
            testCase.RegisterObject("topicName", name);
            testCase.RegisterObject("topicTypeName", typeName);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
