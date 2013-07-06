namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class DomainParticipantFactory2 : Test.Framework.TestCase
    {
        public DomainParticipantFactory2()
            : base("sacs_domainParticipantFactory_tc2", "sacs_domainParticipantFactory"
                , "domainParticipantFactory", "Test if a DomainParticipant can be created/deleted."
                , "Test if a DomainParticipant can be created/deleted", null)
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
                result = new Test.Framework.TestResult(expResult, "Default DomainParticipantQos could not be resolved ("
                     + returnCode + ").", expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipant could not be created."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant2 = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant2 == null)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipant 2 could not be created."
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
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "DomainParticipant  2 could not be deleted."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            participant2 = factory.CreateParticipant(DDS.DomainId.Default, pqosHolder);//, null, 0);
            if (participant2 == null)
            {
                return new Test.Framework.TestResult(expResult, "failure creating a DomainParticipant with an empty domainId"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
            }
            returnCode = factory.DeleteParticipant(participant2);
            if (returnCode != DDS.ReturnCode.Ok)
            {
                return new Test.Framework.TestResult(expResult, "erroneous returncode while deleting a participant"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
            }
            participant = factory.LookupParticipant(DDS.DomainId.Default);
            if (participant != null)
            {
                return new Test.Framework.TestResult(expResult, "could still lookup deleted participant"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
            }
            returnCode = factory.DeleteParticipant(participant);
            if (returnCode == DDS.ReturnCode.Ok)
            {
                return new Test.Framework.TestResult(expResult, "could delete a non existing participant"
                    , expVerdict, Test.Framework.TestVerdict.Fail);
            }
            result = new Test.Framework.TestResult(expResult, expResult, expVerdict, expVerdict
                );
            return result;
        }
    }
}
