namespace test.sacs
{
    /// <summary>Test register_typesupport function.</summary>
    /// <remarks>Test register_typesupport function.</remarks>
    public class TypeSupport4 : Test.Framework.TestCase
    {
        /// <summary>Test register_typesupport function.</summary>
        /// <remarks>Test register_typesupport function.</remarks>
        public TypeSupport4()
            : base("sacs_typeSupport_tc4", "sacs_typeSupport", "typeSupport"
                , "TypeSupport get_type_name", "Test if get_type_name returns the correct typename"
                , null)
        {
        }

        public override Test.Framework.TestResult Run()
        {
            string expResult = "get_type_name returns the correct name";
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
                result.Result = "DomainParticipantFactory.get_instance() did not return a factory";
                return result;
            }

            if (factory.GetDefaultParticipantQos(ref participantQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "factory.get_default_participant_qos() did not return a qos";
                return result;
            }
            participant = factory.CreateParticipant(DDS.DomainId.Default, participantQosHolder);//, null, 0);
            if (participant == null)
            {
                result.Result = "factory.create_participant() did not return a participant";
                return result;
            }
            typeSupport = new mod.tstTypeSupport();
            string typeName = typeSupport.TypeName;
            if (typeName == null)
            {
                result.Result = "get_type_name returned null (1)";
                return result;
            }
            if (!typeName.Equals("mod::tst"))
            {
                result.Result = "get_type_name returned incorrect typename (1)";
                return result;
            }
            rc = typeSupport.RegisterType(participant, "type1");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "could not register a type";
                return result;
            }
            typeName = typeSupport.TypeName;
            if (typeName == null)
            {
                result.Result = "get_type_name returned null (2)";
                return result;
            }
            if (!typeName.Equals("mod::tst"))
            {
                result.Result = "get_type_name returned incorrect typename (2)";
                return result;
            }
            mod.otherTypeTypeSupport typeSupport2 = new mod.otherTypeTypeSupport();
            typeName = typeSupport2.TypeName;
            if (typeName == null)
            {
                result.Result = "get_type_name returned null (3)";
                return result;
            }
            if (!typeName.Equals("mod::otherType"))
            {
                result.Result = "get_type_name returned incorrect typename (3)";
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
