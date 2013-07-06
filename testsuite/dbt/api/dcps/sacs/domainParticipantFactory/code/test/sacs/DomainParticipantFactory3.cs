namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipantFactory3 : Test.Framework.TestCase
    {
        public DomainParticipantFactory3()
            : base("sacs_domainParticipantFactory_tc3", "sacs_domainParticipantFactory"
                , "domainParticipantFactory", "Test if a DomainParticipant can be looked up.", "Test if a DomainParticipant can be looked up"
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "Creating/deleting DomainParticipant succeeded.";
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
            DDS.IDomainParticipant participant2;
			DDS.DomainParticipantQos pqosHolder = null;
            DDS.ReturnCode returnCode;
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
                result = new Test.Framework.TestResult(expResult, "Default DomainParticipantQos could not be resolved."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipant could not be created."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant2 = factory.LookupParticipant(DDS.DomainId.Default);
            if (participant2 == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipant could not be looked up."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (participant != participant2)
            {
                result = new Test.Framework.TestResult(expResult, "Looked up DomainParticipant is not the same as the created one."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = factory.DeleteParticipant(participant);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipant could not be deleted."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = factory.DeleteParticipant(participant2);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipant 2 could be deleted, but is was already gone."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = factory.DeleteParticipant(participant);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipant could be deleted, but is was already gone."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = factory.DeleteParticipant(null);
            if (returnCode != DDS.ReturnCode.BadParameter)
            {
                result = new Test.Framework.TestResult("RETCODE_BAD_PARAMETER (RETCODE " + DDS.ReturnCode.BadParameter +
                    ") after deletion of an invalid participant", "Return code " + returnCode
                     + " was returned by factory.delete_participant", expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = factory.DeleteParticipant(participant);
            if (returnCode != DDS.ReturnCode.BadParameter)
            {
                result = new Test.Framework.TestResult("RETCODE_BAD_PARAMETER (RETCODE " + DDS.ReturnCode.BadParameter +
                     ") after deletion of an invalid participant", "Return code " + returnCode
                     + " was returned by factory.delete_participant", expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            if (participant != null)
            {
                returnCode = participant.Enable();
                if (returnCode == DDS.ReturnCode.Ok)
                {
                    result = new Test.Framework.TestResult("Enable on a deleted participant failed",
                        "Enable on a deleted participant succeeded", Test.Framework.TestVerdict.Fail,
                        Test.Framework.TestVerdict.Fail);
                    return result;
                }
            }
            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
                );
            return result;
        }
    }
}
