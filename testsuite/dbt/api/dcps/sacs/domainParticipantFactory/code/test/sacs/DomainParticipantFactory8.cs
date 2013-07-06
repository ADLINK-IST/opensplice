namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipantFactory8 : Test.Framework.TestCase
    {
        public DomainParticipantFactory8()
            : base("sacs_domainParticipantFactory_tc8", "sacs_domainParticipantFactory"
                , "domainParticipantFactory", "Test PARTICIPANT_QOS_DEFAULT.", "Test PARTICIPANT_QOS_DEFAULT."
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Default participantQos is used when PARTICIPANT_QOS_DEFAULT is specified.";
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
			DDS.DomainParticipantQos pqosHolder = null;
			DDS.DomainParticipantQos pqosHolder2 = null;
            DDS.ReturnCode returnCode;
            byte[] ud;
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipantFactory could not be initialised."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            returnCode = factory.GetDefaultParticipantQos(ref pqosHolder);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Default DomainParticipantQos could not be resolved ("
                     + returnCode + ").", expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            pqosHolder.EntityFactory.AutoenableCreatedEntities = true;
            ud = new byte[1];
            ud[0] = System.Convert.ToByte("4");
            pqosHolder.UserData.Value = ud;
            returnCode = factory.SetDefaultParticipantQos(pqosHolder);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Set default participant qos failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default);
            if (participant == null)
            {
                result = new Test.Framework.TestResult(expResult, "Creation of participant failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }

            returnCode = participant.GetQos(ref pqosHolder2);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Resolving of ParticipantQos failed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = factory.DeleteParticipant(participant);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "Deleting participant failed.",
                    expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.EntityFactory.AutoenableCreatedEntities != pqosHolder2
                .EntityFactory.AutoenableCreatedEntities)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved entity_factory policy does not match the applied one."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.UserData.Value.Length != pqosHolder2.UserData.Value.
                Length)
            {
                result = new Test.Framework.TestResult(expResult, "Resolved UserData policy does not match the applied one."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (pqosHolder.UserData.Value[0] != pqosHolder2.UserData.Value[0])
            {
                result = new Test.Framework.TestResult(expResult, "Resolved UserData policy does not match the applied one (2)."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
                );
            return result;
        }
    }
}
