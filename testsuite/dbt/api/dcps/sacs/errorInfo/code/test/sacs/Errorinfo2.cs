namespace test.sacs
{
    using Test.Framework;

    public class Errorinfo2 : Test.Framework.TestCase
    {
        private DDS.DomainParticipantFactory factory = null;

        private DDS.IDomainParticipant participant = null;

        private DDS.DomainParticipantQos pqosHolder;

        private DDS.TopicQos topQosHolder;

        private DDS.ITopic topic = null;

        private mod.tstTypeSupport typeSupport = null;

        public Errorinfo2()
            : base("sacs_errorinfo_tc2", "errorinfo", "errorinfo", "create an errorinfo instance and retrieve its items"
                , "Entity can be created but no info is returned before error reported ", null)
        {
        }

        public virtual Test.Framework.TestResult Init()
        {
            DDS.ReturnCode rc;
            string name;
            string typeName;
            Test.Framework.TestResult result = new Test.Framework.TestResult("Initialization success"
                , string.Empty, TestVerdict.Pass, TestVerdict.Fail
                );
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
            typeSupport = new mod.tstTypeSupport();
            if (typeSupport == null)
            {
                result.Result = "Creation of tstTypeSupport failed.";
                return result;
            }
            name = "my_topic";
            typeName = "my_type";
            rc = typeSupport.RegisterType(participant, typeName);
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Register type failed.";
                return result;
            }

            if (participant.GetDefaultTopicQos(ref topQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default TopicQos could not be resolved.";
                return result;
            }
            topic = participant.CreateTopic(name, typeName, topQosHolder);//, null, 0);
            if (topic == null)
            {
                result.Result = "Topic could not be created.";
                return result;
            }
            result.Result = "Initialization success.";
            result.Verdict = TestVerdict.Pass;
            return null;
        }

        public virtual void Deinit()
        {
            participant.DeleteContainedEntities();
            factory.DeleteParticipant(participant);
        }

        public override Test.Framework.TestResult Run()
        {
            Test.Framework.TestResult result;
            result = this.Init();
            if (result == null)
            {
                result = Test();
                Deinit();
            }
            return result;
        }

        public virtual Test.Framework.TestResult Test()
        {
            Test.Framework.TestResult result;
            string sh;
            DDS.ErrorCode eh;
            DDS.ReturnCode rc;
            rc = factory.DeleteParticipant(participant);
            if (rc != DDS.ReturnCode.Ok)
            {
                if (DDS.ErrorInfo.Update() != DDS.ReturnCode.Ok)
                {
                    return new Test.Framework.TestResult("Update returns OK after the error occurred"
                        , "Update did not return OK after the error occurred", TestVerdict.Pass, TestVerdict.Fail);
                }

                rc = DDS.ErrorInfo.GetLocation(out sh);
                if (rc != DDS.ReturnCode.Ok)
                {
                    return new Test.Framework.TestResult("get_location returns OK and modifies string"
                        , "Different result and/or string not modified", TestVerdict.Pass, TestVerdict.Fail);
                }
                System.Console.WriteLine("Location: " + sh);
                sh = null;
                rc = DDS.ErrorInfo.GetMessage(out sh);
                if (rc != DDS.ReturnCode.Ok)
                {
                    return new Test.Framework.TestResult("get_message returns OK and modifies string"
                        , "Different result and/or string modified", TestVerdict.Pass, TestVerdict.Fail);
                }
                System.Console.WriteLine("Message: " + sh);
                sh = null;
                rc = DDS.ErrorInfo.GetStackTrace(out sh);
                if (rc != DDS.ReturnCode.Ok)
                {
                    return new Test.Framework.TestResult("get_stack_trace returns OK and modifies string"
                        , "Different result and/or string not modified", TestVerdict.Pass, TestVerdict.Fail);
                }
                System.Console.WriteLine("Stack trace: " + sh);
                sh = null;
                rc = DDS.ErrorInfo.GetSourceLine(out sh);
                if (rc != DDS.ReturnCode.Ok)
                {
                    return new Test.Framework.TestResult("get_source_line returns OK and modifies string"
                        , "Different result and/or string not modified", TestVerdict.Pass, TestVerdict.Fail);
                }
                System.Console.WriteLine("Source line: " + sh);
                eh = (DDS.ErrorCode)(-1);
                rc = DDS.ErrorInfo.GetCode(out eh);
                if ((rc != DDS.ReturnCode.Ok) || (eh == (DDS.ErrorCode)(-1)))
                {
                    return new Test.Framework.TestResult("get_code returns OK and does modify error code"
                        , "Different result and/or error code not modified", TestVerdict.Pass, TestVerdict.Fail);
                }
                System.Console.WriteLine("Returncode: " + rc);
                System.Console.WriteLine("Error code: " + eh);
                if (eh != DDS.ErrorCode.ContainsEntities)
                {
                    return new Test.Framework.TestResult("Expected errorcode " + DDS.ErrorCode.ContainsEntities,
                        "Obtained errorcode " + eh, TestVerdict.Pass, TestVerdict.Fail);
                }
                result = new Test.Framework.TestResult("sacs_errorinfo_tc2 successfull", "sacs_errorinfo_tc2_successfull",
                    TestVerdict.Pass, TestVerdict.Pass);
            }
            else
            {
                result = new Test.Framework.TestResult("delete_participant should fail", "delete_participant did not fail",
                    TestVerdict.Pass, TestVerdict.Fail);
            }
            return result;
        }
    }
}
