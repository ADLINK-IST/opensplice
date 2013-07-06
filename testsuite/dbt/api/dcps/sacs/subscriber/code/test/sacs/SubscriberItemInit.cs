namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class SubscriberItemInit : Test.Framework.TestItem
    {
        public SubscriberItemInit()
            : base("Initialize Subscriber")
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.SubscriberQos subQosHolder = null;
            DDS.ISubscriber subscriber;
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
            testCase.RegisterObject("factory", factory);
            testCase.RegisterObject("participantQos", pqosHolder);
            testCase.RegisterObject("participant", participant);
            testCase.RegisterObject("subscriber", subscriber);
            testCase.RegisterObject("subscriberQos", subQosHolder);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
