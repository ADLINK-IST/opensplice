namespace test
{
    public class BoundsCheckEntities
    {
        public DDS.DomainParticipantFactory factory;
        public DDS.IDomainParticipant participant;
        public DDS.IPublisher publisher;
        public mod.boundsTypeTypeSupport typeSupport;
        public DDS.ITopic topic;
        public mod.IboundsTypeDataWriter datawriter;
        public mod.boundsType message;

        public Test.Framework.TestResult InitBoundsCheckEntities(Test.Framework.TestResult result)
        {
            factory = DDS.DomainParticipantFactory.Instance;

            if (factory == null)
            {
                result.Result = "DomainParticipantFactory could not be initialized.";
                return result;
            }
            DDS.DomainParticipantQos domainParticipantQos = null;
            factory.GetDefaultParticipantQos(ref domainParticipantQos);

            if (domainParticipantQos == null)
            {
                result.Result = "Default DomainParticipantQos could not be resolved.";
                return result;
            }
            participant = factory.CreateParticipant("", domainParticipantQos, null, 0);

            if (participant == null)
            {
                result.Result = "Creation of DomainParticipant failed.";
                return result;
            }

            typeSupport = new mod.boundsTypeTypeSupport();
            DDS.ReturnCode rc = typeSupport.RegisterType(participant, "boundsType");

            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Typesupport could not be registered.";
                return result;
            }
            DDS.TopicQos topicQos = null;
            participant.GetDefaultTopicQos(ref topicQos);

            if (topicQos == null)
            {
                result.Result = "Default TopicQos could not be resolved.";
                return result;
            }
            topic = participant.CreateTopic("bounds", "boundsType", topicQos, null, 0);

            if (topic == null)
            {
                result.Result = "Topic could not be created.";
                return result;
            }

            DDS.PublisherQos publisherQos = null;
            participant.GetDefaultPublisherQos(ref publisherQos);

            if (publisherQos == null)
            {
                result.Result = "Default PublisherQos could not be resolved.";
                return result;
            }
            publisher = participant.CreatePublisher(publisherQos, null, 0);

            if (publisher == null)
            {
                result.Result = "Publisher could not be created.";
                return result;
            }

            DDS.DataWriterQos dataWriterQos = null;
            publisher.GetDefaultDataWriterQos(ref dataWriterQos);

            if (dataWriterQos == null)
            {
                result.Result = "Default DataWriterQos could not be resolved.";
                return result;
            }

            datawriter = publisher.CreateDataWriter(topic, dataWriterQos) as mod.IboundsTypeDataWriter;

            if (datawriter == null)
            {
                result.Result = "DataWriter could not be created.";
                return result;
            }

            return result;
        }
    }

    public class SACSTesterBoundsCheck
    {
        public static void Main(string[] args)
        {
            BoundsCheckEntities bce = new BoundsCheckEntities();

            Test.Framework.TestSuite suite = new Test.Framework.TestSuite();
            suite.AddMilestone("Test start");

            // TODO: JLS, the commented tests are missing? Do we care?

            //suite.AddTest(new test.sacs.BoundsCheck0(bce));
            suite.AddTest(new test.sacs.BoundsCheck1(bce));
            //suite.AddTest(new test.sacs.BoundsCheck2(bce));
            //suite.AddTest(new test.sacs.BoundsCheck3(bce));
            //suite.AddTest(new test.sacs.BoundsCheck4(bce));
            //suite.AddTest(new test.sacs.BoundsCheck5(bce));
            //suite.AddTest(new test.sacs.BoundsCheck6(bce));
            //suite.AddTest(new test.sacs.BoundsCheck7(bce));
            //suite.AddTest(new test.sacs.BoundsCheck8(bce));
            //suite.AddTest(new test.sacs.BoundsCheck9(bce));
            //suite.AddTest(new test.sacs.BoundsCheck999(bce));
            suite.AddMilestone("Test end");
            suite.RunTests();
            suite.PrintReport();
        }
    }
}
