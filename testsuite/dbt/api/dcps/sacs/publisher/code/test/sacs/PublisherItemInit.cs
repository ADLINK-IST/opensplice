namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class PublisherItemInit : Test.Framework.TestItem
    {
        public PublisherItemInit()
            : base("Initialize Publisher")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.PublisherQos pubQosHolder = null;
            DDS.IPublisher publisher;
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
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
            testCase.RegisterObject("factory", factory);
            testCase.RegisterObject("participantQos", pqosHolder);
            testCase.RegisterObject("participant", participant);
            testCase.RegisterObject("publisher", publisher);
            testCase.RegisterObject("publisherQos", pubQosHolder);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
