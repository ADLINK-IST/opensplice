namespace test.sacs
{
    /// <date>May 23, 2005</date>
    public class TypeSupport1 : Test.Framework.TestCase
    {
        public TypeSupport1()
            : base("sacs_typeSupport_tc1", "sacs_typeSupport", "typeSupport"
                , "TypeSupport Register", "Test if a type can be registered", null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            Test.Framework.TestVerdict expVerdict = Test.Framework.TestVerdict.Pass;
            string expResult = "type is registered.";
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
            DDS.DomainParticipantQos pqosHolder = null;
            mod.tstTypeSupport typesupport;
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
            typesupport = new mod.tstTypeSupport();
            if (typesupport == null)
            {
                result = new Test.Framework.TestResult(expResult, "TypeSupport is not constructed."
                    , expVerdict, Test.Framework.TestVerdict.Fail);
                return result;
            }
            returnCode = typesupport.RegisterType(participant, "mod::tst");
            if (returnCode != DDS.ReturnCode.Ok)
            {
                result = new Test.Framework.TestResult(expResult, "TypeSupport is not registered."
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
