namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class ListenerInit : Test.Framework.TestItem
    {
        public ListenerInit()
            : base("Initialize listener")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.SubscriberQos subQosHolder = null;
            DDS.ISubscriber subscriber;
			DDS.PublisherQos pubQosHolder = null;
            DDS.IPublisher publisher;
            mod.tstTypeSupport typeSupport;
			DDS.TopicQos tQosHolder = null;
            DDS.ITopic topic;
			DDS.DataReaderQos drQosHolder = null;
            mod.tstDataReader datareader;
            //mod.tstDataReader datareader2;
			DDS.DataWriterQos dwQosHolder = null;
            mod.tstDataWriter datawriter;
            Test.Framework.TestResult result;
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

            if (participant.GetDefaultSubscriberQos(ref subQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default SubscriberQos could not be resolved.";
                return result;
            }
            subscriber = participant.CreateSubscriber(subQosHolder);//, null, 0);
            if (subscriber == null)
            {
                result.Result = "Subscriber could not be created.";
                return result;
            }
            typeSupport = new mod.tstTypeSupport();
            rc = typeSupport.RegisterType(participant, "tstType");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Typesupport could not be registered.";
                return result;
            }

            if (participant.GetDefaultTopicQos(ref tQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default TopicQos could not be resolved.";
                return result;
            }
            topic = participant.CreateTopic("tst", "tstType", tQosHolder);//, null, 0);
            if (topic == null)
            {
                result.Result = "Topic could not be created.";
                return result;
            }

            if (subscriber.GetDefaultDataReaderQos(ref drQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DataReaderQos could not be resolved.";
                return result;
            }
            datareader = (mod.tstDataReader)subscriber.CreateDataReader(topic, drQosHolder);//, null, 0);
            if (datareader == null)
            {
                result.Result = "DataReader could not be created.";
                return result;
            }
            //datareader2 = (mod.tstDataReader)subscriber.CreateDataReader(topic, ref drQosHolder);//, null, 0);
            //if (datareader2 == null)
            //{
            //    result.Result = "DataReader could not be created.";
            //    return result;
            //}

            if (participant.GetDefaultPublisherQos(ref pubQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default PublisherQos could not be resolved.";
                return result;
            }
            publisher = participant.CreatePublisher(pubQosHolder);//, null, 0);
            if (publisher == null)
            {
                result.Result = "Publisher could not be created.";
                return result;
            }

            if (publisher.GetDefaultDataWriterQos(ref dwQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DataWriterQos could not be resolved.";
                return result;
            }

            datawriter = (mod.tstDataWriter)publisher.CreateDataWriter(topic, dwQosHolder);//, null, 0);
            testCase.RegisterObject("factory", factory);
            testCase.RegisterObject("participantQos", pqosHolder);
            testCase.RegisterObject("participant", participant);
            testCase.RegisterObject("topic", topic);
            testCase.RegisterObject("topicQos", tQosHolder);
            testCase.RegisterObject("subscriber", subscriber);
            testCase.RegisterObject("subscriberQos", subQosHolder);
            testCase.RegisterObject("datareader", datareader);
            //testCase.RegisterObject("datareader2", datareader2);
            testCase.RegisterObject("datareaderQos", drQosHolder);
            testCase.RegisterObject("publisher", publisher);
            testCase.RegisterObject("publisherQos", pubQosHolder);
            testCase.RegisterObject("datawriter", datawriter);
            testCase.RegisterObject("datawriterQos", dwQosHolder);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
