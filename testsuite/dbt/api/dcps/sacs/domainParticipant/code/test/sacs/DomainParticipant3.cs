namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipant3 : Test.Framework.TestCase
    {
        public DomainParticipant3()
            : base("sacs_domainParticipant_tc3", "sacs_domainParticipant"
                , "domainParticipant", "Test if default TopicQos can be resolved and set.", "Test if default TopicQos can be resolved and set."
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Setting/resolving default TopicQos succeeded.";
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.TopicQos tqosHolder = null;
            DDS.ReturnCode returnCode;
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipantFactory could not be initialised."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref pqosHolder) != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Default DomainParticipantQos could not be resolved."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant == null)
            {
                result = new Test.Framework.TestResult(expResult, "Creation of DomainParticipant failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            if (participant.GetDefaultTopicQos(ref tqosHolder) != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Topic qos could not be resolved."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = factory.DeleteParticipant(participant);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Deletion of DomainParticipant failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
                );
            return result;
        }
    }
}
