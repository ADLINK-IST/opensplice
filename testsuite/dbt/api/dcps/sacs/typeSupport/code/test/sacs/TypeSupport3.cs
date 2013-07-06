namespace test.sacs
{
    /// <summary>Test register_typesupport function.</summary>
    /// <remarks>Test register_typesupport function.</remarks>
    public class TypeSupport3 : Test.Framework.TestCase
    {
        /// <summary>Test register_typesupport function.</summary>
        /// <remarks>Test register_typesupport function.</remarks>
        public TypeSupport3()
            : base("sacs_typeSupport_tc3", "sacs_typeSupport", "typeSupport"
                , "TypeSupport Register", "Test with multiple types that use the same name", null
                )
        {
        }

        public override Test.Framework.TestResult Run()
        {
            string expResult = "register_typesupport tests pass";
            DDS.DomainParticipantFactory factory;
            DDS.DomainParticipantQos participantQosHolder = null;
            DDS.IDomainParticipant participant;
            mod.tstTypeSupport typeSupport;
            mod.otherTypeTypeSupport otherTypeTypeSupport;
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
            otherTypeTypeSupport = new mod.otherTypeTypeSupport();
            
            rc = typeSupport.RegisterType(participant, "type1");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not register a type (3)";
                return result;
            }
            rc = typeSupport.RegisterType(participant, "type1");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not register the same type a second time (4)";
                return result;
            }
            rc = otherTypeTypeSupport.RegisterType(participant, "type2");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could register a different type (5)";
                return result;
            }
            rc = otherTypeTypeSupport.RegisterType(participant, "type1");
            if (rc != DDS.ReturnCode.PreconditionNotMet)
            {
                result.Result = "Expected RETCODE_PRECONDITION_NOT_MET but recieved " + rc + " after calling register_type using an already used name for a different type (6)";
                return result;
            }
            rc = factory.DeleteParticipant(participant);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not delete a participant (7)";
                return result;
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}
