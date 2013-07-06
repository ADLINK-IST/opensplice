namespace test.sacs
{
    /// <summary>Test register_typesupport function.</summary>
    /// <remarks>Test register_typesupport function.</remarks>
    public class TypeSupport2 : Test.Framework.TestCase
    {
        /// <summary>Test register_typesupport function.</summary>
        /// <remarks>Test register_typesupport function.</remarks>
        public TypeSupport2()
            : base("sacs_typeSupport_tc2", "sacs_typeSupport", "typeSupport"
                , "TypeSupport Register", "Test if function register_typesupport can handle incorrect parameters"
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            string expResult = "register_typesupport tests pass";
            DDS.DomainParticipantFactory factory;
	    DDS.DomainParticipantQos participantQosHolder = null;
            DDS.IDomainParticipant participant;
            mod.tstTypeSupport typeSupport;
            DDS.ReturnCode rc;
            Test.Framework.TestResult result;
            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict
                .Pass, Test.Framework.TestVerdict.Fail);
            factory = DDS.DomainParticipantFactory.Instance;
            if (factory == null)
            {
                result.Result = "DomainParticipantFactory.get_instance() did not return a factory (1)";
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref participantQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "factory.get_default_participant_qos() did not return a qos (2)";
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, participantQosHolder);//, null, 0);
            if (participant == null)
            {
                result.Result = "factory.create_participant() did not return a participant (2)";
                return result;
            }
            typeSupport = new mod.tstTypeSupport();
            rc = typeSupport.RegisterType(null, null);
            if (rc == DDS.ReturnCode.Ok)
            {
                result.Result = "could register a type on a null participant and with a null name (3)";
                return result;
            }
            rc = typeSupport.RegisterType(participant, null);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not register a type with a null name (4)";
                return result;
            }

            rc = typeSupport.RegisterType(participant, string.Empty);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not register a type with an empty name (5)";
                return result;
            }

            rc = typeSupport.RegisterType(null, "type_name6");
            if (rc == DDS.ReturnCode.Ok)
            {
                result.Result = "could register a type on a null participant (6)";
                return result;
            }

            rc = factory.DeleteParticipant(participant);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a participant (7)";
                return result;
            }
            rc = typeSupport.RegisterType(null, "type_name8");
            if (rc == DDS.ReturnCode.Ok)
            {
                result.Result = "could register a type on a null participant (8)";
                return result;
            }


            rc = typeSupport.RegisterType(participant, "type_name9");
            if (rc == DDS.ReturnCode.Ok)
            {
                result.Result = "could register a type on a deleted participant (9)";
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, participantQosHolder);//, null, 0);
            if (participant == null)
            {
                result.Result = "factory.create_participant() did not return a participant (10)";
                return result;
            }
            rc = typeSupport.RegisterType(participant, "    ");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not register a type with all spaces (11)";
                return result;
            }
            rc = factory.DeleteParticipant(participant);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "participant could not be deleted (12)";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
